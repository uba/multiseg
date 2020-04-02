#include "OpSupportFunctions.hpp"

#include "TerraAidaXML.hpp"
#include "Version.hpp"

#include <terralib/image_processing/TePDIRaster2Vector.hpp>
#include <terralib/image_processing/TePDIUtils.hpp>
#include <terralib/image_processing/TePDIMatrix.hpp>
#include <terralib/image_processing/TePDIStatistic.hpp>
#include <terralib/image_processing/TePDITypes.hpp>

#include <terralib/kernel/TeGeometryAlgorithms.h>
#include <terralib/kernel/TeRTree.h>
#include <terralib/kernel/TeAsciiFile.h>
//#include <terralib/kernel/TeDecoderTIFF.h>
#include <terralib/kernel/TeUtils.h>
#include <terralib/kernel/TeAgnostic.h>
#include <terralib/kernel/TeDefines.h>
#include <terralib/kernel/TeGeometry.h>

#include <shapelib/shapefil.h> // Needed to export disk polygons

#if TePLATFORM == TePLATFORMCODE_MSWINDOWS
  #include <windows.h>
#elif TePLATFORM == TePLATFORMCODE_LINUX
  #include <endian.h>
#else
  #error "ERROR: Unsupported platform"
#endif 

#include <limits.h>

namespace OpSupportFunctions
{
  ClassesDataNode::ClassesDataNode()
  {
    class_value_ = 0;
  }
  
  ClassesDataNode::~ClassesDataNode()
  {
  };
  
  bool getMaskRaster( const std::string& mask_file_name,
    double geoWest, double geoNorth,
    double geoEast, double geoSouth,
    TePDITypes::TePDIRasterPtrType& mask_raster_ptr )
  {
    TEAGN_TRUE_OR_RETURN( !mask_file_name.empty(), 
      "Invalid file name" )
    TEAGN_TRUE_OR_RETURN( ( geoNorth >= geoSouth ), "Invalid coords" );
    TEAGN_TRUE_OR_RETURN( ( geoEast >= geoWest ), "Invalid coords" );
    
    FILE* file_ptr = fopen( mask_file_name.c_str(), 
      "rb" );
    TEAGN_TRUE_OR_RETURN( file_ptr, "Error opening file" )
    
    char filetype[ 3 ];
    unsigned int lines = 0;
    unsigned int cols = 0;
    char dummy_char = 0;
    
    if( 6 != fscanf( file_ptr, "%2s%1c%u%1c%u%1c", 
      filetype, 
      &dummy_char,
      &cols,
      &dummy_char,
      &lines, 
      &dummy_char ) ) {
      
      fclose( file_ptr );
      
      TEAGN_LOG_AND_RETURN( "Invalid file header" )
    }
    
    TEAGN_TRUE_OR_RETURN( ( std::string( "P4" ) ==
      filetype ), "Invalid file type" )
    
    /* Allocating the raster */
    
    TeRasterParams internal_raster_params;
    
    internal_raster_params.setDataType( TeUNSIGNEDCHAR, -1 );
    internal_raster_params.nBands( 1 );
    internal_raster_params.boundingBoxLinesColumns( geoWest, 
      geoSouth, geoEast, geoNorth,
      lines, cols,
      TeBox::TeUPPERLEFT );
       
    TEAGN_TRUE_OR_THROW( createMemRaster( 
      internal_raster_params,
      mask_raster_ptr ), "Unable create raster" );  
      
    /* Reading data */
    
    unsigned int col = 0;
    unsigned int linedatasize = ( cols / 8 ) +
      ( ( cols % 8 ) ? 1 : 0 );
    unsigned char* linedata = new unsigned char[ linedatasize ];
    unsigned int lineindex = 0;
    TeRaster& mask_raster = (*mask_raster_ptr);
    double value = 0;
    
    const unsigned char char_1 = 1;
    const unsigned char char_2 = 2;
    const unsigned char char_4 = 4;
    const unsigned char char_8 = 8;
    const unsigned char char_16 = 16;
    const unsigned char char_32 = 32;
    const unsigned char char_64 = 64;
    const unsigned char char_128 = 128;
    
    for( unsigned line = 0 ; line < lines ; ++line )
    {
      if( 1 == fread( linedata, linedatasize, 1, file_ptr ) ) {
        col = 0;
        
        for( lineindex = 0; lineindex < linedatasize ; 
          ++lineindex ) {
          
          if( col < cols ) {
            value = ( ( linedata[ lineindex ] & char_128 ) ? 0 : 255 );
            TEAGN_TRUE_OR_THROW( mask_raster.setElement( col, line, 
              value, 0 ),
              "Error writing raster element" )
            ++col;
          }
          
          if( col < cols ) {
            value = ( ( linedata[ lineindex ] & char_64 ) ? 0 : 255 );
            TEAGN_TRUE_OR_THROW( mask_raster.setElement( col, line, 
              value, 0 ),
              "Error writing raster element" )
            ++col;
          }
          
          if( col < cols ) {
            value = ( ( linedata[ lineindex ] & char_32 ) ? 0 : 255 );
            TEAGN_TRUE_OR_THROW( mask_raster.setElement( col, line, 
              value, 0 ),
              "Error writing raster element" )
            ++col;
          }
          
          if( col < cols ) {
            value = ( ( linedata[ lineindex ] & char_16 ) ? 0 : 255 );
            TEAGN_TRUE_OR_THROW( mask_raster.setElement( col, line, 
              value, 0 ),
              "Error writing raster element" )
            ++col;
          }
          
          if( col < cols ) {
            value = ( ( linedata[ lineindex ] & char_8 ) ? 0 : 255 );
            TEAGN_TRUE_OR_THROW( mask_raster.setElement( col, line, 
              value, 0 ),
              "Error writing raster element" )
            ++col;
          }
          
          if( col < cols ) {
            value = ( ( linedata[ lineindex ] & char_4 ) ? 0 : 255 );
            TEAGN_TRUE_OR_THROW( mask_raster.setElement( col, line, 
              value, 0 ),
              "Error writing raster element" )
            ++col;
          }
          
          if( col < cols ) {
            value = ( ( linedata[ lineindex ] & char_2 ) ? 0 : 255 );
            TEAGN_TRUE_OR_THROW( mask_raster.setElement( col, line, 
              value, 0 ),
              "Error writing raster element" )
            ++col;
          }
          
          if( col < cols ) {
            value = ( ( linedata[ lineindex ] & char_1 ) ? 0 : 255 );
            TEAGN_TRUE_OR_THROW( mask_raster.setElement( col, line, 
              value, 0 ),
              "Error writing raster element" )
            ++col;
          }
        }
      } else {
        fclose( file_ptr );
        delete[] linedata;
        
        TEAGN_LOG_AND_RETURN( "Error reading data" )        
      }
    }
    
    fclose( file_ptr );
    delete[] linedata;
    
    return true;
  }
  

