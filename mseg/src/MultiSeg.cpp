/*  Copyright (C) 2014 National Institute For Space Research (INPE) - Brazil.

    MultiSeg is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License,
    or (at your option) any later version.

    MultiSeg is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with MultiSeg. See COPYING.
 */

/*!
  \file MultiSeg.cpp

  \brief This class implements the MultiSeg algorithm.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "AbstractOutputter.h"
#include "EuclideanMerger.h"
#include "MultiSeg.h"
#include "OpticalCartoonMerger.h"
#include "RadarCartoonMerger.h"
#include "Region.h"
#include "Utils.h"

// TerraLib PDI
#include <terralib/image_processing/TePDIStatistic.hpp>
#include <terralib/image_processing/TePDIUtils.hpp>

// Boost
#include <boost/math/distributions.hpp>

// STL
#include <algorithm>
#include <cassert>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <fstream>
#include <limits>

MultiSeg::MultiSeg()
  : m_cv(TeMAXFLOAT),
    m_merger(new EuclideanMerger),
    m_similarityIncreaseStep(0),
    m_enableMutualBestFitting(true),
    m_growUntilStop(true),
    m_nSamples(std::string::npos),
    m_considerRegionVsRegion(true),
    m_currentLevel(0),
    m_pyramid(0),
    m_outputPyramid(false),
    m_notifyIntermediateResults(false)
{
}

MultiSeg::~MultiSeg()
{
  delete m_merger;

  std::map<std::size_t, Region*>::iterator it;
  for(it = m_regions.begin(); it != m_regions.end(); ++it)
    delete it->second;

  m_regions.clear();

  delete m_pyramid;
}

bool MultiSeg::CheckParameters(const TePDIParameters& params) const
{
  /* Parameter: input_image */
  TePDITypes::TePDIRasterPtrType inRaster;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("input_image", inRaster), "Missing parameter: input_image");
  TEAGN_TRUE_OR_RETURN(inRaster.isActive(), "Invalid parameter: input_image inactive");
  TEAGN_TRUE_OR_RETURN(inRaster->params().status_ != TeRasterParams::TeNotReady, "Invalid parameter: input_image not ready");

  /* Parameter: input_bands */
  std::vector<std::size_t> bands;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("input_bands", bands), "Missing parameter: input_bands");

  /* Checking input parameters about image informations */
  ImageModelRepresentation model; // Cartoon or Texture?
  TEAGN_TRUE_OR_RETURN(params.GetParameter("image_model", model), "Missing parameter: image_model");
  
  ImageType type; // Radar or Optical?
  TEAGN_TRUE_OR_RETURN(params.GetParameter("image_type", type), "Missing parameter: image_type");

  if(type == Radar)
  {
    ImageFormat format; // Amplitude, Intensity or dB?
    TEAGN_TRUE_OR_RETURN(params.GetParameter("image_radar_format", format), "Missing parameter: image_radar_format");
  }

  std::size_t levels;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("levels", levels), "Missing parameter: levels");

  double similarity;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("similarity", similarity), "Missing parameter: similarity");
  
  std::size_t minArea;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("min_area", minArea), "Missing parameter: min_area");

  if(type == Radar && model == Cartoon)
  {
    double enl; // Need ENL - number of looks
    TEAGN_TRUE_OR_RETURN(params.GetParameter("ENL", enl), "Missing parameter: ENL");
    TEAGN_TRUE_OR_RETURN(enl > 0.0, "The parameter ENL must be greater than 0.0");
    
    double confidenceLevel; // Need confidence_level
    TEAGN_TRUE_OR_RETURN(params.GetParameter("confidence_level", confidenceLevel), "Missing parameter: confidence_level");
  }

  if((type == Radar && model == Texture) || type == Optical)
  {
    double cv; // Need Coefficient of variation.
    TEAGN_TRUE_OR_RETURN(params.GetParameter("cv", cv), "Missing parameter: cv");
    
    if(type == Optical)
    {
      double confidenceLevel; // Need confidence_level
      TEAGN_TRUE_OR_RETURN(params.GetParameter("confidence_level", confidenceLevel), "Missing parameter: confidence_level");
    }
  }

  return true;
}

const TePDITypes::TePDIRasterPtrType& MultiSeg::getInputImage() const
{
  return m_inputImage;
}

const std::map<std::size_t, Region*>& MultiSeg::getRegions() const
{
  return m_regions;
}

const TePDITypes::TePDIRasterPtrType& MultiSeg::getLabelledImage() const
{
  return m_labelledImage;
}

const std::vector<std::size_t>& MultiSeg::getUsedBands() const
{
  return m_bands;
}

void MultiSeg::addOutputter(AbstractOutputter* outputter)
{
  m_outputters.push_back(outputter);
}

void MultiSeg::outputPyramid(bool on)
{
  m_outputPyramid = on;
}

void MultiSeg::notifyIntermediateResults(bool on)
{
  m_notifyIntermediateResults = on;
}

void MultiSeg::ResetState(const TePDIParameters& /*params*/)
{
  // To fix seeds
  std::srand(0);

  // To randomize seeds
  //std::srand(unsigned int(std::time(0)));
}

