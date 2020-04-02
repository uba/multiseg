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

  \brief Main file of MultiSeg for IDL applications.
*/

// MultiSeg
#include <mseg/MultiSeg.h>

// TerraLib PDI
#include <terralib/image_processing/TePDIUtils.hpp>

// STL
#include <string>

/*
  \brief Main file of MultiSeg for IDL applications.

  \note Command-line: ./mseg-idl.exe <input-image-path> <input-bands> <image-radar-format> <number-of-levels> <similarity> <enl> <confidence-level> <min-area>

  <input-image-path>: file path of the input image.
  <input-bands>: the input bands (comma separated).
  <image-radar-format>: dB, Amplitude or Intensity.
  <number-of-levels>: Number of compression levels.
  <similarity>: Similarity value.
  <enl>: Equivalent Number of Looks value.
  <confidence-level>: Confidence Level value.
  <min-area>: Minimum area value (in pixels).

  \note Example of command-line: ./mseg-idl.exe "D:\images\palsar.tif" "0,1" "Amplitude" "5" "1.0" "8" "0.99" "20"

  <input-image-path> = "D:/images/palsar.tif"
  <input-bands>: "0,1"
  <image-radar-format>: "Amplitude"
  <number-of-levels>: "5"
  <similarity>: "1.0"
  <enl>: "8"
  <confidence-level>: "0.99"
  <min-area>: "20"
*/
int main(int argc, char** argv)
{
  std::cout << ":: MultiSeg IDL ::" << std::endl;

  if(argc != 9)
  {
    std::cout << "Invalid number of parameters." << std::endl;

    std::cout << "Try: ./mseg-idl.exe <input-image-path> <input-bands> <image-radar-format> <number-of-levels> <similarity> <enl> <confidence-level> <min-area>" << std::endl;

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return EXIT_FAILURE;
  }

  try
  {
    /* Extracting the parameters */

    int parameterIndex = 1;

    // Input Image
    std::string inputImagePath = argv[parameterIndex];
    ++parameterIndex;

    // Initiating inputImage
    TePDITypes::TePDIRasterPtrType inputImage(new TeRaster(inputImagePath, 'r'));
    TEAGN_TRUE_OR_THROW(inputImage->init(), "Unable to get input image." );

    // Input bands
    std::string inputBandsStr = argv[parameterIndex];
    ++parameterIndex;

    std::vector<std::string> auxVector;
    std::vector<std::size_t> inputBands;

    TeSplitString(inputBandsStr, ",", auxVector);
    TEAGN_TRUE_OR_THROW((auxVector.size() > 0), "Invalid number of image bands.");

    for(std::size_t i = 0; i < auxVector.size(); i++)
      inputBands.push_back(atoi(auxVector[i].c_str()));

    // Image Format (dB, Amplitude or Intensity)
    std::string imageFormatStr = argv[parameterIndex];
    ++parameterIndex;

    // Number of Levels
    std::size_t levels = static_cast<std::size_t>(atoi(argv[parameterIndex]));
    ++parameterIndex;

    // Similarity
    double similarity = atof(argv[parameterIndex]);
    ++parameterIndex;

    // ENL
    double enl = atof(argv[parameterIndex]);
    ++parameterIndex;

    // Confidence Level
    double confidenceLevel = atof(argv[parameterIndex]);
    ++parameterIndex;

    // Minimum Area
    std::size_t minArea = static_cast<std::size_t>(atoi(argv[parameterIndex]));
    ++parameterIndex;

    /* Bulding MultiSeg parameters */
    TePDIParameters msegParameters;

    msegParameters.SetParameter("input_image", inputImage);
    msegParameters.SetParameter("input_bands", inputBands);

    msegParameters.SetParameter("image_type", Radar);    // Here using a constant value for ImageType = Radar
    msegParameters.SetParameter("image_model", Cartoon); // Here using a constant values for ImageType = Radar

    ImageFormat imageFormat = dB;
    imageFormatStr == "amplitude" ? imageFormat = Amplitude : imageFormat = Intensity;

    msegParameters.SetParameter("image_radar_format", imageFormat);

    msegParameters.SetParameter("levels", (std::size_t)levels);
    msegParameters.SetParameter("similarity", (double)similarity);
    msegParameters.SetParameter("ENL", (std::size_t)enl);
    msegParameters.SetParameter("confidence_level", (double)confidenceLevel);
    msegParameters.SetParameter("min_area", (std::size_t)minArea);
    
    /* Executing the MultiSeg algorithm */
    MultiSeg mseg;
    TEAGN_TRUE_OR_THROW(mseg.Reset(msegParameters), "Algorithm reset error.");
    TEAGN_TRUE_OR_THROW(mseg.Apply(), "Algorithm apply error.");

    // Here we have the result!
    TePDITypes::TePDIRasterPtrType labelledImage = mseg.getLabelledImage();

    // continue... you can save the file, etc.
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