  bool getMaskPolygons( const TePDITypes::TePDIRasterPtrType& mask_raster_ptr,
    TePolygonSet& mask_pols )
  {
    TEAGN_TRUE_OR_THROW( mask_raster_ptr.isActive(), "Invalid pointer" )
    
    mask_pols.clear();

    TePDITypes::TePDIPolSetMapPtrType aux_output_polsets( 
      new TePDITypes::TePDIPolSetMapType );
          
    TePDIParameters algo_params;
  
    algo_params.SetParameter( "rotulated_image", mask_raster_ptr );
    algo_params.SetParameter( "output_polsets", aux_output_polsets );
    algo_params.SetParameter( "channel", (unsigned int)0 );

    TePDIRaster2Vector vectorizer_instance;
    
    TEAGN_TRUE_OR_RETURN( vectorizer_instance.Reset( algo_params ), 
      "Algorithm Reset error" ); 
          
    TEAGN_TRUE_OR_RETURN( vectorizer_instance.Apply(), 
      "Algorithm Apply error" );  
            
    TePDITypes::TePDIPolSetMapType::iterator psmapit = 
      aux_output_polsets->find( 0.0 );
    TEAGN_TRUE_OR_RETURN( ( psmapit != aux_output_polsets->end() ),
      "No mask polygons found" )
    
    mask_pols = psmapit->second;
    
    return true;
  }
  
