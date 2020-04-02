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
  \file MultiSegOp.cpp

  \brief This file implements the MultiSeg Operator for InterIMAGE application.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg Operator
#include "MultiSegOp.h"
#include "OpSupportFunctions.hpp"

// MultiSeg
#include <mseg/MultiSeg.h>

// TerraLib PDI
#include <terralib/image_processing/TePDIUtils.hpp>

int mseg::op::Run(int argc, char** argv)
{
  if(argc == 1)
  {
    std::cout << std::endl << "MultiSeg Segmenter Operator - Version " + OpSupportFunctions::getMultiSegVersion() << std::endl;
    return EXIT_SUCCESS;
  }

  FILE* stdoutfile = 0;
  FILE* stderrfile = 0;

  try
  {
    /* Extracting parameters */

    int param_index = 1;

    // Input Image
    std::string input_image_file_name = argv[ param_index ];
    ++param_index;

    TEAGN_TRUE_OR_THROW( input_image_file_name.find( ";" ) == input_image_file_name.npos, "Only one input image file must be used" )

    // Bounds
    const double geoWest = atof( argv[ param_index ] );
    ++param_index;

    const double geoNorth = atof( argv[ param_index ] );
    ++param_index;

    const double geoEast = atof( argv[ param_index ] );
    ++param_index;
    
    const double geoSouth = atof( argv[ param_index ] );
    ++param_index;

    // Mask File
    const std::string mask_file_name = argv[ param_index ];
    ++param_index;

    // Temp Directory
    const std::string tmpdir = argv[ param_index ];
    ++param_index;

    // Fuzzy Sets
    const std::string fuzzysets = argv[ param_index ];
    ++param_index;

    /* MultiSeg Specific Parameters */

    // Input bands
    const std::string str_input_bands = argv[ param_index ];
    ++param_index;

    // Image Format (dB, Amplitude or Intensity)
    const std::string str_image_format = argv[ param_index ];
    ++param_index;

    // Levels
    const int levels = atoi( argv[ param_index ] );
    ++param_index;

    // Similarity
    const double similarity = atof( argv[ param_index ] );
    ++param_index;

    // ENL
    const int enl = atoi( argv[ param_index ] );
    ++param_index;

    // Confidence Level
    const double confidenceLevel = atof( argv[ param_index ] );
    ++param_index;

    // Minimum Area
    const int area_min = atoi( argv[ param_index ] );
    ++param_index;

    // Output file
    std::string output_polygons_file_name = argv[ param_index ];
    ++param_index;

    // Node Class
    const std::string node_class = argv[ param_index ];
    ++param_index;

    // Node Weight
    const double node_weight = atof( argv[ param_index ] );
    ++param_index;

    // cmd
    const std::string cmd = argv[ param_index ];
    ++param_index;

    if (cmd.length() > 0)
      output_polygons_file_name += "_fs";

    /* Redirecting std out and std error to files */

    stdoutfile = freopen( ( output_polygons_file_name + 
      "_stdout.txt" ).c_str(), "w", stdout );
    stderrfile = freopen( ( output_polygons_file_name + 
      "_stderr.txt" ).c_str(), "w", stderr );     

    // Checking if we need to segment the image
    const std::string sharedDesFileToken( std::string( "class=\"" ) + "TASEGMENTERUNKNOWN" + std::string( "\"" ) );
    const std::string myDesFileToken( std::string( "class=\"" ) + node_class + "\"" );

    // Extract input bands from pattern string
    std::vector<std::string> tmp_vector;
    std::vector<std::size_t> input_bands;

    TeSplitString( str_input_bands, ",", tmp_vector );
    TEAGN_TRUE_OR_THROW( ( tmp_vector.size() > 0 ), "Invalid number of image bands" );

    for(unsigned i = 0; i < tmp_vector.size(); i++)
      input_bands.push_back( atoi( tmp_vector[i].c_str() ) );

    /* Other global vars */
    const std::string output_image_file_name = output_polygons_file_name + ".plm";
    const std::string temp_output_image_file_name = output_image_file_name + ".pgm";

    /* Initiating input_raster_ptr */

    TePDITypes::TePDIRasterPtrType input_raster_ptr(new TeRaster( input_image_file_name, 'r' ) ) ;
    TEAGN_TRUE_OR_THROW( input_raster_ptr->init(), "Unable to get input_raster" );

    TEAGN_DEBUG_CONDITION( OpSupportFunctions::createTIFFFile(
      output_image_file_name + "_input_raster.tif", input_raster_ptr, 
      input_raster_ptr->params().dataType_[ 0 ] ),
      "Error writing tif" )

    TePDITypes::TePDIRasterPtrType input_raster_clip_ptr;
    TEAGN_TRUE_OR_THROW( OpSupportFunctions::createRasterClip(input_raster_ptr, geoWest, geoNorth,
                                                                                geoEast, geoSouth, input_raster_clip_ptr), "Error clipping raster" )

    TEAGN_DEBUG_CONDITION( OpSupportFunctions::createTIFFFile(
      output_image_file_name + "_input_raster_clip.tif", 
      input_raster_clip_ptr, 
      input_raster_clip_ptr->params().dataType_[ 0 ] ),
      "Error writing tif" )
             
    /* Initiating mask raster */
         
    TePDITypes::TePDIRasterPtrType mask_raster_ptr;
    bool use_mask = false;
    if( ! mask_file_name.empty() ) {
      TEAGN_TRUE_OR_THROW( OpSupportFunctions::getMaskRaster( 
        mask_file_name, geoWest, geoNorth, geoEast, geoSouth,
        mask_raster_ptr ), "Unable to get mask image" );
          
      TEAGN_DEBUG_CONDITION( OpSupportFunctions::createTIFFFile(
        output_image_file_name + "_mask_raster.tif", mask_raster_ptr, 
        mask_raster_ptr->params().dataType_[ 0 ] ),
        "Error writing tif" )              
          
      use_mask = true;
    }

    /* Bring all rasters to same dimentions */
      
    if( use_mask ) {
      TePDITypes::TePDIRasterVectorType input_rasters_vec;
      input_rasters_vec.push_back( input_raster_clip_ptr );
        
      TePDITypes::TePDIRasterVectorType output_rasters_vec;
              
      TEAGN_TRUE_OR_THROW( OpSupportFunctions::resampleRasters(
        mask_raster_ptr->params().nlines_,
        mask_raster_ptr->params().ncols_,
        input_rasters_vec, output_rasters_vec ),
        "Error resampling rasters" )
          
      input_raster_clip_ptr = output_rasters_vec[ 0 ];
        
      TEAGN_DEBUG_CONDITION( OpSupportFunctions::createTIFFFile(
        output_image_file_name + "_input_raster_clip_resampled.tif", 
        input_raster_clip_ptr, 
        input_raster_clip_ptr->params().dataType_[ 0 ] ),
        "Error writing tif" )         
    }

    TePDIParameters msegParameters;

    msegParameters.SetParameter( "input_image", input_raster_clip_ptr );
    msegParameters.SetParameter( "input_bands", input_bands );

    msegParameters.SetParameter("image_type", Radar);
    msegParameters.SetParameter("image_model", Cartoon);

    ImageFormat imageFormat = dB;
    str_image_format == "Amplitude" ? imageFormat = Amplitude : imageFormat = Intensity;

    msegParameters.SetParameter("image_radar_format", imageFormat);

    msegParameters.SetParameter("levels", (std::size_t)levels);
    msegParameters.SetParameter("similarity", (double)similarity);
    msegParameters.SetParameter("ENL", (std::size_t)enl);
    msegParameters.SetParameter("confidence_level", (double)confidenceLevel);
    msegParameters.SetParameter("min_area", (std::size_t)area_min);

    MultiSeg mseg;
    mseg.notifyIntermediateResults(true);

    TEAGN_TRUE_OR_THROW(mseg.Reset(msegParameters), "Algorithm Reset error");

    TEAGN_TRUE_OR_THROW(mseg.Apply(), "Algorithm Apply error");

    TePDITypes::TePDIRasterPtrType segmented_raster = mseg.getLabelledImage();

    TEAGN_DEBUG_CONDITION( OpSupportFunctions::createTIFFFile(
      output_image_file_name + "_segmented.tif", segmented_raster, 
      segmented_raster->params().dataType_[ 0 ] ),
      "Error writing tif" )

    /* Set all segmented_raster pixels outside mask to zero */
      
    if( use_mask )
    {
      unsigned int nlines = (unsigned int)segmented_raster->params().nlines_;
      unsigned int ncols = (unsigned int)segmented_raster->params().ncols_;
      unsigned int col = 0;
      double mask_value = 0;
        
      for( unsigned int line = 0 ; line < nlines ; ++line ) {
        for( col = 0 ; col < ncols ; ++col ) {
          if( mask_raster_ptr->getElement( col, line, mask_value, 0 ) ) {
            if( mask_value != 0.0 ) {
              segmented_raster->setElement( col, line, 0, 0 );
            }
          } else {
            segmented_raster->setElement( col, line, 0, 0 );
          }        
        }
      }
        
      TEAGN_DEBUG_CONDITION( OpSupportFunctions::createTIFFFile(
        output_image_file_name + "_segmented_masked.tif", segmented_raster, 
        segmented_raster->params().dataType_[ 0 ] ),
        "Error writing tif" )           
    }      
        
    /* Generating the classes data vector */
      
    OpSupportFunctions::ClassesDataVectorT classes_data_vector;
    {
      TEAGN_TRUE_OR_THROW( OpSupportFunctions::createClassesDataVector(
        segmented_raster, classes_data_vector ),
        "Error getting classes from label image" )

      /* Renaming classes */
        
      for(unsigned int cdvidx = 0 ; cdvidx < classes_data_vector.size(); ++cdvidx)
        classes_data_vector[ cdvidx ].class_id_ = node_class;
    }

    TEAGN_DEBUG_CONDITION( OpSupportFunctions::exportPolygons(
      classes_data_vector, output_polygons_file_name ),
      "Error saving output shapefile" )
        
    /* Generating the output image */

    TEAGN_TRUE_OR_THROW( OpSupportFunctions::saveLabeledImageFile( 
      output_image_file_name, segmented_raster, 0 ), 
      "Unable save label raster" );

      /* Generating output polygons file */
  
    TEAGN_TRUE_OR_THROW( OpSupportFunctions::createRegionsDescFile( 
      output_polygons_file_name, classes_data_vector, 
      segmented_raster->params(), 
      node_weight,
      geoWest, geoNorth, geoEast, geoSouth, mseg.getRegions() ), 
      "Unable to export polygons file" );

    if (cmd.length() > 0)
    {
      std::string old_output_polygons_file_name = 
        output_polygons_file_name.substr( 0, 
        output_polygons_file_name.length() - 3 );
        
      TEAGN_TRUE_OR_THROW( OpSupportFunctions::decision_rule( 
        output_polygons_file_name, old_output_polygons_file_name, cmd,
        fuzzysets ), "Error calling decision rule" );
    }
  }
  catch( const TeException& e )
  {
    TEAGN_LOGERR( e.message() )
    return EXIT_FAILURE;
  }    
  catch( ... )
  {
    TEAGN_LOGERR( "Unhandled exception" )
    return EXIT_FAILURE;
  }

  TEAGN_LOGMSG( "Segmentation finished" )

  return EXIT_SUCCESS;
}
