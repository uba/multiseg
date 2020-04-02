#ifndef OPSUPPORTFUNCTIONS_HPP
  #define OPSUPPORTFUNCTIONS_HPP
  
  // MultiSeg Operator
  #include "OperatorsBaseDefines.h"

  // MultiSeg
  #include <mseg/Region.h>
 
  // TerraLib 
  #include <terralib/image_processing/TePDIHaralick.hpp>
  #include <terralib/image_processing/TePDIInterpolator.hpp>
  #include <terralib/image_processing/TePDITypes.hpp>
  #include <terralib/kernel/TeRaster.h>

  #include <map>
  #include <string>
  #include <vector>

  /**
   * @brief A namespace for operators support functions.
   * @author Emiliano F. Castejon <castejon@dpi.inpe.br>
   */  
  namespace OpSupportFunctions
  {
    /**
     * @brief Region attributes type (name, value).
     */   
    typedef std::pair< std::string, std::string > RegionAttT;
        
    /**
     * @brief Region attributes vector type.
     */   
    typedef std::vector< RegionAttT > RegionAttsVecT;

    /**
     * @brief Class polygons map type (pixel value to polygon).
     * @note zero as pixel value not allowed.
     * @param unsigned int The polygon ID over the output labeled image.
     * @param TePolygon The region polygon.
     */   
    typedef std::map< unsigned int, TePolygon > ClassPolsMapT;     
    
    /**
     * @brief All class polygons attributes vector type.
     */   
    typedef std::vector< RegionAttsVecT > AllRegionsAttsVecT;
      
    /**
     * @brief Contains classes data used when generating the regions 
     * descriptions file.
     */   
    class ClassesDataNode {
      public :
        /** @brief The class ID. */
        std::string class_id_;
        
        /** @brief The class Pixel Value over the labeled image. 
          Default=0 (dummy)*/
        unsigned int class_value_;
        
        /** @brief The class polgons (projected coords).
          */
        TePolygonSet pols_;
        
        /** @brief The polygons attributes. */
        AllRegionsAttsVecT attributes_;
        
        /** Region A vector of indexed boxes (line,column) of
          each polygon over the label image. */
        std::vector< TeBox > polsIndexedBoxes_;
        
        ClassesDataNode();
        ~ClassesDataNode();
    };
    
    /**
     * @typedef Classes data vector type definition.
     */      
    typedef std::vector< ClassesDataNode > ClassesDataVectorT;
    
    
    /**
     * @brief Load the mask image into memory raster.
     * @param mask_file_name The mask file name.
     * @param geoWest West bounding box coord.
     * @param geoNorth North bounding box coord.
     * @param geoEast East box coord.
     * @param geoSouth South bounding box coord.     
     * @param mask_raster_ptr The raster instance pointer.
     * @return true if OK, false on error.
     */  
    bool getMaskRaster( 
      const std::string& mask_file_name,
      double geoWest, double geoNorth,
      double geoEast, double geoSouth,
      TePDITypes::TePDIRasterPtrType& mask_raster_ptr );      
      
    /**
     * @brief Get the mask polygons from the mask raster.
     * @param mask_raster_ptr The raster instance pointer.
     * @param mask_pols The generated mask polygons.
     * @return true if OK, false on error.
     */  
    bool getMaskPolygons( 
      const TePDITypes::TePDIRasterPtrType& mask_raster_ptr,
      TePolygonSet& mask_pols );
    
    /**
     * @brief Generate a raster geometry clipping an input raster.
     * @param input_raster_ptr Input raster pointer.
     * @param geoWest Clip West box coord.
     * @param geoNorth Clip North box coord.
     * @param geoEast Clip East box coord.
     * @param geoSouth Clip South box coord.
     * @param clip_raster_ptr A pointer to the generated clip.
     * @return true if OK, false on error.
     * @note All bands/channels will be used.
     * @deprecated Will be removed in the future.
     */  
    bool createRasterClip( 
      const TePDITypes::TePDIRasterPtrType& input_raster_ptr,
      double geoWest, double geoNorth,
      double geoEast, double geoSouth,
      TePDITypes::TePDIRasterPtrType& clip_raster_ptr );      
      
    /**
     * @brief Generate a raster geometry clipping an input raster.
     * @param input_raster_ptr Input raster pointer.
     * @param channels_vec The used channels/bands.
     * @param geoWest Clip West box coord.
     * @param geoNorth Clip North box coord.
     * @param geoEast Clip East box coord.
     * @param geoSouth Clip South box coord.
     * @param clip_raster_ptr A pointer to the generated clip.
     * @return true if OK, false on error.
     */  
    bool createRasterClip( 
      const TePDITypes::TePDIRasterPtrType& input_raster_ptr,
      const std::vector< unsigned int >& channels_vec,
      double geoWest, double geoNorth,
      double geoEast, double geoSouth,
      TePDITypes::TePDIRasterPtrType& clip_raster_ptr );       
      
    /**
     * @brief Create a labeled RAM image using the regions data vector.
     * @param regions_data_vector The regions data vector.
     * @param nlines The number of lines for the generated 
     * raster.
     * @param ncols The number of columns for the generated 
     * raster.
     * @param geoWest Raster West bounding box coord.
     * @param geoNorth Raster North bounding box coord.
     * @param geoEast Raster East bounding box coord.
     * @param geoSouth Raster South bounding box coord.   
     * @param raster_ptr The generated raster.
     * @return true if OK, false on errors.
     * @note The ClassesDataNode::polsIndexedBoxes_ of each vector
     * element will be updated.
     */  
    bool createRAMLabeledImage( 
      std::vector< ClassesDataNode >& regions_data_vector,
      unsigned int nlines, unsigned int ncols,
      double geoWest, double geoNorth,
      double geoEast, double geoSouth,      
      TePDITypes::TePDIRasterPtrType& raster_ptr );   
      
    /**
     * @brief Save a RAM labeled image to a disk file.
     * @param output_image_file_name_str Image disk file name.
     * @param raster_ptr The RAM raster pointer.
     * @param raster_channel Raster channel to use when generating the
     * labeled image.
     * @return true if OK, false on errors.
     * indicate dummy raster data.
     */  
    bool saveLabeledImageFile( 
      const std::string& output_image_file_name_str,
      TePDITypes::TePDIRasterPtrType& raster_ptr,
      unsigned int raster_channel );      
      
    /**
     * @brief Create a memory TeRaster for reading and writing.
     * @param raster_params The params used to create the new raster.
     * @param raster_ptr The output raster instance pointer.
     * @return true if OK, false on error.
     */  
    bool createMemRaster( 
      const TeRasterParams& raster_params,
      TePDITypes::TePDIRasterPtrType& raster_ptr );      
      
    /**
     * @brief Create a TIFF file from a raster instance.
     * @param file_name The output TIFF file name.
     * @param raster_ptr The raster instance pointer.
     * @param out_data_type Output data type.
     * @return true if OK, false on error.
     */  
    bool createTIFFFile( 
      const std::string& file_name,
      const TePDITypes::TePDIRasterPtrType& raster_ptr,
      TeDataType out_data_type );       
 
    /**
     * @brief Export polygon objects to the Interimage regions description
     * XML file.
     * @param reg_desc_file_name Regions description file name.
     * @param regions_data_vector The regions data vector.
     * @param output_raster_params The output labeled image parameters.
     * @param node_weight The node weight ( "p" attribute value )
     * @param geoWest West bounding box coord.
     * @param geoNorth North bounding box coord.
     * @param geoEast East box coord.
     * @param geoSouth South bounding box coord.      
     * @return true if OK, false on error.
     * 
     */   
    bool createRegionsDescFile( 
      const std::string& reg_desc_file_name,
      const std::vector< ClassesDataNode >& regions_data_vector,
      const TeRasterParams& output_raster_params, 
      double node_weight, 
      double geoWest, double geoNorth,
      double geoEast, double geoSouth );

    /**
     * @brief Export polygon objects to the Interimage regions description
     * XML file.
     * @param reg_desc_file_name Regions description file name.
     * @param regions_data_vector The regions data vector.
     * @param output_raster_params The output labeled image parameters.
     * @param node_weight The node weight ( "p" attribute value )
     * @param geoWest West bounding box coord.
     * @param geoNorth North bounding box coord.
     * @param geoEast East box coord.
     * @param geoSouth South bounding box coord.      
     * @return true if OK, false on error.
     * 
     */   
    bool createRegionsDescFile( 
      const std::string& reg_desc_file_name,
      const std::vector< ClassesDataNode >& regions_data_vector,
      const TeRasterParams& output_raster_params, 
      double node_weight, 
      double geoWest, double geoNorth,
      double geoEast, double geoSouth, const std::map<std::size_t, Region*>& regions );
                  
    /**
     * @brief Resample the rasters to match dimentions (Bicubic).
     * @param targetNLines Target number of lines.
     * @param targetNCols Target number of columns.
     * @param in_rasters_vec The input rasters vector.
     * @param out_rasters_vec The output rasters vector.
     * @return true if OK, false on error.
     * @note Inactive in_rasters_vec pointers will be ignored
     * and passed out to the output vector.
     * @note Rasters matching the target dimensions will not bet
     * altered and will be repassed to the output vector.
     */  
    bool resampleRasters( 
      unsigned int targetNLines, unsigned int targetNCols,
      const TePDITypes::TePDIRasterVectorType& in_rasters_vec,
      TePDITypes::TePDIRasterVectorType& out_rasters_vec  );  
      
    /**
     * @brief Resample the rasters to match the dimention of
     * the small raster (Bicubic).
     * @param in_rasters_vec The input rasters vector.
     * @param out_rasters_vec The output rasters vector.
     * @return true if OK, false on error.
     * @note Inactive in_rasters_vec pointers will be ignored
     * and passed out to the output vector.
     * @note Rasters matching the target dimensions will not bet
     * altered and will be repassed to the output vector.
     */  
    bool resampleRasters( 
      const TePDITypes::TePDIRasterVectorType& in_rasters_vec,
      TePDITypes::TePDIRasterVectorType& out_rasters_vec  );           

    /**
     * @brief Call Decision Rules for internal results.
     * @param input_file Input file name.
     * @param output_file Output file name.
     * @param decision_rule String of decision rule.
     * @param fuzzysets The fuzzy sets passed to the operator.
     * @return true if OK, false on error.
     */  
    bool decision_rule(
      const std::string& infile,
      const std::string& outfile,
      const std::string& cmd,
      const std::string& fuzzysets );

    /**
     * @brief Returns the current TerraAIDA version string.
     * @return Returns the current TerraAIDA version string.
     */  
    std::string getMultiSegVersion();
    
    /**
     * @brief Export polygons to a shape file.
     * @param ps Input polygon set.
     * @return true if Ok, false on errors.
     * @param shpFileName Output shape file name.
     */     
    bool exportPolygons( const TePolygonSet& ps, 
      const std::string& shpFileName );
      
    /**
     * @brief Export polygons to a shape file.
     * @param classes_data_vector Classes data vector.
     * @return true if Ok, false on errors.
     * @param shpFileName Output shape file name.
     */         
    bool exportPolygons( const OpSupportFunctions::ClassesDataVectorT&
      classes_data_vector, const std::string& shpFileName );
      
    /**
     * @brief Create a classes data vector from a lable image. 
     * @param classes_data_vector Classes data vector.
     * @return true if Ok, false on errors.
     * @param label_image_ptr The label image.
     * @note this method does not generate classes from zero value pixels
     * (zero is assumed to have no data - dummy).
     */       
    bool createClassesDataVector( 
      TePDITypes::TePDIRasterPtrType& label_image_ptr,
      OpSupportFunctions::ClassesDataVectorT& classes_data_vector );
      
    /**
     * @brief Update the polsIndexedBoxes_ from each vector element
     * using the label image.
     * @param classes_data_vector Classes data vector.
     * @return true if Ok, false on errors.
     * @param label_image_ptr The label image.
     * @note this method does not generate classes from zero value pixels
     * (zero is assumed to have no data - dummy).
     */        
    bool updatePolsIndexedBoxes( 
      TePDITypes::TePDIRasterPtrType& label_image_ptr,
      OpSupportFunctions::ClassesDataVectorT& classes_data_vector );
  };

#endif