  bool createRasterClip( 
    const TePDITypes::TePDIRasterPtrType& input_raster_ptr,
    double geoWest, double geoNorth,
    double geoEast, double geoSouth,
    TePDITypes::TePDIRasterPtrType& clip_raster_ptr )
  {
    TEAGN_TRUE_OR_RETURN( input_raster_ptr.isActive(),
      "Invalid input raster pointer" );
    TEAGN_TRUE_OR_RETURN( ( geoNorth >= geoSouth ), "Invalid coords" );
    TEAGN_TRUE_OR_RETURN( ( geoEast >= geoWest ), "Invalid coords" );      
      
    std::vector< unsigned int > channels_vec;
    
    for( unsigned int channel_idx = 0 ; 
      channel_idx < (unsigned int)input_raster_ptr->params().nBands() ;
      ++channel_idx )
    {
      channels_vec.push_back( channel_idx );
    }  
    
    return createRasterClip( input_raster_ptr, channels_vec, 
      geoWest, geoNorth, geoEast, geoSouth, clip_raster_ptr );
  }  

  
  bool createRasterClip( 
    const TePDITypes::TePDIRasterPtrType& input_raster_ptr,
    const std::vector< unsigned int >& channels_vec,
    double geoWest, double geoNorth,
    double geoEast, double geoSouth,
    TePDITypes::TePDIRasterPtrType& clip_raster_ptr )
  {
    TEAGN_TRUE_OR_RETURN( input_raster_ptr.isActive(),
      "Invalid input raster pointer" );
    TEAGN_TRUE_OR_RETURN( ( geoNorth >= geoSouth ), "Invalid coords" );
    TEAGN_TRUE_OR_RETURN( ( geoEast >= geoWest ), "Invalid coords" );      
      
    /* Defining the upper-left point and lower-right bound over input image  */

    long int ul_x = 0;
    long int ul_y = 0;
    long int lr_x = 0;
    long int lr_y = 0;
    {    
      TeCoord2D ul_point = input_raster_ptr->coord2Index( 
        TeCoord2D( geoWest, geoNorth ) );
      TeCoord2D lr_point = input_raster_ptr->coord2Index( 
        TeCoord2D( geoEast, geoSouth ) );      
        
      const double x_diff_index = lr_point.x() - ul_point.x();
      const double y_diff_index = lr_point.y() - ul_point.y();
      
      TEAGN_TRUE_OR_RETURN( ( x_diff_index >= 1.0 ),
        "Trying to clip an area smaller than 1 pixel" )
      TEAGN_TRUE_OR_RETURN( ( y_diff_index >= 1.0 ),
        "Trying to clip an area smaller than 1 pixel" )
      
      ul_x = (long int)TeRound( ul_point.x() + 0.5 );
      ul_y = (long int)TeRound( ul_point.y() + 0.5 );
      
      lr_x = (long int)TeRound( lr_point.x() - 0.5 );
      lr_y = (long int)TeRound( lr_point.y() - 0.5 );
    }
      
    const long int clip_nlines = lr_y - ul_y + 1;
    const long int clip_ncols = lr_x - ul_x + 1;  
    TEAGN_TRUE_OR_RETURN( ( clip_nlines > 0 ),
     "Invalid clipping number of lines" )
    TEAGN_TRUE_OR_RETURN( ( clip_ncols > 0 ),
     "Invalid clipping number of columns" )
           
    /* Allocating the raster clip */
    
    TeRasterParams internal_raster_params = 
      input_raster_ptr->params();
    internal_raster_params.nBands( (int)channels_vec.size() );
    internal_raster_params.boundingBoxLinesColumns( geoWest, 
      geoSouth, geoEast, geoNorth, clip_nlines, clip_ncols,
      TeBox::TeUPPERLEFT );
    internal_raster_params.setPhotometric( 
      TeRasterParams::TeMultiBand, -1 );
      
    TePDITypes::TePDIRasterPtrType internal_clip_raster_ptr;
    TEAGN_TRUE_OR_RETURN( createMemRaster( internal_raster_params, 
      internal_clip_raster_ptr ), "Unable to create raster" )        
      
    /* Copying pixels */
      
    const unsigned int clip_nbands = (unsigned int)
      internal_clip_raster_ptr->params().nBands();       
    double clip_dummy = 0;
    if( internal_clip_raster_ptr->params().useDummy_ ) {
      clip_dummy = internal_clip_raster_ptr->params().dummy_[ 0 ];
    }
    long int curr_clip_col = 0;
    long int curr_clip_line = 0;
    double value = 0.0;
    unsigned int curr_channel = 0;
    
    for( unsigned int channels_vec_idx = 0 ; 
      channels_vec_idx < clip_nbands ; 
      ++channels_vec_idx ) {
      
      curr_channel = channels_vec[ channels_vec_idx ];
      
      for( curr_clip_line = 0 ; curr_clip_line < clip_nlines ; 
        ++curr_clip_line ) {
        
        for( curr_clip_col = 0 ; curr_clip_col < clip_ncols ; 
          ++curr_clip_col ) {
          
          if( input_raster_ptr->getElement( curr_clip_col + ul_x, 
            curr_clip_line + ul_y, value, curr_channel ) ) {
            
            TEAGN_TRUE_OR_THROW( internal_clip_raster_ptr->setElement( 
              curr_clip_col, 
              curr_clip_line, value, channels_vec_idx ), 
              "Error writing clip raster" )
          } else {
            TEAGN_TRUE_OR_THROW( internal_clip_raster_ptr->setElement( 
              curr_clip_col, 
              curr_clip_line, clip_dummy, channels_vec_idx ), 
              "Error writing clip raster" )
          }
        }
      }
    }
    
    clip_raster_ptr = internal_clip_raster_ptr;
  
    return true;
  }
  
  
 bool createRAMLabeledImage( 
    std::vector< ClassesDataNode >& regions_data_vector,
    unsigned int nlines, unsigned int ncols,
    double geoWest, double geoNorth,
    double geoEast, double geoSouth,
    TePDITypes::TePDIRasterPtrType& raster_ptr )
  {
    TEAGN_TRUE_OR_RETURN( ( nlines > 0 ), "Invalid nlines" )
    TEAGN_TRUE_OR_RETURN( ( ncols > 0 ), "Invalid ncols" )
    TEAGN_TRUE_OR_RETURN( ( geoNorth >= geoSouth ), "Invalid coords" );
    TEAGN_TRUE_OR_RETURN( ( geoEast >= geoWest ), "Invalid coords" );     
    
    unsigned int rdv_size = (unsigned int)
      regions_data_vector.size();
    TePolygonSet::iterator ps_it;
    TePolygonSet::iterator ps_it_end;      
      
    /* Allocating the raster */
    
    TeRasterParams internal_raster_params;
    
    internal_raster_params.setDataType( TeINTEGER, -1 );
    internal_raster_params.nBands( 1 );
    internal_raster_params.setDummy( 0.0, -1 );
    internal_raster_params.boundingBoxLinesColumns( geoWest, 
      geoSouth, geoEast, geoNorth, (int)nlines, (int)ncols, 
      TeBox::TeUPPERLEFT );
             
    TEAGN_TRUE_OR_RETURN( createMemRaster( 
      internal_raster_params,
      raster_ptr ), "Unable to get the output raster" );
      
    /* Writing polygons */
    
    TeRaster::iteratorPoly raster_it;
    int curr_line = 0;
    int curr_col = 0;
    double llx = 0;
    double lly = 0;
    double urx = 0;
    double ury = 0;    
    
    for( unsigned int rdv_index = 0 ; rdv_index < rdv_size ; 
      ++rdv_index ) 
    {
      ClassesDataNode& current_node = regions_data_vector[ 
        rdv_index ];
        
      current_node.polsIndexedBoxes_.clear();
      
      ps_it = current_node.pols_.begin();
      ps_it_end = current_node.pols_.end();
            
      if( ps_it != ps_it_end )
      {
        TEAGN_DEBUG_CONDITION( ( current_node.class_value_ != 0 ),
          "Invalid map index value (zero used for dummy)" )   
          
        while( ps_it != ps_it_end ) 
        {
          raster_it = raster_ptr->begin( *ps_it, 
            TeBoxPixelIn, 0 );
            
          if( raster_it.end() )
          {
            current_node.polsIndexedBoxes_.push_back( TeBox( 0, 0,
              0, 0 ) );          
          }
          else
          {
            llx = DBL_MAX;
            lly = -1.0 * DBL_MAX;               
            urx = -1.0 * DBL_MAX;
            ury = DBL_MAX;          
            
            while( ! raster_it.end() ) 
            {
              curr_line = raster_it.currentLine();
              curr_col = raster_it.currentColumn();
              
              TEAGN_TRUE_OR_THROW( raster_ptr->setElement( curr_col, curr_line, 
                current_node.class_value_, 0 ), "Error writing to label image" )
              
              if( llx > curr_col ) llx = curr_col;
              if( lly < curr_line ) lly = curr_line;    
              if( urx < curr_col ) urx = curr_col;
              if( ury > curr_line ) ury = curr_line;             
              
              ++raster_it;
            }
              
            current_node.polsIndexedBoxes_.push_back( TeBox( llx, lly,
              urx, ury ) );
          }
          
          ++ps_it;
        }
      }
    }
    
    return true;
  }
  
  
  bool saveLabeledImageFile( const std::string& output_image_file_name_str,
    TePDITypes::TePDIRasterPtrType& raster_ptr, unsigned int raster_channel )
  {
    TEAGN_TRUE_OR_THROW( raster_ptr.isActive(), 
      "Invalid raster_ptr" )
    TEAGN_TRUE_OR_THROW( 
      ( raster_ptr->params().nBands() > (int)raster_channel ), 
      "Invalid channel" )      
      
    unsigned int nlines = (unsigned int)raster_ptr->params().nlines_;
    unsigned int ncols = (unsigned int)raster_ptr->params().ncols_;
    unsigned int curr_line = 0 ;
    unsigned int curr_col = 0 ;
    double curr_value_double = 0;
    int curr_value_int = 0;
    size_t sizeofint = sizeof( int );      
    
    /* Finding min and max raster values */
    
    double max_value_dbl = DBL_MAX * ( -1.0 );
    double min_value_dbl = DBL_MAX;
    
    for( curr_line = 0 ; curr_line < nlines ; ++curr_line ) 
    {
      for( curr_col = 0 ; curr_col < ncols ; ++curr_col ) 
      {
        if( ! raster_ptr->getElement( curr_col, curr_line, curr_value_double,
          raster_channel ) ) 
        {
          curr_value_double = 0;
        }
        
        if( curr_value_double > max_value_dbl ) {
          max_value_dbl = curr_value_double;  
        }
        if( curr_value_double < min_value_dbl ) {
          min_value_dbl = curr_value_double;  
        }
      }
    }
    
    unsigned int max_value_uint = (unsigned int)max_value_dbl;
    unsigned int min_value_uint = (unsigned int)min_value_dbl;
    
    /* Creating file */
      
    FILE* file_ptr = fopen( output_image_file_name_str.c_str(), 
      "wb" );
    TEAGN_TRUE_OR_RETURN( ( file_ptr != 0 ), "Cannot create file" )
    
    /* Writing header */
    
    fprintf( file_ptr, "F5\n" );
     
    #if BYTE_ORDER == LITTLE_ENDIAN
      fprintf( file_ptr, "L\n" );
    #elif BYTE_ORDER == BIG_ENDIAN
      fprintf( file_ptr, "B\n" );
    #else
      #error "ERROR: Endianess format detection error"
    #endif 
    
    fprintf( file_ptr, "%u %u\n", ncols, nlines );
    
    fprintf( file_ptr, "%u %u\n", min_value_uint, max_value_uint );
      
    /* Writing image data */
    
    for( curr_line = 0 ; curr_line < nlines ; ++curr_line ) {
      for( curr_col = 0 ; curr_col < ncols ; ++curr_col ) {
        if( raster_ptr->getElement( curr_col, curr_line, curr_value_double,
          raster_channel ) ) {
          
          curr_value_int = (int)curr_value_double;
        } else {
          curr_value_int = 0;
        }
        
        if( 1 != fwrite( &curr_value_int, sizeofint, 1, file_ptr ) ) {
          TEAGN_LOGERR( "Error wrinting file" );
          fclose( file_ptr );
          return false;
        }
      }
    }
    
    fclose( file_ptr );
    
    return true;
  }
  
  
  bool createMemRaster( const TeRasterParams& raster_params,
      TePDITypes::TePDIRasterPtrType& raster_ptr )
  {
    TeRasterParams internal_params = raster_params;
    internal_params.mode_ = 'c';
    internal_params.decoderIdentifier_ = "SMARTMEM";
    
    raster_ptr.reset( new TeRaster( internal_params ) );
    
    if( raster_ptr->init() ) {
      return true;
    } else {
      raster_ptr.reset();
      
      return false;
    }
  }
  
  
  bool createTIFFFile( const std::string& file_name,
    const TePDITypes::TePDIRasterPtrType& raster_ptr,
    TeDataType out_data_type )
  {
    TEAGN_TRUE_OR_THROW( raster_ptr.isActive(), "Invalid pointer" )
    TEAGN_TRUE_OR_THROW( ! file_name.empty(), "Invalid file name" )
  
    TeRasterParams temp_params = raster_ptr->params();

    temp_params.mode_ = 'c';
    temp_params.fileName_ = file_name;
    temp_params.decoderIdentifier_ = "TIF";
    temp_params.setDataType( out_data_type, -1 );
    
    if( ( temp_params.nBands() == 3 ) &&
      ( temp_params.photometric_[ 0 ] == TeRasterParams::TeMultiBand ) &&
      ( temp_params.photometric_[ 1 ] == TeRasterParams::TeMultiBand ) &&
      ( temp_params.photometric_[ 2 ] == TeRasterParams::TeMultiBand ) ) {
      
      temp_params.setPhotometric( TeRasterParams::TeRGB, -1 );
    }
    
    /* Creating disk output raster */
    
    TeRaster outRaster( temp_params );

    TEAGN_TRUE_OR_RETURN( outRaster.init(),
      "Unable to init GeoTIFF Raster" );  
      
    /* Copying data */
    
    unsigned int lines = (unsigned int)temp_params.nlines_;
    unsigned int cols = (unsigned int)temp_params.ncols_;
    unsigned int bands = (unsigned int)temp_params.nBands();
    unsigned int line = 0 ;
    unsigned int col = 0;
    unsigned int band = 0;
    double value = 0;
    
    for( band = 0 ; band < bands ; ++band ) {
      for( line = 0 ; line < lines ; ++line ) {
        for( col = 0 ; col < cols ; ++col ) {
          if( raster_ptr->getElement( col, line, value, band ) ) {
            TEAGN_TRUE_OR_RETURN( outRaster.setElement( col, line, value,
              band ), "Error writing raster data" )
          }
        }
      }
    }
  
    return true;
  }