bool MultiSeg::RunImplementation()
{
  initializeParameters();

  // To region growing
  bool useRandomSeeds = true;

  if(m_levels == 0)
  {
    // One level!
    m_pyramid = new Pyramid(m_inputImage, m_levels, m_bands, progress_enabled_);

    // Initializes the labelled image
    TeRasterParams params = m_inputImage->params();
    params.nBands(1);
    params.setDataType(TeUNSIGNEDLONG);
    TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(params, m_labelledImage), "Error creating the level labelled image.");

    // Initializes the regions
    initializeRegions(m_inputImage);

    // Updates the thresholds
    updateThresholds(m_levels);

    /* Region Growing */
    executeRegionGrowing(m_regions, useRandomSeeds);

    /* Minimum area */

    // In the minimum area process it is always valid
    m_considerRegionVsRegion = true;

    delete m_merger;
    m_merger = new EuclideanMerger;

    // Process the small regions. Note: Here uses the euclidean merger
    processSmallRegions();
  }
  else
  {
    // Generates the pyramid hierarchy
    m_pyramid = new Pyramid(m_inputImage, m_levels, m_bands, progress_enabled_);

    // Output the pyramid
    if(m_outputPyramid)
    {
      for(std::size_t i = 0; i < m_outputters.size(); ++i)
        m_outputters[i]->outputPyramid(*m_pyramid);
    }

    // Gets the lowest level
    TePDITypes::TePDIRasterPtrType lowestLevel = m_pyramid->getLevel(m_levels);

    // Initializes the labelled image
    TeRasterParams params = lowestLevel->params();
    params.nBands(1);
    params.setDataType(TeUNSIGNEDLONG);
    TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(params, m_labelledImage), "Error creating the lowest level labelled image.");

    // Initializes the regions
    initializeRegions(lowestLevel);

    // Updates the thresholds
    updateThresholds(m_levels);

    /* First Region Growing */
    m_considerRegionVsRegion = true;
    executeRegionGrowing(m_regions, useRandomSeeds);

    // Notifies the intermediate results
    if(m_notifyIntermediateResults)
      notifyResult();

    std::cout << "--- Level " <<  m_currentLevel << " completed! # Number of Regions: " << m_regions.size() << std::endl;

    // A flag that indicates if the last level should be splitted
    bool splitLastLevel = false;

    for(int i = m_levels - 1; i >= 0; --i) // for each requested level
    {
      // Releases the previous used level
      m_pyramid->releaseLevel(i + 1);

      // Gets the next pyramid level
      TePDITypes::TePDIRasterPtrType inputImageCurrentLevel = m_pyramid->getLevel(i);

      // Resizes the labelled image
      m_labelledImage = Pyramid::resize(m_labelledImage, inputImageCurrentLevel->params());

      // Resizes the regions
      resizeRegions();

      // Updates the thresholds
      updateThresholds(i);

      // Updating the region statistics...
      updateRegionStatistics(inputImageCurrentLevel);

      /* Border adjustment */
      adjustRegionBorders(inputImageCurrentLevel); // Need review!

      // Updating the region statistics...
      updateRegionStatistics(inputImageCurrentLevel);

      if(i != 0 || splitLastLevel)
      {
        /* Split Regions */
        std::map<std::size_t, Region*> newRegions;
        splitRegions(inputImageCurrentLevel, newRegions);

        /* Region Growing of the new regions */
        m_considerRegionVsRegion = false;
        executeRegionGrowing(newRegions, useRandomSeeds);
      }

      /* Region Growing to regions merge */
      m_considerRegionVsRegion = true;
      executeRegionGrowing(m_regions, useRandomSeeds);

      // Updating the region statistics...
      updateRegionStatistics(inputImageCurrentLevel);

      // Notifies the intermediate results
      if(i != 0 && m_notifyIntermediateResults)
        notifyResult();

      std::cout << "--- Level " <<  m_currentLevel << " completed! # Number of Regions: " << m_regions.size() << std::endl;
    }

    // In the minimum area process it is always valid
    m_considerRegionVsRegion = true;

    delete m_merger;
    m_merger = new EuclideanMerger;

    // Process the small regions. Note: Here uses the euclidean merger
    processSmallRegions();
  }

  // Notifies the final results
  notifyResult();

  std::cout << "--- Segmentation completed! # Number of Regions: " << m_regions.size() << std::endl << std::endl;

  return true;
}

void MultiSeg::initializeParameters()
{
  // Getting input image
  params_.GetParameter("input_image", m_inputImage);

  // Getting input bands
  params_.GetParameter("input_bands", m_bands);

  // Cartoon or Texture?
  params_.GetParameter("image_model", m_imageModel);

  // Radar or Optical?
  params_.GetParameter("image_type", m_imageType);

  params_.GetParameter("levels", m_levels);

  // Verify requested number of levels
  std::size_t maxLevels = Utils::ComputeMaxLevels(m_inputImage->params().nlines_, m_inputImage->params().ncols_);
  if(m_levels > maxLevels)
  {
    m_levels = maxLevels;
    std::cout << "* Number of compression levels overridden to " << maxLevels << " ." << std::endl;
  }

  params_.GetParameter("similarity", m_similarity);
  params_.GetParameter("min_area", m_minArea);

  if(m_imageType == Radar)
  {
    // So, the input image is Radar. Which is the format?
    params_.GetParameter("image_radar_format", m_imageRadarFormat); // Intensity, Amplitude or dB?
    if(m_imageRadarFormat != Intensity)
    {
      if(m_imageRadarFormat == Amplitude)
      {
        TePDITypes::TePDIRasterPtrType intensityImage = Utils::Amplitude2Intensity(m_inputImage);
        m_inputImage = intensityImage;
      }
      else
      {
        // TODO: Utils::dB2Intensity method!
      }
    }

    /* Converts similarity (dB) to Intensity */

    // Statistic Algorithm
    TePDIParameters statParams;

    // Input Raster & input bands
    TePDITypes::TePDIRasterVectorType rasters;
    std::vector<int> bands;

    for(std::size_t i = 0; i < m_bands.size(); ++i)
    {
      rasters.push_back(m_inputImage);
      bands.push_back(m_bands[i]);
    }

    statParams.SetParameter("rasters", rasters);
    statParams.SetParameter("bands", bands);

    // Calculates the mean of image
    TePDIStatistic stat;
    stat.ToggleProgInt(false);
    stat.Reset(statParams);

    double minMean = TeMAXFLOAT;
    for(std::size_t i = 0; i < m_bands.size(); ++i)
      minMean = (std::min)(stat.getMean(i), minMean);

    double db = m_similarity;
    m_similarity = minMean * (std::pow(10.0, db / 10.0) - 1.0);
    
    /* end-Converts similarity (dB) to Intensity */
  }

  if(m_imageType == Radar && m_imageModel == Cartoon)
  {
    params_.GetParameter("ENL", m_ENL);
    params_.GetParameter("confidence_level", m_confidenceLevel);

    if(m_confidenceLevel == 1.0)
      m_confidenceLevel = 0.99999;

    // Loads the table of Coefficient of Variation.
    m_cvTable.load(m_confidenceLevel);
  }

  if((m_imageType == Radar && m_imageModel == Texture) || m_imageType == Optical)
  {
    params_.GetParameter("cv", m_cv);
    if(m_imageType == Optical)
      params_.GetParameter("confidence_level", m_confidenceLevel);
  }

  initializeMerger();
}

