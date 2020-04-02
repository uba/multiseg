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
  \file Main.cpp

  \brief Several examples of MultiSeg support.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include <mseg/CVTable.h>
#include <mseg/MultiSeg.h>
#include <mseg/Pyramid.h>
#include <mseg/Utils.h>

// TerraLib
#include <terralib/kernel/TeInitRasterDecoders.h>

// TerraLib PDI
#include <terralib/image_processing/TePDIUtils.hpp>

// STL
#include <iostream>

// Input data
const std::string inputData  = "./data/input/8re72.tif";

// Output directory
const std::string outputPath = "./data/results/";

void Pyramid_example()
{
  TePDITypes::TePDIRasterPtrType inputImage(new TeRaster(inputData, 'r'));
  TEAGN_TRUE_OR_THROW(inputImage->init(), "Unable to init input image");

  // Number of levels
  std::size_t nLevels = 5;

  // Build the hierarchical pyramid
  Pyramid pyramid(inputImage, nLevels);

  for(std::size_t i = 0; i <= nLevels; ++i)
    // Save level i-th to file
    TePDIUtils::TeRaster2Geotiff(pyramid.getLevel(i), outputPath + "pyramid_level_" + Te2String(i) + ".tif", false);
}

void PyramidStatistics_example()
{
  TePDITypes::TePDIRasterPtrType inputImage(new TeRaster(inputData, 'r'));
  TEAGN_TRUE_OR_THROW(inputImage->init(), "Unable to init input image");

  // All bands will be used
  std::vector<std::size_t> bands;
  for(int i = 0; i < inputImage->params().nBands(); ++i)
    bands.push_back(i);

  // Number of levels
  std::size_t nLevels = 5;

  // Build the hierarchical pyramid
  Pyramid pyramid(inputImage, nLevels, bands);

  for(std::size_t i = 0; i <= nLevels; ++i)
  {
    // Build the statistics of the i-th level
    TePDIStatistic* stats = pyramid.buildStats(i);
    
    std::cout << "* Statistics of Level " << i << std::endl;
    for(std::size_t b = 0; b < bands.size(); ++b)
    {
      std::cout << "- Mean Band " << b << ": " << stats->getMean(b) << std::endl;
      std::cout << "- Variance Band " << b << ": " << stats->getVariance(b) << std::endl << std::endl;
    }

    delete stats;
  }
}

void CVTable_example()
{
  CVTable table(0.999);

  std::size_t enl = 8;
  std::size_t nSamples = 0;

  for(std::size_t i = 1; i <= 9000; ++i)
  {
    nSamples = i;
    std::cout << "Value: " << table.getCV(enl, nSamples) << std::endl;
  }
}

void Amplitude2Intensity_example()
{
  // Amplitude image
  std::string amplitudeImagePath  = "./data/input/PALSAR_2010_2.tif";
  TePDITypes::TePDIRasterPtrType amplitudeImage(new TeRaster(amplitudeImagePath, 'r'));
  TEAGN_TRUE_OR_THROW(amplitudeImage->init(), "Unable to init amplitude image");

  // Conversion
  TePDITypes::TePDIRasterPtrType intensityImage = Utils::Amplitude2Intensity(amplitudeImage);

  // Save result to file...
  TePDIUtils::TeRaster2Geotiff(intensityImage, outputPath + "intensity_image.tif", false);
}

void MultiSeg_example()
{
  // Input Image
  TePDITypes::TePDIRasterPtrType inputImage(new TeRaster(inputData, 'r'));
  TEAGN_TRUE_OR_THROW(inputImage->init(), "Unable to init input image");

  // Input bands
  std::vector<std::size_t> bands;
  for(int i = 0; i < inputImage->params().nBands(); ++i)
    bands.push_back(static_cast<std::size_t>(i));

  // MultiSeg Parameters
  TePDIParameters params;
  params.SetParameter("input_image", inputImage);
  params.SetParameter("input_bands", bands);

  // Radar
  params.SetParameter("image_model", Cartoon);
  params.SetParameter("image_type", Radar);
  params.SetParameter("image_radar_format", Intensity);
  params.SetParameter("ENL", (double)8.0);
  params.SetParameter("levels", (std::size_t)5);
  params.SetParameter("similarity", (double)1.0);
  params.SetParameter("min_area", (std::size_t)10);
  params.SetParameter("confidence_level", (double)0.95);

  // MultiSeg Segmenter
  MultiSeg segmenter;

  // Reseting...
  TEAGN_TRUE_OR_THROW(segmenter.Reset(params), "Reset failed");

  double initTime = ((double)clock()) / ((double)CLOCKS_PER_SEC);

  // Run!
  TEAGN_TRUE_OR_THROW(segmenter.Apply(), "Apply error");

  double endTime = ((double)clock()) / ((double)CLOCKS_PER_SEC);
  
  TEAGN_WATCH(endTime - initTime);

  // Gets the labelled image final result
  const TePDITypes::TePDIRasterPtrType& labelledImage = segmenter.getLabelledImage();

  // Save the labelled image to file
  TePDIUtils::TeRaster2Geotiff(labelledImage, outputPath + "labelled_image.tif");
}

int main(int /*argc*/, char** /*argv*/)
{
  std::cout << ":: MultiSeg Examples ::" << std::endl << std::endl;

  try
  {
    // Initialize TerraLib Raster Decoders
    TeInitRasterDecoders();

    // Call the examples!
  }
  catch(const TeException& e)
  {
    std::cout << std::endl << "An exception has occurred: " << e.message() << std::endl;

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return EXIT_FAILURE;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurred!" << std::endl;

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
