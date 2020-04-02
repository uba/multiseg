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
  \file ParallelMultiSegStrategy.cpp

  \brief Multi-threaded image segmenter MultiSeg strategy.

  \author Douglas Uba <douglas@dpi.inpe.br>

  \note It is a prototype under development!
*/

// MultiSeg
#include "ParallelMultiSegStrategy.h"

// TerraLib
#include <terralib/kernel/TeAgnostic.h>
#include <terralib/functions/TeTerralibTranslator.h>

// TerraLib PDI
#include <terralib/image_processing/TePDIParaSegRegGrowStrategy.hpp>
#include <terralib/image_processing/TePDIUtils.hpp>

// TerraLib
#include <map>

ParallelMultiSegStrategy::ParallelMultiSegStrategy(const TePDIParaSegStrategyParams& params)
  : TePDIParaSegStrategy(params),
    m_eucTreshold(0),
    m_inputProjection(0)
{
}

ParallelMultiSegStrategy::~ParallelMultiSegStrategy()
{
}

bool ParallelMultiSegStrategy::execute(const RasterDataVecT& rasterDataVector, TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& outSegsMatrix)
{
  TEAGN_DEBUG_CONDITION(rasterDataVector.size(), TR_TERRALIB("Empty data vector"))

  // Globals
  const unsigned int nLines = rasterDataVector[0].GetLines();
  const unsigned int nCols = rasterDataVector[0].GetColumns();
  const unsigned int nBands = (unsigned int)rasterDataVector.size();
  
  // Copy data from data vector to the internal raster instance
  {
    // Initiating the input raster
    if((! m_inputRasterPtr.isActive()) || (m_inputRasterPtr->params().nBands() != (int)nBands) ||
       (m_inputRasterPtr->params().nlines_ != (int)nLines) || (m_inputRasterPtr->params().ncols_ != (int)nCols))
    {
      TeRasterParams inRasterParams;
      inRasterParams.nBands(nBands);
      inRasterParams.setDataType(TeDOUBLE, -1);
      inRasterParams.setNLinesNColumns(nLines, nCols);
      inRasterParams.projection(m_inputProjection); // This method makes a copy of the given projection pointer.
      
      TEAGN_TRUE_OR_RETURN(TePDIUtils::TeAllocRAMRaster(inRasterParams, m_inputRasterPtr), TR_TERRALIB("Error creating output raster"));
    }
    
    // Copy data
    unsigned int line = 0;
    unsigned int col = 0;
    unsigned int band = 0;
    double const* linePtr = 0;
    TeDecoder& inputRasterDecoder = *(m_inputRasterPtr->decoder());
    
    for(band = 0 ; band < nBands ; ++band)
    {
      for(line = 0 ; line < nLines ; ++line)
      {
        linePtr = rasterDataVector[band][line];
      
        for(col = 0 ; col < nCols ; ++col)
        {
          TEAGN_TRUE_OR_RETURN(inputRasterDecoder.setElement(col, line, linePtr[col], band), TR_TERRALIB("Error writing input raster"))
        }
      }
    }
  }

  // Updating segmenter algorithm parameters
  std::vector<unsigned> inputBands;
  for(unsigned int band = 0 ; band < nBands ; ++band)
    inputBands.push_back(band);

  m_segParams.SetParameter("input_bands", inputBands);
  m_segParams.SetParameter("input_image", m_inputRasterPtr);
  
  // Running the algorithm
  TEAGN_TRUE_OR_RETURN(m_segmenter.Apply(m_segParams), TR_TERRALIB("Segmentation error"));
    
  // Creating the segment objects
  TEAGN_TRUE_OR_RETURN(TePDIParaSegRegGrowStrategy::createSegsBlkFromLImg<ParallelMultiSegStrategy>(rasterDataVector, *m_segmenter.getLabelledImage(), this, outSegsMatrix),
                       TR_TERRALIB("Error creating output segments"));

  return true;
}

bool ParallelMultiSegStrategy::mergeSegments(TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& centerMatrix,
                                             TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* topMatrixPtr,
                                             TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* leftMatrixPtr)
{
  return TePDIParaSegRegGrowStrategy::staticMergeSegments(centerMatrix, topMatrixPtr, leftMatrixPtr, m_eucTreshold);
}