void MultiSeg::initializeMerger()
{
  if(m_imageType == Radar && m_imageModel == Cartoon)
  {
    delete m_merger;
    m_merger = new RadarCartoonMerger;
  }

  if(m_imageType == Optical && m_imageModel == Cartoon)
  {
    delete m_merger;
    m_merger = new OpticalCartoonMerger;
  }

  // TODO: [Radar + Texture] / [Optical + Texture]

  m_merger->setParam("confidence_level", m_confidenceLevel);
}

void MultiSeg::initializeRegions(const TePDITypes::TePDIRasterPtrType& image)
{
  int nBands = m_bands.size();
  int nLines = image->params().nlines_;
  int nCols  = image->params().ncols_;

  std::vector<double> pixel;
  pixel.resize(nBands, 0.0);

  double value = 0.0;
  bool valueWasRead;

  std::size_t nRegions = 0;
  TePDIPIManager progress("Initializing Regions", nLines * nCols, progress_enabled_);

  for(int lin = 0; lin < nLines; ++lin)
  {
    for(int col = 0; col < nCols; ++col)
    {
      for(int b = 0; b < nBands; ++b)
      {
        valueWasRead = image->getElement(col, lin, value, m_bands[b]);
        assert(valueWasRead);
        pixel[b] = value;
      }

      // Generates an id for the new region
      std::size_t id = Utils::GenerateId(lin, col, nCols);

      // Initializing a new region
      Region* region = new Region(id, pixel, lin, col);

      // Indexing...
      m_regions[id] = region;

      // First, each region is a pixel
      m_labelledImage->setElement(col, lin, id);

      // Building the neighborhood information
      if(lin)
      {
        Region* neighbour = getRegion(Utils::GenerateId(lin - 1, col, nCols));
        if(neighbour != 0)
        {
          region->addNeighbour(neighbour);
          neighbour->addNeighbour(region);
        }
      }

      if(col)
      {
        Region* neighbour = getRegion(Utils::GenerateId(lin, col - 1, nCols));
        if(neighbour != 0)
        {
          region->addNeighbour(neighbour);
          neighbour->addNeighbour(region);
        }
      }

      progress.Update(++nRegions);
    }
  }
}

void MultiSeg::executeRegionGrowing(std::map<std::size_t, Region*>& regions, bool useRandomSeeds, std::size_t maxIterations)
{
  std::size_t iteration = 0;
  std::size_t noMergeIterations = 0;
  std::size_t mergedRegions = 0;

  double threshold = m_currentSimilarity / (double)(m_similarityIncreaseStep + 1);
  m_merger->setParam("euclidean_distance_threshold", threshold);

  TePDIPIManager progress("Merging Regions - Level " + Te2String(m_currentLevel), maxIterations, progress_enabled_);

  while(true)
  {
    useRandomSeeds == false ? mergedRegions = mergeRegions(regions) :
    /* else */                mergedRegions = mergeRegionsRandomly(regions);

    ++iteration;

    progress.Update(iteration);

    if(iteration == maxIterations)
      break;

    if(mergedRegions == 0)
    {
      ++noMergeIterations;

      if(noMergeIterations > m_similarityIncreaseStep)
        break;

      threshold += (((double)m_currentSimilarity) / ((double)(m_similarityIncreaseStep + 1)));
      m_merger->setParam("euclidean_distance_threshold", threshold);
    }
  }

  m_merger->setParam("euclidean_distance_threshold", m_currentSimilarity);
}

std::size_t MultiSeg::mergeRegions(std::map<std::size_t, Region*>& regions)
{
  // The number of merged regions
  std::size_t mergedRegions = 0;

  std::map<std::size_t, Region*>::iterator regionsIt = regions.begin();
  std::map<std::size_t, Region*>::iterator regionsItEnd = regions.end();

  while(regionsIt != regionsItEnd) // for each region
  {
    // The current region
    Region* currentRegion = regionsIt->second;

    // Try gets the closest neighbour region
    Region* closestNeighbour = getClosestRegion(currentRegion);

    // Is necessary mutual best fitting?
    if(closestNeighbour != 0 && m_enableMutualBestFitting)
    {
      Region* backClosestNeighbour = getClosestRegion(closestNeighbour);
      if(backClosestNeighbour == 0 || (backClosestNeighbour != currentRegion))
        closestNeighbour = 0;
    }

    // If the minimum dissimilarity neighbour was found it will be merged
    if(closestNeighbour != 0)
    {
      m_merger->merge(currentRegion, closestNeighbour);

      updateNeighborhoodAfterMerge(currentRegion, closestNeighbour);

      updateLabelledImage(currentRegion, closestNeighbour);

      std::size_t idToRemove = closestNeighbour->getId();

      delete closestNeighbour;

      regions.erase(idToRemove);

      m_regions.erase(idToRemove);

      ++mergedRegions;

      if(m_growUntilStop)
        continue;
    }

    ++regionsIt; // next region!
  }

  return mergedRegions;
}