  bool createRegionsDescFile( const std::string& reg_desc_file_name,
      const std::vector< ClassesDataNode >& regions_data_vector,
      const TeRasterParams& output_raster_params, 
      double node_weight,
      double geoWest, double geoNorth,
      double geoEast, double geoSouth )
  {
    TEAGN_TRUE_OR_RETURN( ( ! reg_desc_file_name.empty() ), 
      "Invalid parameter : reg_desc_file_name" )
    TEAGN_TRUE_OR_RETURN( ( geoNorth >= geoSouth ), "Invalid coords" );
    TEAGN_TRUE_OR_RETURN( ( geoEast >= geoWest ), "Invalid coords" );         
      
    if( regions_data_vector.size() == 0 ) {
      TeAsciiFile fileinstance( reg_desc_file_name, "w" );
      fileinstance.writeString( "<regions></regions>" );
    } 
    else 
    {
      const std::string output_image_file_name( reg_desc_file_name +
        ".plm" );
          
      TerraAidaXML xmlparser;
      
      TEAGN_TRUE_OR_RETURN( 
        xmlparser.InsertNode( TAG, 0, 0, "regions", "", "" ), 
        "Error inserting XML node" );      
      
      for( unsigned int regions_index = 0 ; 
        regions_index < regions_data_vector.size() ; 
        ++regions_index ) 
      {
        const ClassesDataNode& curr_class_data = 
          regions_data_vector[ regions_index ];
          
        TEAGN_DEBUG_CONDITION( curr_class_data.class_value_ != 0, 
          "Invalid polygon ID - Zero is used to indicate dummy value " );    
        TEAGN_DEBUG_CONDITION( ( curr_class_data.pols_.size() ?
          ( curr_class_data.pols_.size() == 
          curr_class_data.polsIndexedBoxes_.size() ) : true ), 
          "Size mismatch between polygons vector and polygons indexed "
          "boxes vector" );
        
        const unsigned int extra_attributes_nmb =
          (unsigned int)curr_class_data.attributes_.size();
        TEAGN_DEBUG_CONDITION( ( extra_attributes_nmb ? 
          ( curr_class_data.polsIndexedBoxes_.size() == extra_attributes_nmb ) 
          : true ), 
          "Size mismatch between polygons vector and polygons "
          "attributes vector" );          

        unsigned int polindex = 0;
        unsigned int parserIdx = 0;

        std::vector< TeBox >::const_iterator pIBIt;
        std::vector< TeBox >::const_iterator pIBItEnd;
        TeBox polBoxIndexed;
        
        if( extra_attributes_nmb == 0 )
        {
          pIBIt = curr_class_data.polsIndexedBoxes_.begin();
          pIBItEnd = curr_class_data.polsIndexedBoxes_.end();
          
          if( pIBIt != pIBItEnd )
          {
            // Generating the class indexed box
            
            polBoxIndexed = *pIBIt;
            
            while( pIBIt != pIBItEnd  ) 
            {
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x1() <= pIBIt->x2() ),
                "Invalid points" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y1() >= pIBIt->y2() ),
                "Invalid points" )
                
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x1() >= 0 ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y1() >= 0 ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x1() < output_raster_params.ncols_ ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y1() < output_raster_params.nlines_ ),
                "Invalid ur point" )                
                
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x2() >= 0 ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y2() >= 0 ),
                "Invalid ur point" )              
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x2() < output_raster_params.ncols_ ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y2() < output_raster_params.nlines_ ),
                "Invalid ur point" )            
                
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x1() <= pIBIt->x2() ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y1() >= pIBIt->y2() ),
                "Invalid ur point" )                    

              updateBox( polBoxIndexed, *pIBIt );
              
              ++pIBIt;
            }
           
            /* Generating default XML nodes for this class */     
          
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( TAG, 1, parserIdx, "region", "", 
              "regions" ), "Error inserting XML node" );
              
            unsigned int attr_index = 0;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "class", 
                curr_class_data.class_id_ , "region" ), 
                "Error inserting XML node" );        
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "id", 
              Te2String( curr_class_data.class_value_ ), "region" ), 
              "Error inserting XML node" );
      
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file", 
              output_image_file_name, 
              "region" ), "Error inserting XML node" );
              
            ++attr_index;  
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "llx", Te2String( 
              (long int)polBoxIndexed.x1() ), "region" ), 
              "Error inserting XML node" );
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "lly", Te2String( 
              (long int)polBoxIndexed.y1() ), "region" ), 
              "Error inserting XML node" );        
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "urx", Te2String( 
              (long int)polBoxIndexed.x2() ), "region" ), 
              "Error inserting XML node" );
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "ury", Te2String( 
              (long int)polBoxIndexed.y2() ), "region" ), 
              "Error inserting XML node" );       
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "p", Te2String( 
              node_weight, 10 ), "region" ), 
              "Error inserting XML node" );            

            /* inserting file_geo attributes for this class */
            
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoNorth", Te2String( 
              geoNorth, 10 ), "region" ), 
              "Error inserting XML node" );            
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoSouth", Te2String( 
              geoSouth, 10 ), "region" ), 
              "Error inserting XML node" );          
            
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoWest", Te2String( 
              geoWest, 10 ), "region" ), 
              "Error inserting XML node" );           
                    
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoEast", Te2String( 
              geoEast, 10 ), "region" ), 
              "Error inserting XML node" );
          }
            
          ++parserIdx;
        }
        else // ( extra_attributes_nmb != 0 )
        {
          pIBIt = curr_class_data.polsIndexedBoxes_.begin();
          pIBItEnd = curr_class_data.polsIndexedBoxes_.end();          
          
          while( pIBIt != pIBItEnd ) 
          {
            const TeBox& polBoxIndexed = *pIBIt;
        
            /* Checking polygon indexed box coords */

            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x1() >= 0 ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y1() >= 0 ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x1() < output_raster_params.ncols_ ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y1() < output_raster_params.nlines_ ),
              "Invalid ur point" )                
              
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x2() >= 0 ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y2() >= 0 ),
              "Invalid ur point" )              
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x2() < output_raster_params.ncols_ ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y2() < output_raster_params.nlines_ ),
              "Invalid ur point" )            
              
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x1() <= polBoxIndexed.x2() ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y1() >= polBoxIndexed.y2() ),
              "Invalid ur point" )              
            
            /* Genrating default XML nodes */     
          
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( TAG, 1, parserIdx, "region", "", 
              "regions" ), "Error inserting XML node" );
              
            unsigned int attr_index = 0;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "class", 
                curr_class_data.class_id_ , "region" ), 
                "Error inserting XML node" );        
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "id", 
              Te2String( curr_class_data.class_value_ ), "region" ), 
              "Error inserting XML node" );
      
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file", 
              output_image_file_name, 
              "region" ), "Error inserting XML node" );
              
            ++attr_index;  
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "llx", 
              Te2String( (long int)polBoxIndexed.x1() ), "region" ), 
              "Error inserting XML node" );
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "lly", 
              Te2String( (long int)polBoxIndexed.y1() ), "region" ), 
              "Error inserting XML node" );        
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "urx", 
              Te2String( (long int)polBoxIndexed.x2() ), "region" ), 
              "Error inserting XML node" );
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "ury", 
              Te2String( (long int)polBoxIndexed.y2() ), "region" ), 
              "Error inserting XML node" );       

            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "p", Te2String( 
              node_weight, 10 ), "region" ), 
              "Error inserting XML node" );            
  
            /* inserting file_geo attributes */
            
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoNorth", Te2String( 
              geoNorth, 10 ), "region" ), 
              "Error inserting XML node" );            
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoSouth", Te2String( 
              geoSouth, 10 ), "region" ), 
              "Error inserting XML node" );          
            
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoWest", Te2String( 
              geoWest, 10 ), "region" ), 
              "Error inserting XML node" );           
                    
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoEast", Te2String( 
              geoEast, 10 ), "region" ), 
              "Error inserting XML node" );
  
            /* Generating extra attributes node */
            
            if( extra_attributes_nmb ) {
              const std::vector< std::pair< std::string, 
                std::string > >& curr_pol_attributes = 
                curr_class_data.attributes_[ polindex ];
                
              for( unsigned int curr_pol_attributes_index = 0 ; 
                curr_pol_attributes_index < curr_pol_attributes.size() ; 
                  ++curr_pol_attributes_index ) {
                
                const std::pair< std::string, std::string >& curr_attribute =
                  curr_pol_attributes[ curr_pol_attributes_index ];
                  
                ++attr_index;
                TEAGN_TRUE_OR_RETURN( 
                  xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, 
                    curr_attribute.first, curr_attribute.second, "region" ), 
                  "Error inserting XML node" );                
              }
            }
              
            ++parserIdx;
            ++polindex;
            ++pIBIt;
          }
        }
      }
      
      xmlparser.SetOutputFile( reg_desc_file_name );
      TEAGN_TRUE_OR_RETURN( xmlparser.SaveToDisk(), 
        "Error saving regions description file" );  
    } //if( polsetsmap_ptr->size() == 0 )
      
    return true;
  } 
  
   bool createRegionsDescFile( const std::string& reg_desc_file_name,
      const std::vector< ClassesDataNode >& regions_data_vector,
      const TeRasterParams& output_raster_params, 
      double node_weight,
      double geoWest, double geoNorth,
      double geoEast, double geoSouth, const std::map<std::size_t, Region*>& regions )
  {
    TEAGN_TRUE_OR_RETURN( ( ! reg_desc_file_name.empty() ), 
      "Invalid parameter : reg_desc_file_name" )
    TEAGN_TRUE_OR_RETURN( ( geoNorth >= geoSouth ), "Invalid coords" );
    TEAGN_TRUE_OR_RETURN( ( geoEast >= geoWest ), "Invalid coords" );         
      
    if( regions_data_vector.size() == 0 ) {
      TeAsciiFile fileinstance( reg_desc_file_name, "w" );
      fileinstance.writeString( "<regions></regions>" );
    } 
    else 
    {
      const std::string output_image_file_name( reg_desc_file_name +
        ".plm" );
          
      TerraAidaXML xmlparser;
      
      TEAGN_TRUE_OR_RETURN( 
        xmlparser.InsertNode( TAG, 0, 0, "regions", "", "" ), 
        "Error inserting XML node" );      
      
      for( unsigned int regions_index = 0 ; 
        regions_index < regions_data_vector.size() ; 
        ++regions_index ) 
      {
        const ClassesDataNode& curr_class_data = 
          regions_data_vector[ regions_index ];
          
        TEAGN_DEBUG_CONDITION( curr_class_data.class_value_ != 0, 
          "Invalid polygon ID - Zero is used to indicate dummy value " );    
        TEAGN_DEBUG_CONDITION( ( curr_class_data.pols_.size() ?
          ( curr_class_data.pols_.size() == 
          curr_class_data.polsIndexedBoxes_.size() ) : true ), 
          "Size mismatch between polygons vector and polygons indexed "
          "boxes vector" );
        
        const unsigned int extra_attributes_nmb =
          (unsigned int)curr_class_data.attributes_.size();
        TEAGN_DEBUG_CONDITION( ( extra_attributes_nmb ? 
          ( curr_class_data.polsIndexedBoxes_.size() == extra_attributes_nmb ) 
          : true ), 
          "Size mismatch between polygons vector and polygons "
          "attributes vector" );          

        unsigned int polindex = 0;
        unsigned int parserIdx = 0;

        std::vector< TeBox >::const_iterator pIBIt;
        std::vector< TeBox >::const_iterator pIBItEnd;
        TeBox polBoxIndexed;
        
        if( extra_attributes_nmb == 0 )
        {
          pIBIt = curr_class_data.polsIndexedBoxes_.begin();
          pIBItEnd = curr_class_data.polsIndexedBoxes_.end();
          
          if( pIBIt != pIBItEnd )
          {
            // Generating the class indexed box
            
            polBoxIndexed = *pIBIt;
            
            while( pIBIt != pIBItEnd  ) 
            {
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x1() <= pIBIt->x2() ),
                "Invalid points" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y1() >= pIBIt->y2() ),
                "Invalid points" )
                
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x1() >= 0 ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y1() >= 0 ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x1() < output_raster_params.ncols_ ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y1() < output_raster_params.nlines_ ),
                "Invalid ur point" )                
                
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x2() >= 0 ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y2() >= 0 ),
                "Invalid ur point" )              
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x2() < output_raster_params.ncols_ ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y2() < output_raster_params.nlines_ ),
                "Invalid ur point" )            
                
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->x1() <= pIBIt->x2() ),
                "Invalid ur point" )
              TEAGN_DEBUG_CONDITION( 
                ( pIBIt->y1() >= pIBIt->y2() ),
                "Invalid ur point" )                    

              updateBox( polBoxIndexed, *pIBIt );
              
              ++pIBIt;
            }
           
            /* Generating default XML nodes for this class */     
          
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( TAG, 1, parserIdx, "region", "", 
              "regions" ), "Error inserting XML node" );
              
            unsigned int attr_index = 0;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "class", 
                curr_class_data.class_id_ , "region" ), 
                "Error inserting XML node" );        
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "id", 
              Te2String( curr_class_data.class_value_ ), "region" ), 
              "Error inserting XML node" );
      
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file", 
              output_image_file_name, 
              "region" ), "Error inserting XML node" );
              
            // Find Region
            std::map<std::size_t, Region*>::const_iterator it = regions.find(curr_class_data.class_value_);
            TEAGN_TRUE_OR_RETURN(it != regions.end(), "Error inserting XML node - Region not found!" );

            Region* region = it->second;

            ++attr_index;  
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "llx", Te2String( 
              (long int)region->getXStart() ), "region" ), 
              "Error inserting XML node" );
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "lly", Te2String( 
              (long int)region->getYBound() ), "region" ), 
              "Error inserting XML node" );        
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "urx", Te2String( 
              (long int)region->getXBound() ), "region" ), 
              "Error inserting XML node" );
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "ury", Te2String( 
              (long int)region->getYStart() ), "region" ), 
              "Error inserting XML node" );       
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "p", Te2String( 
              node_weight, 10 ), "region" ), 
              "Error inserting XML node" );            

            /* inserting file_geo attributes for this class */
            
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoNorth", Te2String( 
              geoNorth, 10 ), "region" ), 
              "Error inserting XML node" );            
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoSouth", Te2String( 
              geoSouth, 10 ), "region" ), 
              "Error inserting XML node" );          
            
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoWest", Te2String( 
              geoWest, 10 ), "region" ), 
              "Error inserting XML node" );           
                    
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoEast", Te2String( 
              geoEast, 10 ), "region" ), 
              "Error inserting XML node" );
          }
            
          ++parserIdx;
        }
        else // ( extra_attributes_nmb != 0 )
        {
          pIBIt = curr_class_data.polsIndexedBoxes_.begin();
          pIBItEnd = curr_class_data.polsIndexedBoxes_.end();          
          
          while( pIBIt != pIBItEnd ) 
          {
            const TeBox& polBoxIndexed = *pIBIt;
        
            /* Checking polygon indexed box coords */

            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x1() >= 0 ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y1() >= 0 ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x1() < output_raster_params.ncols_ ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y1() < output_raster_params.nlines_ ),
              "Invalid ur point" )                
              
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x2() >= 0 ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y2() >= 0 ),
              "Invalid ur point" )              
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x2() < output_raster_params.ncols_ ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y2() < output_raster_params.nlines_ ),
              "Invalid ur point" )            
              
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.x1() <= polBoxIndexed.x2() ),
              "Invalid ur point" )
            TEAGN_DEBUG_CONDITION( 
              ( polBoxIndexed.y1() >= polBoxIndexed.y2() ),
              "Invalid ur point" )              
            
            /* Genrating default XML nodes */     
          
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( TAG, 1, parserIdx, "region", "", 
              "regions" ), "Error inserting XML node" );
              
            unsigned int attr_index = 0;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "class", 
                curr_class_data.class_id_ , "region" ), 
                "Error inserting XML node" );        
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "id", 
              Te2String( curr_class_data.class_value_ ), "region" ), 
              "Error inserting XML node" );
      
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file", 
              output_image_file_name, 
              "region" ), "Error inserting XML node" );
              
             // Find Region
            std::map<std::size_t, Region*>::const_iterator it = regions.find(curr_class_data.class_value_);
            TEAGN_TRUE_OR_RETURN(it != regions.end(), "Error inserting XML node - Region not found!" );

            Region* region = it->second;

            ++attr_index;  
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "llx", Te2String( 
              (long int)region->getXStart() ), "region" ), 
              "Error inserting XML node" );
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "lly", Te2String( 
              (long int)region->getYBound() ), "region" ), 
              "Error inserting XML node" );        
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "urx", Te2String( 
              (long int)region->getXBound() ), "region" ), 
              "Error inserting XML node" );
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "ury", Te2String( 
              (long int)region->getYStart() ), "region" ), 
              "Error inserting XML node" );       

            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "p", Te2String( 
              node_weight, 10 ), "region" ), 
              "Error inserting XML node" );            
  
            /* inserting file_geo attributes */
            
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoNorth", Te2String( 
              geoNorth, 10 ), "region" ), 
              "Error inserting XML node" );            
              
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoSouth", Te2String( 
              geoSouth, 10 ), "region" ), 
              "Error inserting XML node" );          
            
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoWest", Te2String( 
              geoWest, 10 ), "region" ), 
              "Error inserting XML node" );           
                    
            ++attr_index;
            TEAGN_TRUE_OR_RETURN( 
              xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, "file_geoEast", Te2String( 
              geoEast, 10 ), "region" ), 
              "Error inserting XML node" );
  
            /* Generating extra attributes node */
            
            if( extra_attributes_nmb ) {
              const std::vector< std::pair< std::string, 
                std::string > >& curr_pol_attributes = 
                curr_class_data.attributes_[ polindex ];
                
              for( unsigned int curr_pol_attributes_index = 0 ; 
                curr_pol_attributes_index < curr_pol_attributes.size() ; 
                  ++curr_pol_attributes_index ) {
                
                const std::pair< std::string, std::string >& curr_attribute =
                  curr_pol_attributes[ curr_pol_attributes_index ];
                  
                ++attr_index;
                TEAGN_TRUE_OR_RETURN( 
                  xmlparser.InsertNode( ATTRIBUTE, 1, attr_index, 
                    curr_attribute.first, curr_attribute.second, "region" ), 
                  "Error inserting XML node" );                
              }
            }
              
            ++parserIdx;
            ++polindex;
            ++pIBIt;
          }
        }
      }
      
      xmlparser.SetOutputFile( reg_desc_file_name );
      TEAGN_TRUE_OR_RETURN( xmlparser.SaveToDisk(), 
        "Error saving regions description file" );  
    } //if( polsetsmap_ptr->size() == 0 )
      
    return true;
  } 
  
  bool resampleRasters( 
    unsigned int targetNLines, unsigned int targetNCols,
    const TePDITypes::TePDIRasterVectorType& in_rasters_vec,
    TePDITypes::TePDIRasterVectorType& out_rasters_vec )
  {
    const unsigned int vec_size = (unsigned int)in_rasters_vec.size();

    /* Resampling rasters to fit the target dimentions */
      
    for( unsigned int vec_index = 0 ; vec_index < vec_size ; ++vec_index ) 
    {
      const TePDITypes::TePDIRasterPtrType& curr_raster_ptr = 
        in_rasters_vec[ vec_index ];
        
      if( curr_raster_ptr.isActive() )
      {
        unsigned int curr_raster_lines = (unsigned int)
          curr_raster_ptr->params().nlines_;
        unsigned int curr_raster_cols = (unsigned int)
          curr_raster_ptr->params().ncols_;
          
        if( ( curr_raster_lines == targetNLines ) &&
          ( curr_raster_cols == targetNCols ) ) {
          
          out_rasters_vec.push_back( curr_raster_ptr );
        } 
        else 
        {
          /* Initiating the new raster in memory */
          TeRasterParams auxparams = curr_raster_ptr->params();
          auxparams.setNLinesNColumns( 1, 1 );
          auxparams.nBands( 1 );
          
          TePDITypes::TePDIRasterPtrType new_raster_ptr;
          if( ! createMemRaster( auxparams, new_raster_ptr ) ) 
          {
            TEAGN_LOGERR( "Error creating new memory raster" );
            
            return false;
          }
            
          /* Resampling */
          
          if( ! TePDIUtils::resampleRasterByLinsCols( 
            curr_raster_ptr, new_raster_ptr, targetNLines, 
            targetNCols, false, TePDIInterpolator::BicubicMethod ) ) {
            
            TEAGN_LOGERR( "Error interpolating raster" );
            
            return false;
          }
          
          TEAGN_DEBUG_CONDITION( 
            ( new_raster_ptr->params().nlines_ == (int)targetNLines ),
            "Invalid number of resampled raster lines" )
          TEAGN_DEBUG_CONDITION( 
            ( new_raster_ptr->params().ncols_ == (int)targetNCols ),
            "Invalid number of resampled raster lines" )
          
          out_rasters_vec.push_back( new_raster_ptr );
        }
      }
      else
      {
        out_rasters_vec.push_back( curr_raster_ptr );
      }
    }
    
    return true;
  }
  
  bool resampleRasters( 
    const TePDITypes::TePDIRasterVectorType& in_rasters_vec,
    TePDITypes::TePDIRasterVectorType& out_rasters_vec )
  {
    const unsigned int vec_size = (unsigned int)in_rasters_vec.size();
    
    if( vec_size ) 
    {
      /* Locating the small raster index */
      
      unsigned int smallRasterSize = UINT_MAX;
      unsigned int currRasterSize = 0;
      unsigned int smallRasterIdx = 0;
        
      for( unsigned int vec_index = 0 ; vec_index < vec_size ; ++vec_index ) 
      {
        currRasterSize = (unsigned int)( 
          in_rasters_vec[ vec_index ]->params().nlines_ *
          in_rasters_vec[ vec_index ]->params().ncols_ );
          
        if( currRasterSize < smallRasterSize )
        {
          smallRasterSize = currRasterSize;
          smallRasterIdx = vec_index;
        }
      }
      
      // Resampling rasters
      
      return resampleRasters( in_rasters_vec[ smallRasterIdx ]->params().nlines_,
        in_rasters_vec[ smallRasterIdx ]->params().ncols_, in_rasters_vec,
        out_rasters_vec );
    }
    
    return true;
  }  

  bool decision_rule(
    const std::string& infile,
    const std::string& outfile,
    const std::string& cmd,
    const std::string& fuzzysets )
  {
    #if TePLATFORM == TePLATFORMCODE_MSWINDOWS
      std::string commandLine = "ta_td_generic";
    #elif TePLATFORM == TePLATFORMCODE_LINUX
      std::string commandLine = "./ta_td_generic";
    #else
      #error "ERROR: Unknown platform"
    #endif
      
    commandLine += " " + infile + " " + outfile + " \"" + cmd + 
      "\" " + fuzzysets;
      
    
    if( system( commandLine.c_str() ) > -1 )
    { 
      return true;
    }
    else
    {
      return false;
    }
  }
  
  std::string getMultiSegVersion()
  {
    return std::string( MSEGVERSION );
  }
  
  bool exportPolygons( const TePolygonSet& ps, const std::string& shpFileName )
  {
    std::string base_file_name = TeGetName( shpFileName.c_str() );
    
    // creating files names
    std::string dbfFilename = base_file_name + ".dbf";
    std::string shpFilename = base_file_name + ".shp";

    // creating polygons attribute list ( max attribute size == 12 )
    TeAttributeList attList;
    
    TeAttribute at;
    at.rep_.type_ = TeSTRING;               //the id of the cell
    at.rep_.numChar_ = 10;
    at.rep_.name_ = "object_id_";
    at.rep_.isPrimaryKey_ = true;
    
    attList.push_back(at);
    
    /* DBF output file handle creation */

    DBFHandle hDBF = DBFCreate( dbfFilename.c_str() );
    TEAGN_TRUE_OR_RETURN( ( hDBF != 0 ), "DBF file creation error" );
    
    /* Writing attributes */

    TeAttributeList::iterator it=attList.begin();
    while ( it != attList.end() )
    {
      TeAttribute at = (*it);
      string atName = at.rep_.name_;

      // *OBS****atributos podem ter no maximo 12 caracteres
      // max attribute size == 12
      if (at.rep_.type_ == TeSTRING )
      {
        TEAGN_TRUE_OR_THROW( 
          ( DBFAddField( hDBF, atName.c_str(), FTString, at.rep_.numChar_, 0 ) 
           != -1 ), "Error writing TeSTRING attribute" );
      }
      else if (at.rep_.type_ == TeINT)
      {
        TEAGN_TRUE_OR_THROW( 
          ( DBFAddField( hDBF, atName.c_str(), FTInteger, 10, 0 ) != -1 ), 
          "Error writing TeINT attribute" );
      }
      else if (at.rep_.type_ == TeREAL)
      {
        TEAGN_TRUE_OR_THROW( 
          ( DBFAddField( hDBF, atName.c_str(), FTDouble, 10, 5 ) != -1 ), 
          "Error writing TeREAL attribute" );
          
      }
      else if (at.rep_.type_ == TeDATETIME)
      {
        TEAGN_TRUE_OR_THROW( 
          ( DBFAddField( hDBF, atName.c_str(), FTDate, 8, 0 ) != -1 ), 
          "Error writing TeDATETIME attribute" );
      }
                
      ++it;
    }
    
    /* SHP output file handle creation */

    SHPHandle hSHP = SHPCreate( shpFilename.c_str(), SHPT_POLYGON );
    if( hSHP == 0 ) {
      TEAGN_LOGERR( "DBF file creation error" );
      DBFClose( hDBF );
      return false;
    }
    
    /* Writing polygons */

    int iRecord = 0;
    int totpoints = 0;
    double  *padfX, *padfY;
    SHPObject       *psObject;
    int posXY, npoints, nelem;
    int nVertices;
    int* panParts;

    TePolygonSet::iterator itps;
    TePolygon poly;

    for (itps = ps.begin() ; itps != ps.end() ; itps++ ) {
      poly=(*itps);
      totpoints = 0;
      nVertices = poly.size();
      for (unsigned int n=0; n<poly.size();n++) {
        totpoints += poly[n].size();
      }

      panParts = (int *) malloc(sizeof(int) * nVertices);
      padfX = (double *) malloc(sizeof(double) * totpoints);
      padfY = (double *) malloc(sizeof(double) * totpoints);
      posXY = 0;
      nelem = 0;
      
      for (unsigned int l=0; l<poly.size(); ++l) {
        if (l==0) {
          if (TeOrientation(poly[l]) == TeCOUNTERCLOCKWISE) {
            TeReverseLine(poly[l]);
          }
        } else {
          if (TeOrientation(poly[l]) == TeCLOCKWISE) {
            TeReverseLine(poly[l]);
          }
        }
        
        npoints = poly[l].size();
        panParts[nelem]=posXY;
        
        for (int m=0; m<npoints; m++ ) {
          padfX[posXY] = poly[l][m].x_;
          padfY[posXY] = poly[l][m].y_;
          posXY++;
        }
        
        nelem++;
      }
                
      psObject = SHPCreateObject( SHPT_POLYGON, -1, nelem, panParts, NULL,
        posXY, padfX, padfY, NULL, NULL );
        
      int shpRes = SHPWriteObject( hSHP, -1, psObject );
      TEAGN_TRUE_OR_THROW( ( shpRes != -1 ), 
        "Unable to create a shape write object" )
        
      SHPDestroyObject( psObject );
      free( panParts );
      free( padfX );
      free( padfY );

      // writing attributes - same creation order
      for (unsigned int j=0; j<attList.size();j++) {
        if ( attList[j].rep_.type_ == TeSTRING ) {
          DBFWriteStringAttribute(hDBF, iRecord, j, poly.objectId().c_str() );
        } /*else if ( attList[j].rep_.type_ == TeINT) {
          DBFWriteIntegerAttribute(hDBF, iRecord, j,  VALOR INT );        
        } else if ( attList[j].rep_.type_ == TeREAL) {
          DBFWriteDoubleAttribute(hDBF, iRecord, j,  VALOR DOUBLE);
        } else if ( attList[j].rep_.type_ == TeDATETIME) {
          TeTime time =  VALOR DATA;
          char dd[8];
          sprintf(dd,"%04d%02d%02d",time.year(),time.month(),time.day());
          DBFWriteDateAttribute(hDBF, iRecord, j, dd );
        }*/
      }
                
      iRecord++;
    }
        
    DBFClose( hDBF );
    SHPClose( hSHP );

    return true;  
  }

  bool exportPolygons( const OpSupportFunctions::ClassesDataVectorT&
    classes_data_vector, const std::string& shpFileName )
  {
    TePolygonSet internalPs;
    
    ClassesDataVectorT::const_iterator cdvIt = classes_data_vector.begin();
    ClassesDataVectorT::const_iterator cdvItEnd = classes_data_vector.end();
    TePolygonSet::iterator psIt;
    TePolygonSet::iterator psItEnd;
    
    while( cdvIt != cdvItEnd )
    {
      psIt = cdvIt->pols_.begin();
      psItEnd = cdvIt->pols_.end();
      
      while( psIt != psItEnd )
      {
        internalPs.add( *psIt );
        ++psIt;
      }
      
      ++cdvIt;
    }

    return exportPolygons( internalPs, shpFileName );
  }
   
  bool createClassesDataVector( 
    TePDITypes::TePDIRasterPtrType& label_image_ptr,
    OpSupportFunctions::ClassesDataVectorT& classes_data_vector )
  {
    TEAGN_DEBUG_CONDITION( label_image_ptr.isActive(), "invalid pointer" )
    
    classes_data_vector.clear();
    
    // Vectorizing
    
    TePDIParameters algo_params;
  
    algo_params.SetParameter( "rotulated_image", 
      label_image_ptr );
    
    TePDITypes::TePDIPolSetMapPtrType output_polsets( 
      new TePDITypes::TePDIPolSetMapType );      
    algo_params.SetParameter( "output_polsets", output_polsets );
    algo_params.SetParameter( "channel", (unsigned int)0 );

    TePDIRaster2Vector vectorizer_instance;
    
    TEAGN_TRUE_OR_THROW( vectorizer_instance.Reset( algo_params ), 
      "Algorithm Reset error" ); 
          
    TEAGN_TRUE_OR_RETURN( vectorizer_instance.Apply(), 
      "Algorithm Apply error" );
          
    /* building vector */
    
    TePDITypes::TePDIPolSetMapType::iterator mapIt =
      output_polsets->begin();
    const TePDITypes::TePDIPolSetMapType::iterator mapItEnd =
      output_polsets->end();
    TePolygonSet::iterator psIt;
    TePolygonSet::iterator psItEnd;

    while( mapIt != mapItEnd )
    {
      if( mapIt->first != 0.0 )
      {
        psIt = mapIt->second.begin();
        psItEnd = mapIt->second.end();
        
        if( psIt != psItEnd )
        {
          classes_data_vector.push_back( ClassesDataNode() );
          ClassesDataNode& newNode = classes_data_vector[ 
            classes_data_vector.size() - 1 ];
            
          newNode.class_value_ = (unsigned int)mapIt->first;
          
          while( psIt != psItEnd )
          {
            newNode.pols_.add( *psIt );
            
            ++psIt;
          }
        }
      }
      
      ++mapIt;
    }
          
    return updatePolsIndexedBoxes( label_image_ptr, classes_data_vector );
  }
  
  bool updatePolsIndexedBoxes( 
    TePDITypes::TePDIRasterPtrType& label_image_ptr,
    OpSupportFunctions::ClassesDataVectorT& classes_data_vector )
  {
    const unsigned int cdvSize = (unsigned int)classes_data_vector.size();
    TeCoord2D polllIndexed; 
    TeCoord2D polurIndexed;
    TeBox indexedBox;
    unsigned int psSize = 0;
    unsigned int psIdx = 0;
    
    for( unsigned int cdvIdx = 0; cdvIdx < cdvSize ; ++cdvIdx )
    {
      ClassesDataNode& node = classes_data_vector[ cdvIdx ];
      node.polsIndexedBoxes_.clear();
      
      psSize = (unsigned int)node.pols_.size();
      
      for( psIdx = 0 ; psIdx < psSize ; ++psIdx )
      {
        const TeBox& polBox = node.pols_[ psIdx ].box();
        
        polllIndexed = label_image_ptr->coord2Index( polBox.lowerLeft() );
        polurIndexed = label_image_ptr->coord2Index( polBox.upperRight() );
        
        indexedBox.x1_ = TeRound( polllIndexed.x() + 0.5 );
        indexedBox.y1_ = TeRound( polllIndexed.y() - 0.5 );
        
        indexedBox.x2_ = TeRound( polurIndexed.x() - 0.5 );
        indexedBox.y2_ = TeRound( polurIndexed.y() + 0.5 );   
        
        TEAGN_DEBUG_CONDITION( ( indexedBox.x1_ >= 0 ), "Invalid value" )
        TEAGN_DEBUG_CONDITION( ( indexedBox.y1_ >= 0 ), "Invalid value" )
        TEAGN_DEBUG_CONDITION( ( indexedBox.x2_ >= 0 ), "Invalid value" )
        TEAGN_DEBUG_CONDITION( ( indexedBox.y2_ >= 0 ), "Invalid value" )
        
        TEAGN_DEBUG_CONDITION( 
          ( indexedBox.x1_ < label_image_ptr->params().ncols_ ), 
          "Invalid value" )
        TEAGN_DEBUG_CONDITION( 
          ( indexedBox.y1_ < label_image_ptr->params().nlines_ ), 
          "Invalid value" )
        TEAGN_DEBUG_CONDITION( 
          ( indexedBox.x2_ < label_image_ptr->params().ncols_ ), 
          "Invalid value" )
        TEAGN_DEBUG_CONDITION( 
          ( indexedBox.y2_ < label_image_ptr->params().nlines_ ), 
          "Invalid value" )          
                  
        node.polsIndexedBoxes_.push_back( indexedBox );
      }
    }
    
    return true;
  }
  
}; // namespace OpSupportFunctions

