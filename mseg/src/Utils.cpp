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
  \file Utils.cpp

  \brief Utility methods related with MultiSeg.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "Pyramid.h"
#include "Region.h"
//#include "FixGeometries.h"
#include "Utils.h"

// TerraLib
#include <terralib/drivers/shapelib/TeDriverSHPDBF.h>
#include <terralib/kernel/TeRaster.h>
#include <terralib/kernel/TeRasterRemap.h>
#include <terralib/image_processing/TePDIRaster2Vector.hpp>
#include <terralib/image_processing/TePDIUtils.hpp>

// Qt [Need review to remove the use of QtCore module!]
#include <QtCore/QFileInfo>

// STL
#include <algorithm>
#include <cassert>
#include <cmath>

TePDITypes::TePDIRasterPtrType Utils::Amplitude2Intensity(TePDITypes::TePDIRasterPtrType& image)
{
  TeRasterParams params = image->params();
  params.decoderIdentifier_ = "SMARTMEM";
  params.mode_ = 'w';
  params.setDataType(TeDOUBLE);

  TeRaster* intensityImage = new TeRaster(params);
  intensityImage->init();

  double value = 0.0;
  bool valueWasRead;
  bool valueWasWrite;

  for(int lin = 0; lin < image->params().nlines_; ++lin)
  {
    for(int col = 0; col < image->params().ncols_; ++col)
    {
      for(int b = 0; b < image->params().nBands(); ++b)
      {
         valueWasRead = image->getElement(col, lin, value, b);
         assert(valueWasRead);
         
         valueWasWrite= intensityImage->setElement(col, lin, value * value, b);
         assert(valueWasWrite);
      }
    }
  }

  return TePDITypes::TePDIRasterPtrType(intensityImage);
}

std::size_t Utils::GenerateId(const std::size_t& lin, const std::size_t& col, const std::size_t& nCols)
{
  return lin * nCols + col;
}

void Utils::DecodeId(const std::size_t& id, std::size_t& lin, std::size_t& col, const std::size_t& nCols)
{
  lin = static_cast<std::size_t>(floor(id / static_cast<double>(nCols)));
  col = id % nCols;
}

void Utils::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{
  std::size_t lastPos = str.find_first_not_of(delimiters, 0);

  std::size_t pos = str.find_first_of(delimiters, lastPos);

  while((std::string::npos != pos) || (std::string::npos != lastPos))
  {
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    lastPos = str.find_first_not_of(delimiters, pos);
    pos = str.find_first_of(delimiters, lastPos);
  }
}

void Utils::GenerateOutputFilesNames(const TePDIParameters& params, const std::string& inputFilePath, 
                                     std::map<OutputResultType, std::string>& outputFilesNames, const std::string& separator)
{
  QFileInfo fileInfo(inputFilePath.c_str());
  std::string baseName = fileInfo.baseName().toStdString() + separator;

  ImageType type; // Radar or Optical?
  params.GetParameter("image_type", type);
  type == Radar ? baseName += "radar" : baseName += "optical";
  baseName += separator;

  ImageModelRepresentation model; // Cartoon or Texture?
  params.GetParameter("image_model", model);
  model == Cartoon ? baseName += "cartoon" : baseName += "texture";
  baseName += separator;

  if(type == Radar)
  {
    ImageFormat format; // Amplitude, Intensity or dB?
    params.GetParameter("image_radar_format", format);

    switch(format)
    {
      case Amplitude:
        baseName += "amplitude";
      break;

      case Intensity:
        baseName += "intensity";
      break;

      case dB:
        baseName += "dB";
      break;
    }
    baseName += separator;
  }

  int precision = 3;

  // Levels
  std::size_t levels;
  params.GetParameter("levels", levels);
  baseName += QString::number(levels).toStdString();
  baseName += separator;

  // Similarity
  double similarity;
  params.GetParameter("similarity", similarity);
  baseName += QString::number(similarity, 'g', precision).toStdString();
  baseName += separator;

  if(type == Radar && model == Cartoon)
  {
    double enl; // ENL - number of looks
    params.GetParameter("ENL", enl);
    baseName += QString::number(enl, 'g', precision).toStdString();
    baseName += separator;
    
    double confidenceLevel; // Confidence Level
    params.GetParameter("confidence_level", confidenceLevel);
    baseName += QString::number(confidenceLevel, 'g', precision).toStdString();
    baseName += separator;
  }

  if((type == Radar && model == Texture) || type == Optical)
  {
    double cv; // Coefficient of Variation
    params.GetParameter("cv", cv);
    baseName += QString::number(cv, 'g', precision).toStdString();
    baseName += separator;
    
    if(type == Optical)
    {
      double confidenceLevel; // Confidence Level
      params.GetParameter("confidence_level", confidenceLevel);
      baseName += QString::number(confidenceLevel, 'g', precision).toStdString();
      baseName += separator;
    }
  }

  // Minimum Area
  std::size_t minArea;
  params.GetParameter("min_area", minArea);
  baseName += QString::number(minArea).toStdString();
  baseName += separator;

  // The results
  outputFilesNames[LabelledImage] = baseName + "labelled";
  outputFilesNames[CartoonImage] = baseName + "cartoon";
  outputFilesNames[Vector] = baseName + "vector";
}