std::size_t MultiSeg::mergeRegionsRandomly(std::map<std::size_t, Region*>& regions)
{
  // The number of merged regions
  std::size_t mergedRegions = 0;

  std::map<std::size_t, Region*>::iterator regionsIt = regions.begin();
  std::map<std::size_t, Region*>::iterator regionsItEnd = regions.end();

  // Random ids vector
  std::vector<std::size_t> ids(regions.size(), 0);
  std::size_t index = 0;
  while(regionsIt != regionsItEnd)
  {
    ids[index] = regionsIt->second->getId();
    ++index;
    ++regionsIt;
  }

  // Suffle!
  std::random_shuffle(ids.begin(), ids.end());

  for(std::size_t i = 0; i < ids.size(); ++i)
  {
    std::map<std::size_t, Region*>::iterator it = regions.find(ids[i]);
    if(it == regions.end())
      continue;

    // The current region
    Region* currentRegion = regions[ids[i]];

    while(true)
    {
      // Try gets the closest neighbour region
      Region* closestNeighbour = getClosestRegion(currentRegion);

      // Is necessary mutual best fitting?
      if(closestNeighbour != 0 && m_enableMutualBestFitting)
      {
        Region* backClosestNeighbour = getClosestRegion(closestNeighbour);
        if(backClosestNeighbour == 0 || (backClosestNeighbour != currentRegion))
          closestNeighbour = 0;
      }

      // If the minimum dissimilarity neighbour was found it will be merged
      if(closestNeighbour != 0)
      {
        m_merger->merge(currentRegion, closestNeighbour);

        updateNeighborhoodAfterMerge(currentRegion, closestNeighbour);

        updateLabelledImage(currentRegion, closestNeighbour);

        std::size_t idToRemove = closestNeighbour->getId();

        delete closestNeighbour;

        regions.erase(idToRemove);

        m_regions.erase(idToRemove);

        ++mergedRegions;

        if(!m_growUntilStop)
          break;
      }
      else
        break;
    }
  }

  return mergedRegions;
}

std::size_t MultiSeg::mergeSmallRegions()
{
  // The number of merged regions
  std::size_t mergedRegions = 0;

  std::map<std::size_t, Region*>::iterator regionsIt = m_regions.begin();
  std::map<std::size_t, Region*>::iterator regionsItEnd = m_regions.end();

  while(regionsIt != regionsItEnd) // for each region
  {
    // The current region
    Region* currentRegion = regionsIt->second;

    // Is this a small region ?
    if(currentRegion->getSize() > m_minArea)
    {
      ++regionsIt;
      continue;
    }

    // Gets the closest neighbour region
    Region* closestNeighbour = getClosestRegion(currentRegion, true);

    // If the minimum dissimilarity neighbour was not found, continue...
    if(closestNeighbour == 0)
    {
      ++regionsIt;
      continue;
    }

    m_merger->merge(closestNeighbour, currentRegion);

    updateNeighborhoodAfterMerge(closestNeighbour, currentRegion);

    updateLabelledImage(closestNeighbour, currentRegion);

    std::size_t idToRemove = currentRegion->getId();

    delete currentRegion;

    ++regionsIt; // next region!

    m_regions.erase(idToRemove);

    ++mergedRegions;
  }

  return mergedRegions;
}

Region* MultiSeg::getRegion(const std::size_t& id)
{
  std::map<std::size_t, Region*>::const_iterator it = m_regions.find(id);

  if(it == m_regions.end())
    return 0;

  return it->second;
}

Region* MultiSeg::getClosestRegion(Region* region, bool useAllNeighbours)
{
  assert(region);

  // Gets the closest regions
  std::list<Region*> closestRegions;
  useAllNeighbours ? closestRegions = region->getNeighbours() : closestRegions = getClosestRegions(region);

  if(closestRegions.empty())
    return 0;

  if(closestRegions.size() == 1)
    return *closestRegions.begin();

  // There are two or more closest regions. Computes the euclidean distance!
  Region* closestRegion = 0;
  double minEuclideanDistance = (std::numeric_limits<double>::max)();

  std::list<Region*>::iterator it;
  for(it = closestRegions.begin(); it != closestRegions.end(); ++it)
  {
    Region* currentNeighbor = *it;
    assert(currentNeighbor);

    double euclideanDistance = m_merger->getSquaredEuclideanDistance(region, currentNeighbor);
    
    if(euclideanDistance < minEuclideanDistance)
    {
      minEuclideanDistance = euclideanDistance;
      closestRegion = currentNeighbor;
    }
  }

  return closestRegion;
}

std::list<Region*> MultiSeg::getClosestRegions(Region* region)
{
  assert(region);

  std::list<Region*>& neighbours = region->getNeighbours();

  std::list<Region*>::iterator neighborIt = neighbours.begin();
  std::list<Region*>::iterator neighborItEnd = neighbours.end();

  std::list<Region*> closestRegions;

  while(neighborIt != neighborItEnd)
  {
    Region* currentNeighbor = *neighborIt;
    assert(currentNeighbor);

    if(region->getSize() > 1 && currentNeighbor->getSize() > 1 && !m_considerRegionVsRegion)
    {
      ++neighborIt;
      continue;
    }

    if(m_merger->predicate(region, currentNeighbor) == true)
      closestRegions.push_back(currentNeighbor);

    ++neighborIt;
  }

  return closestRegions;
}