bool ParallelMultiSegStrategy::setParameters(const TePDIParameters& params)
{
  TEAGN_TRUE_OR_RETURN(params.GetParameter("euc_treshold", m_eucTreshold), TR_TERRALIB("Missing parameter euc_treshold"));

  checkParameters(params);

  m_segParams.Clear();

  // Cartoon or Texture?
  ImageModelRepresentation imageModel;
  params.GetParameter("image_model", imageModel); /* ---> */ m_segParams.SetParameter("image_model", imageModel);

  // Radar or Optical?
  ImageType imageType;
  params.GetParameter("image_type", imageType); /* ---> */   m_segParams.SetParameter("image_type", imageType);
  
  std::size_t levels;
  params.GetParameter("levels", levels); /* ---> */  m_segParams.SetParameter("levels", levels);

  double similarity;
  params.GetParameter("similarity", similarity); /* ---> */ m_segParams.SetParameter("similarity", similarity);

  std::size_t minArea;
  params.GetParameter("min_area", minArea); /* ---> */  m_segParams.SetParameter("min_area", minArea);

  if(imageType == Radar)
  {
    // So, the input image is Radar. Which is the format? Intensity, Amplitude or dB?
    ImageFormat imageFormat;
    params.GetParameter("image_radar_format", imageFormat); /* -> */ m_segParams.SetParameter("image_radar_format", imageFormat);
  }

  if(imageType == Radar && imageModel == Cartoon)
  {
    std::size_t ENL;
    params.GetParameter("ENL", ENL); /* ---> */ m_segParams.SetParameter("ENL", ENL);

    double condifenceLevel;
    params.GetParameter("confidence_level", condifenceLevel); /* ---> */ m_segParams.SetParameter("confidence_level", condifenceLevel);
  }

  if((imageType == Radar && imageModel == Texture) || imageType == Optical)
  {
    double cv;
    params.GetParameter("cv", cv); /* ---> */ m_segParams.SetParameter("cv", cv);

    if(imageType == Optical)
    {
      double condifenceLevel;
      params.GetParameter("confidence_level", condifenceLevel); /* ---> */ m_segParams.SetParameter("confidence_level", condifenceLevel);
    }
  }

  return true;
}

double ParallelMultiSegStrategy::getMemUsageFactor() const
{
  return 1.5;
}

unsigned int ParallelMultiSegStrategy::getMinimumBlockWH() const
{
  return 1;
}

bool ParallelMultiSegStrategy::checkParameters(const TePDIParameters& params)
{
  /* Checking input parameters about image informations */

  ImageModelRepresentation model; // Cartoon or Texture?
  TEAGN_TRUE_OR_RETURN(params.GetParameter("image_model", model), TR_TERRALIB("Missing parameter: image_model"));
  
  ImageType type; // Radar or Optical?
  TEAGN_TRUE_OR_RETURN(params.GetParameter("image_type", type), TR_TERRALIB("Missing parameter: image_type"));

  if(type == Radar)
  {
    ImageFormat format; // Amplitude, Intensity or dB?
    TEAGN_TRUE_OR_RETURN(params.GetParameter("image_radar_format", format), TR_TERRALIB("Missing parameter: image_radar_format"));
  }

  std::size_t levels;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("levels", levels), TR_TERRALIB("Missing parameter: levels"));

  double similarity;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("similarity", similarity), TR_TERRALIB("Missing parameter: similarity"));
  
  std::size_t minArea;
  TEAGN_TRUE_OR_RETURN(params.GetParameter("min_area", minArea), TR_TERRALIB("Missing parameter: min_area"));

  if(type == Radar && model == Cartoon)
  {
    std::size_t enl; // Need ENL - number of looks
    TEAGN_TRUE_OR_RETURN(params.GetParameter("ENL", enl), TR_TERRALIB("Missing parameter: ENL"));
    
    double confidenceLevel; // Need confidence_level
    TEAGN_TRUE_OR_RETURN(params.GetParameter("confidence_level", confidenceLevel), TR_TERRALIB("Missing parameter: confidence_level"));
  }

  if((type == Radar && model == Texture) || type == Optical)
  {
    double cv; // Need Coefficient of variation.
    TEAGN_TRUE_OR_RETURN(params.GetParameter("cv", cv), TR_TERRALIB("Missing parameter: cv"));
    
    if(type == Optical)
    {
      double confidenceLevel; // Need confidence_level
      TEAGN_TRUE_OR_RETURN(params.GetParameter("confidence_level", confidenceLevel), TR_TERRALIB("Missing parameter: confidence_level"));
    }
  }

  TEAGN_TRUE_OR_RETURN(params.GetParameter("input_projection", m_inputProjection), TR_TERRALIB("Missing parameter: input_projection"));
  TEAGN_TRUE_OR_RETURN(m_inputProjection != 0, TR_TERRALIB("Invalid parameter: input_projection is NULLs"));

  return true;
}