void Utils::SaveResult(const MultiSeg& mseg, const std::string& outputDir, std::map<OutputResultType, std::string>& outputFilesNames, bool resizeResults)
{
  // Gets the current labelled image result
  TePDITypes::TePDIRasterPtrType labelledImage = mseg.getLabelledImage();

  TePDITypes::TePDIRasterPtrType li2Save = labelledImage;
  if(resizeResults)
    li2Save = Pyramid::resize(labelledImage, mseg.getInputImage()->params());

  // Saves Labelled Image
  TePDIUtils::TeRaster2Geotiff(li2Save, outputDir + "/" + outputFilesNames[LabelledImage] + ".tif");

  // Gets the regions
  const std::map<std::size_t, Region*>& regions = mseg.getRegions();

  // Vectorizing...
  TePDITypes::TePDIPolSetMapPtrType polygons(new TePDITypes::TePDIPolSetMapType);
  TePDIParameters vectorizerParams;
  vectorizerParams.SetParameter("rotulated_image", labelledImage);
  vectorizerParams.SetParameter("channel", (unsigned int)0);
  vectorizerParams.SetParameter("output_polsets", polygons);

  TePDIRaster2Vector vectorizer;
  vectorizer.ToggleProgInt(true);
  TEAGN_TRUE_OR_THROW(vectorizer.Reset(vectorizerParams), "Invalid Parameters for raster2Vector");
  TEAGN_TRUE_OR_THROW(vectorizer.Apply(), "Apply error");

  TePolygonSet geometries;
  TePDITypes::TePDIPolSetMapType::iterator it = polygons->begin();
  TePDITypes::TePDIPolSetMapType::iterator itEnd = polygons->end();
  while(it != itEnd) 
  {
    geometries.copyElements(it->second);
    ++it;
  }

  //AppFixGeometries fix;
  //fix.AppFixPolygon(geometries);

  // Saves the Vector 
  TeExportPolygonSet2SHP(geometries, outputDir + "/" + outputFilesNames[Vector]);
  
  // Cartoon Image
  TePDITypes::TePDIRasterPtrType cartoonImage;
  TeRasterParams params = labelledImage->params();
  params.nBands(mseg.getUsedBands().size() * 3); // Here, 3 = [mean; variance; cv] for each band
  params.setDataType(TeDOUBLE);
  TEAGN_TRUE_OR_THROW(TePDIUtils::TeAllocRAMRaster(params, cartoonImage), "Error creating the cartoon image.");

  // Initializes the pixels of cartoon image
  double invalidValue = -1.0;
  for(int lin = 0; lin < params.nlines_; ++lin)
    for(int col = 0; col < params.ncols_; ++col)
      for(int b = 0; b < cartoonImage->nBands(); ++b)
        cartoonImage->setElement(col, lin, invalidValue, b);

  double idValue;
  bool valueWasRead;

  std::map<std::size_t, Region*>::const_iterator itRegions;
  for(itRegions = regions.begin(); itRegions != regions.end(); ++itRegions)
  {
    Region* currentRegion = itRegions->second;
    assert(currentRegion);

    // Gets the region mean
    const std::vector<double>& mean = currentRegion->getMean();

    // Gets the region variance
    const std::vector<double>& variance = currentRegion->getVariance();

    // Gets the region cv
    const std::vector<double>& cv = currentRegion->getCV();

    for(std::size_t lin = currentRegion->getYStart(); lin < currentRegion->getYBound(); ++lin)
    { 
      for(std::size_t col = currentRegion->getXStart(); col < currentRegion->getXBound(); ++col)
      {
        valueWasRead = labelledImage->getElement(col, lin, idValue);
        assert(valueWasRead);

        if(idValue != currentRegion->getId())
          continue;

        int currentBand = 0;

        // Write mean
        for(std::size_t b = currentBand; b < mean.size(); ++b, ++currentBand)
          cartoonImage->setElement(col, lin, mean[b], currentBand);

        // Write variance
        for(std::size_t b = 0; b < variance.size(); ++b, ++currentBand)
          cartoonImage->setElement(col, lin, variance[b], currentBand);

        // Write cv
        for(std::size_t b = 0; b < cv.size(); ++b, ++currentBand)
          cartoonImage->setElement(col, lin, cv[b], currentBand);
      }
    }
  }

  TePDITypes::TePDIRasterPtrType ci2Save = cartoonImage;
  if(resizeResults)
  {
    TeRasterParams params = mseg.getInputImage()->params();
    params.decoderIdentifier_ = "SMARTMEM";
    params.mode_ = 'w';
    params.nBands(cartoonImage->params().nBands());
    params.setDataType(TeDOUBLE);

    TeRaster* ciResized = new TeRaster(params);
    ciResized->init();

    TeRasterRemap remap;
    remap.setInterpolation(1);
    remap.setInput(cartoonImage.nakedPointer());
    remap.setOutput(ciResized);
    remap.apply();

    ci2Save = TePDITypes::TePDIRasterPtrType(ciResized);
  }

  // Saves the Cartoon Image 
  TePDIUtils::TeRaster2Geotiff(ci2Save, outputDir + "/" + outputFilesNames[CartoonImage] + ".tif");
}

std::size_t Utils::ComputeMaxLevels(const std::size_t& nlines, const std::size_t& ncols, const std::size_t& minimumSize)
{
  std::size_t maxlevels = 0;

  std::size_t levelSize = (std::min)(nlines, ncols) / 2;

  while(levelSize >= minimumSize)
  {
    ++maxlevels;
    levelSize /= 2;
  }

  return maxlevels;
}