void MultiSeg::updateNeighborhoodAfterMerge(Region* region, Region* merged)
{
  std::list<Region*>::iterator neighborIt = merged->getNeighbours().begin();
  std::list<Region*>::iterator neighborItEnd = merged->getNeighbours().end();

  while(neighborIt != neighborItEnd)
  {
    if((*neighborIt) != region)
    {
      region->addNeighbour((*neighborIt));
      (*neighborIt)->addNeighbour(region);
      (*neighborIt)->removeNeighbour(merged);
    }

    ++neighborIt;
  }

  region->removeNeighbour(merged);
}

void MultiSeg::updateRegionStatistics(const TePDITypes::TePDIRasterPtrType& image)
{
  assert(image->params().nlines_ == m_labelledImage->params().nlines_);
  assert(image->params().ncols_  == m_labelledImage->params().ncols_);

  int nBands = m_bands.size();

  // Statistics to be updated
  std::vector<double> mean;
  mean.resize(nBands, 0.0);

  std::vector<double> variance;
  variance.resize(nBands, 0.0);

  std::vector<double> cv;
  cv.resize(nBands, 0.0);

  // To gets the pixel values
  std::vector<double> pixel;
  pixel.resize(nBands, 0.0);

  double idValue = 0.0;
  double pixelValue = 0.0;
  bool valueWasRead;

  std::map<std::size_t, Region*>::iterator regionsIt = m_regions.begin();
  std::map<std::size_t, Region*>::iterator regionsItEnd = m_regions.end();

  std::size_t nRegions = 0;
  TePDIPIManager progress("Updating Regions Statistics - Level " + Te2String(m_currentLevel), m_regions.size(), progress_enabled_);

  while(regionsIt != regionsItEnd) // for each region
  {
    Region* currentRegion = regionsIt->second;

    for(int b = 0; b < nBands; ++b)
    {
      mean[b] = 0.0;
      variance[b] = 0.0;
    }

    // Region pixel values (samples)
    std::vector<std::vector<double> > regionPixels;

    // Region size
    std::size_t regionSize = 0;

    // Analysing in the region boundaries to compute the new statistics values
    for(std::size_t lin = currentRegion->getYStart(); lin < currentRegion->getYBound(); ++lin)
    {
      for(std::size_t col = currentRegion->getXStart(); col < currentRegion->getXBound(); ++col)
      {
        valueWasRead = m_labelledImage->getElement(col, lin, idValue);
        assert(valueWasRead);

        // Assert that the read value is a valid region id
        assert(m_regions.find(static_cast<std::size_t>(idValue)) != m_regions.end());

        // The current pixel composes the region?
        if(idValue != currentRegion->getId())
          continue;

        for(int b = 0; b < nBands; ++b)
        {
          valueWasRead = image->getElement(col, lin, pixelValue, m_bands[b]);
          assert(valueWasRead);

          mean[b] += pixelValue;

          pixel[b] = pixelValue;
        }
        
        ++regionSize;

        if(regionPixels.size() <= m_nSamples)
          regionPixels.push_back(pixel);
      }
    }

    if(regionSize == 0)
    {
      ++regionsIt; // next region!
      removeRegion(currentRegion, true);
      continue;
    }

    // Compute mean
    for(int b = 0; b < nBands; ++b)
      mean[b] /= static_cast<double>(regionSize);

    currentRegion->setMean(mean);
    currentRegion->setSize(regionSize);

    // Compute variance (from samples)
    for(std::size_t i = 0; i < regionPixels.size(); ++i) // for each pixel of region
    {
      const std::vector<double>& currentPixel = regionPixels[i];
      for(int b = 0; b < nBands; ++b)
        variance[b] += ((currentPixel[b] - mean[b]) * (currentPixel[b] - mean[b]));
    }

    for(int b = 0; b < nBands; ++b)
    {
      variance[b] /= static_cast<double>(regionPixels.size());

      if(mean[b] != 0.0)
        cv[b] = sqrt(variance[b]) / mean[b];
      else
        cv[b] = 0.0;
    }

    currentRegion->setVariance(variance);
    currentRegion->setCV(cv);

    ++regionsIt; // next region!

    progress.Update(++nRegions);
  }
}

void MultiSeg::adjustRegionBorders(const TePDITypes::TePDIRasterPtrType& image)
{
  std::map<std::size_t, Region*>::iterator regionsIt = m_regions.begin();
  std::map<std::size_t, Region*>::iterator regionsItEnd = m_regions.end();

  Pixels alreadyAdjustedPixels;

  std::size_t nRegions = 0;
  TePDIPIManager progress("Adjusting Regions Borders - Level " + Te2String(m_currentLevel), m_regions.size(), progress_enabled_);

  while(regionsIt != regionsItEnd) // for each region
  {
    adjustRegionBorders(regionsIt->second, image, alreadyAdjustedPixels);
    ++regionsIt;

    progress.Update(++nRegions);
  }
}

void MultiSeg::adjustRegionBorders(Region* region,
                                   const TePDITypes::TePDIRasterPtrType& image,
                                   Pixels& alreadyAdjustedPixels)
{
  assert(region);

  const std::size_t startLine = region->getYStart();
  const std::size_t startCol = region->getXStart();
  const std::size_t nLines = region->getYBound();
  const std::size_t nCols = region->getXBound();

  const std::size_t lastLine = m_labelledImage->params().nlines_ - 1;
  const std::size_t lastCol  = m_labelledImage->params().ncols_ - 1;

  double idValue = 0.0;
  bool valueWasRead;

  // Analysing in the region boundaries to adjust the borders
  for(std::size_t lin = startLine; lin < nLines; ++lin)
  {
    for(std::size_t col = startCol; col < nCols; ++col)
    {
      // The current pixel was already adjusted?
      if(alreadyAdjustedPixels.find(std::make_pair(lin, col)) != alreadyAdjustedPixels.end())
        continue;

      valueWasRead = m_labelledImage->getElement(col, lin, idValue);
      assert(valueWasRead);

      // Assert that the read value is a valid region id
      assert(m_regions.find(static_cast<std::size_t>(idValue)) != m_regions.end());

      // The current pixel composes the region?
      if(idValue != region->getId())
        continue;

      /* Verifying if the current pixel is a border pixel */

      // Output neighbour parameters
      std::size_t neighbourLin;
      std::size_t neighbourCol;
      std::size_t neighbourRegionId;
      BorderPixelType borderType;

      // Verify!
      bool isBorder = isBorderPixel(lin, col, region->getId(),
                                    neighbourLin, neighbourCol, neighbourRegionId,
                                    borderType, lastLine, lastCol);
      if(!isBorder)
        continue; // next pixel of region!

      // The current border pixel was already adjusted?
      if(alreadyAdjustedPixels.find(std::make_pair(neighbourLin, neighbourCol)) != alreadyAdjustedPixels.end())
        continue;

      // Gets the neighbour region
      Region* neighbourRegion = getRegion(neighbourRegionId);
      assert(neighbourRegion);

      if(!region->isNeighbour(neighbourRegion))
      {
        region->addNeighbour(neighbourRegion);
        neighbourRegion->addNeighbour(region);
      }

      // The current pixel is a border pixel. Is necessary an adjustment?
      std::size_t destiny = computeBorderDestiny(lin, col, region,
                                                 neighbourLin, neighbourCol, neighbourRegion,
                                                 image);

      if(destiny == std::string::npos)
      {
        // No winner!
        alreadyAdjustedPixels.insert(std::make_pair(lin, col));
        alreadyAdjustedPixels.insert(std::make_pair(neighbourLin, neighbourCol));
        continue;
      }

      if(destiny == neighbourRegion->getId())
        continue; // Will be adjusted on a next iteration

      assert(destiny == region->getId());

      // Adjusted!
      m_labelledImage->setElement(neighbourCol, neighbourLin, destiny);

      // Border already adjusted!
      alreadyAdjustedPixels.insert(std::make_pair(lin, col));
      alreadyAdjustedPixels.insert(std::make_pair(neighbourLin, neighbourCol));

      switch(borderType)
      {
        case Top:
          assert(neighbourLin < lin);
          region->updateYStart(neighbourLin);
        break;

        case Bottom:
          assert(neighbourLin > lin);
          region->updateYBound(neighbourLin + 1);
        break;

        case Left:
          assert(neighbourCol < col);
          region->updateXStart(neighbourCol);
        break;

        case Right:
          assert(neighbourCol > col);
          region->updateXBound(neighbourCol + 1);
        break;
      }
    }
  }
}

bool MultiSeg::isBorderPixel(const std::size_t& lin, const std::size_t& col, const std::size_t& regionId,
                             std::size_t& neighbourLin, std::size_t& neighbourCol, std::size_t& neighbourRegionId,
                             BorderPixelType& neighbourPixelType,
                             const std::size_t& lastLin, const std::size_t& lastCol)
{
  double nid = 0.0;
  bool valueWasRead;

  // Try get the value of (Left) pixel
  if(col)
  {
    valueWasRead = m_labelledImage->getElement(col - 1, lin, nid);
    assert(valueWasRead);

    // Assert that the read value is a valid region id
    assert(m_regions.find(static_cast<std::size_t>(nid)) != m_regions.end());

    if(static_cast<std::size_t>(nid) != regionId)
    {
      neighbourLin = lin;
      neighbourCol = col - 1;
      neighbourRegionId = static_cast<std::size_t>(nid);
      neighbourPixelType = Left;
      return true;
    }
  }

  // Try get the value of (Right) pixel
  if(col < lastCol)
  {
    valueWasRead = m_labelledImage->getElement(col + 1, lin, nid);
    assert(valueWasRead);

    // Assert that the read value is a valid region id
    assert(m_regions.find(static_cast<std::size_t>(nid)) != m_regions.end());

    if(static_cast<std::size_t>(nid) != regionId)
    {
      neighbourLin = lin;
      neighbourCol = col + 1;
      neighbourRegionId = static_cast<std::size_t>(nid);
      neighbourPixelType = Right;
      return true;
    }
  }

  // Try gets the value of (Top) pixel
  if(lin)
  {
    valueWasRead = m_labelledImage->getElement(col, lin - 1, nid);
    assert(valueWasRead);

    // Assert that the read value is a valid region id
    assert(m_regions.find(static_cast<std::size_t>(nid)) != m_regions.end());

    if(static_cast<std::size_t>(nid) != regionId)
    {
      neighbourLin = lin - 1;
      neighbourCol = col;
      neighbourRegionId = static_cast<std::size_t>(nid);
      neighbourPixelType = Top;
      return true;
    }
  }

  // Try gets the value of (Bottom) pixel
  if(lin < lastLin)
  {
    valueWasRead = m_labelledImage->getElement(col, lin + 1, nid);
    assert(valueWasRead);

    // Assert that the read value is a valid region id
    assert(m_regions.find(static_cast<std::size_t>(nid)) != m_regions.end());

    if(static_cast<std::size_t>(nid) != regionId)
    {
      neighbourLin = lin + 1;
      neighbourCol = col;
      neighbourRegionId = static_cast<std::size_t>(nid);
      neighbourPixelType = Bottom;
      return true;
    }
  }

  return false; // no border pixel!
}

std::size_t MultiSeg::computeBorderDestiny(const std::size_t& linA, const std::size_t& colA, Region* rA,
                                           const std::size_t& linB, const std::size_t& colB, Region* rB,
                                           const TePDITypes::TePDIRasterPtrType& image)
{
  assert(rA);
  assert(rB);

  // Gets the values of border pixels
  std::vector<double> pixelAValues;
  pixelAValues.resize(m_bands.size(), 0.0);

  std::vector<double> pixelBValues;
  pixelBValues.resize(m_bands.size(), 0.0);

  getPixelValues(linA, colA, pixelAValues, image);
  getPixelValues(linB, colB, pixelBValues, image);

  double VAa = m_merger->getDissimilarity(pixelAValues, rA);
  double VBa = m_merger->getDissimilarity(pixelAValues, rB);

  double VBb = m_merger->getDissimilarity(pixelBValues, rB);
  double VAb = m_merger->getDissimilarity(pixelBValues, rA);

  if(VAa < VBa && VBb >= VAb)
    return rA->getId();

  if(VAa > VBa && VBb <= VAb)
    return rB->getId();

  return std::string::npos;
}

void MultiSeg::splitRegions(const TePDITypes::TePDIRasterPtrType& image, std::map<std::size_t, Region*>& newRegions)
{
  // The current regions
  std::map<std::size_t, Region*> currentRegions = m_regions;

  std::map<std::size_t, Region*>::iterator regionsIt = currentRegions.begin();
  std::map<std::size_t, Region*>::iterator regionsItEnd = currentRegions.end();

  // Round ENL value to get CV from table
  std::size_t enl = static_cast<std::size_t>(m_currentENL);

  std::size_t numberOfHomogenousRegions = 0;

  std::size_t nRegions = 0;
  TePDIPIManager progress("Split Heterogeneous Regions - Level " + Te2String(m_currentLevel), m_regions.size(), progress_enabled_);

  while(regionsIt != regionsItEnd) // for each region
  {
    Region* currentRegion = regionsIt->second;

    // In this case, extracts the coefficient of variation from table
    if(m_imageType == Radar && m_imageModel == Cartoon)
    {
      std::size_t size = currentRegion->getSize();
      m_currentCV = m_cvTable.getCV(enl, size);
      m_merger->setParam("cv_threshold", m_currentCV);
    }

    if(m_merger->isHomogenous(currentRegion))
    {
      ++numberOfHomogenousRegions;
      ++regionsIt;  // next region!

      progress.Update(++nRegions);

      continue;
    }

    ++regionsIt; // next region!

    splitRegion(currentRegion, image, newRegions);

    progress.Update(++nRegions);
  }
}

void MultiSeg::invalidateRegionPixels(Region* region)
{
  assert(region);

  double idValue = 0.0;
  bool valueWasRead;

  for(std::size_t lin = region->getYStart(); lin < region->getYBound(); ++lin)
  {
    for(std::size_t col = region->getXStart(); col < region->getXBound(); ++col)
    {
      valueWasRead = m_labelledImage->getElement(col, lin, idValue);
      assert(valueWasRead);

      // The current pixel composes the region?
      if(idValue != region->getId()) 
        continue;

      m_labelledImage->setElement(col, lin, std::string::npos); // Invalidated!
    }
  }
}

void MultiSeg::splitRegion(Region* region, const TePDITypes::TePDIRasterPtrType& image, std::map<std::size_t, Region*>& newRegions)
{
  assert(region);

  // First, invalidate the region pixels
  invalidateRegionPixels(region);

  double idValue = 0.0;
  double pixelValue = 0.0;
  bool valueWasRead;

  int nBands = m_bands.size();

  std::vector<double> pixel;
  pixel.resize(nBands, 0.0);

  // The last region id
  std::map<std::size_t, Region*>::reverse_iterator it = m_regions.rbegin();
  assert(it->second);
  std::size_t lastId = it->second->getId();

  const std::size_t lastLine = m_labelledImage->params().nlines_ - 1;
  const std::size_t lastCol  = m_labelledImage->params().ncols_ - 1;

  for(std::size_t lin = region->getYStart(); lin < region->getYBound(); ++lin)
  {
    for(std::size_t col = region->getXStart(); col < region->getXBound(); ++col)
    {
      valueWasRead = m_labelledImage->getElement(col, lin, idValue);
      assert(valueWasRead);

      // The current pixel composes the region? Remember: the region was invalidated!
      if(idValue != std::string::npos)
        continue;

      for(int b = 0; b < nBands; ++b)
      {
        image->getElement(col, lin, pixelValue, m_bands[b]);
        pixel[b] = pixelValue;
      }

      // Generates an id for the new region
      std::size_t id = ++lastId;

      // Initializing the new region
      Region* newRegion = new Region(id, pixel, lin, col);

      // Indexing...
      assert(m_regions.find(id) == m_regions.end());
      m_regions[id] = newRegion;

      // It is a new region!
      assert(newRegions.find(id) == newRegions.end());
      newRegions[id] = newRegion;

      // Writing the new region body
      m_labelledImage->setElement(col, lin, id);

      // Building the neighborhood information
      if(lin)
      {
        valueWasRead = m_labelledImage->getElement(col, lin - 1, idValue);
        assert(valueWasRead);

        if(idValue != std::string::npos)
        {
          Region* neighbour = getRegion(static_cast<std::size_t>(idValue));
          assert(neighbour);
          newRegion->addNeighbour(neighbour);
          neighbour->addNeighbour(newRegion);
        }
      }

      if(col)
      {
        valueWasRead = m_labelledImage->getElement(col - 1, lin, idValue);
        assert(valueWasRead);

        if(idValue != std::string::npos)
        {
          Region* neighbour = getRegion(static_cast<std::size_t>(idValue));
          assert(neighbour);
          newRegion->addNeighbour(neighbour);
          neighbour->addNeighbour(newRegion);
        }
      }

      if(lin < lastLine)
      {
        valueWasRead = m_labelledImage->getElement(col, lin + 1, idValue);
        assert(valueWasRead);

        if(idValue != std::string::npos)
        {
          Region* neighbour = getRegion(static_cast<std::size_t>(idValue));
          assert(neighbour);
          newRegion->addNeighbour(neighbour);
          neighbour->addNeighbour(newRegion);
        }
      }

      if(col < lastCol)
      {
        valueWasRead = m_labelledImage->getElement(col + 1, lin, idValue);
        assert(valueWasRead);

        if(idValue != std::string::npos)
        {
          Region* neighbour = getRegion(static_cast<std::size_t>(idValue));
          assert(neighbour);
          newRegion->addNeighbour(neighbour);
          neighbour->addNeighbour(newRegion);
        }
      }
    }
  }

  // Removes the splitted region
  removeRegion(region);
}

void MultiSeg::processSmallRegions()
{
  std::size_t mergedRegions;

  if(m_minArea > 1)
  {
    while(true)
    {
      mergedRegions = mergeSmallRegions();
      if(mergedRegions == 0)
        break;
    }
  }
}

void MultiSeg::updateLabelledImage(Region* region, Region* merged)
{
  std::size_t regionId = region->getId();
  std::size_t mergedId = merged->getId();

  double value = 0.0;
  bool valueWasRead;

  for(std::size_t lin = merged->getYStart(); lin < merged->getYBound(); ++lin)
  {
    for(std::size_t col = merged->getXStart(); col < merged->getXBound(); ++col)
    {
      valueWasRead = m_labelledImage->getElement(col, lin, value);
      assert(valueWasRead);

      if(value == mergedId)
        m_labelledImage->setElement(col, lin, static_cast<double>(regionId));
    }
  }
}

void MultiSeg::removeRegion(Region* region, bool linkNeighbourhood)
{
  std::list<Region*>::iterator neighborIt = region->getNeighbours().begin();
  std::list<Region*>::iterator neighborItEnd = region->getNeighbours().end();

  while(neighborIt != neighborItEnd) // for each neighbor
  {
    (*neighborIt)->removeNeighbour(region);

    if(linkNeighbourhood || (*neighborIt)->getNeighbours().empty())
    {
      std::list<Region*>::iterator nnIt = region->getNeighbours().begin();
      std::list<Region*>::iterator nnItEnd = region->getNeighbours().end();

      while(nnIt != nnItEnd)
      {
        if((*neighborIt) != (*nnIt))
        {
          (*neighborIt)->addNeighbour(*nnIt);
          (*nnIt)->addNeighbour(*neighborIt);
        }

        ++nnIt;
      }
    }

    ++neighborIt;
  }

  m_regions.erase(region->getId());

  delete region;
}

void MultiSeg::getPixelValues(const std::size_t& lin, const std::size_t& col,
                              std::vector<double>& pixel,
                              const TePDITypes::TePDIRasterPtrType& image)
{
  double value = 0.0;
  bool valueWasRead;

  int nBands = m_bands.size();

  for(int b = 0; b < nBands; ++b)
  {
    valueWasRead = image->getElement(col, lin, value, m_bands[b]);
    assert(valueWasRead);
    pixel[b] = value;
  }
}

void MultiSeg::resizeRegions()
{
  std::map<std::size_t, Region*>::iterator it;
  for(it = m_regions.begin(); it != m_regions.end(); ++it)
    it->second->updateBounds(2, m_labelledImage->params().nlines_, m_labelledImage->params().ncols_);
}

void MultiSeg::updateThresholds(const std::size_t& currentLevel)
{
  m_currentLevel = currentLevel;

  double lag01 = ((std::pow(2.0, static_cast<double>(currentLevel)) - 1.0) / (std::pow(2.0, static_cast<double>(currentLevel)))) * 0.5; // * corr[0]
  double lag10 = ((std::pow(2.0, static_cast<double>(currentLevel)) - 1.0) / (std::pow(2.0, static_cast<double>(currentLevel)))) * 0.5; // * corr[1]
  double lag11 = ((std::pow(2.0, static_cast<double>(currentLevel)) - 1.0) / (std::pow(2.0, static_cast<double>(currentLevel)))) * 0.5; // * corr[2]

  // Computes the current similarity threshold
  m_currentSimilarity = (m_similarity / std::pow(4.0, static_cast<double>(currentLevel))) *
                        (1 + (2 * (lag01 + lag10 + lag11)));

  // Informs the current merger
  m_merger->setParam("euclidean_distance_threshold", m_currentSimilarity);

  // Computes the current ENL
  m_currentENL = (m_ENL * std::pow(4.0, static_cast<double>(currentLevel)))
                 / (1 + (2 * (lag01 + lag10 + lag11)));

  // Range (0, 250]
  m_currentENL = (std::min)(static_cast<std::size_t>(m_currentENL), static_cast<std::size_t>(250));

  // Computes the current coefficient of variation threshold
  m_currentCV = (m_cv / std::pow(4.0, static_cast<double>(currentLevel))) *
                (1 + (2 * (lag01 + lag10 + lag11)));

  // Informs the current merger
  m_merger->setParam("cv_threshold", m_currentCV);

  m_nSamples = m_nSamples * 4;

  if(m_imageType == Radar && m_imageModel == Cartoon)
  {
    boost::math::gamma_distribution<double> gamma(m_currentENL, 1.0);

    double vcritic = boost::math::quantile(gamma, m_confidenceLevel) / m_currentENL;

    // Informs the current merger
    m_merger->setParam("vcritic_factor", vcritic);
    m_merger->setParam("ENL", m_currentENL);
  }

  if(m_imageType == Optical && m_imageModel == Cartoon)
  {
    TePDIStatistic* stats = m_pyramid->buildStats(m_currentLevel);
    for(std::size_t i = 0; i < m_bands.size(); ++i)
    {
      std::string name = "image_variance_" + Te2String(i);
      m_merger->setParam(name, stats->getVariance(i)); 
    }
  }
}

void MultiSeg::notifyResult()
{
  for(std::size_t i = 0; i < m_outputters.size(); ++i)
    m_outputters[i]->output(*this, m_currentLevel);
}
