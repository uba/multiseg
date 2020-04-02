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
  \file MultiSeg.h

  \brief This class implements the MultiSeg algorithm.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_MULTISEG_H
#define __MULTISEG_INTERNAL_MULTISEG_H

// MultiSeg
#include "Config.h"
#include "CVTable.h"
#include "Enums.h"
#include "Pyramid.h"

// TerraLib PDI
#include <terralib/image_processing/TePDIAlgorithm.hpp>

// STL
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>

// Forward declaration
class AbstractMerger;
class AbstractOutputter;
class Region;

/*! \brief Set of pixel indexes. */
typedef std::set<std::pair<std::size_t, std::size_t> > Pixels;

/*!
  \class MultiSeg

  \brief This class implements the MultiSeg algorithm.

  \author Douglas Uba <douglas@dpi.inpe.br>

  \note The segmentation is performed through an approach described in:
  "Segmentação Multiníveis e Multimodelos para imagens de radar e óptica"- MultiSeg - http://mtc-m17.sid.inpe.br/col/sid.inpe.br/jeferson/2005/06.01.18.47/doc/publicacao.pdf

  \note The general required parameters:

  \param input_image (TePDITypes::TePDIRasterPtrType)

  \param image_model (ImageModelRepresentation)
  Defines the image model representation that will be used on segmentation process (Cartoon or Texture).

  \param image_type (ImageType)
  Defines the type of the image that will be segmented.

  \param image_radar_format (ImageFormat)
  Defines the type of the radar image that that will be segmented. (Amplitude, Intensity or dB).

  \param levels (std::size) - The maximum number of levels.
  \param similarity (double) - Expressed in dB case ImageType == Radar or gray scale case ImageType == Optical.
  \param min_area (std::size_t) - Region pixel size minimum value.
  \param ENL (double) - Number of looks. Required when ImageType == Radar and ImageModelRepresentation == Cartoon.
  \param confidence_level (double) - Required when ImageType == Radar and ImageModelRepresentation == Texture. Or ImageType == Optical.
  \param cv (double) - Coefficient of variation. Required when ImageType == Radar and ImageModelRepresentation == Texture. Or ImageType == Optical.
*/
class MSEGEXPORT MultiSeg : public TePDIAlgorithm
{
  public:

    /*! \brief Default constructor. */
    MultiSeg();

    /*! \brief Destructor. */
    ~MultiSeg();

    /*!
      \brief This method checks if the supplied parameters fits the requirements of MultiSeg algorithm.

      \param params The parameters to be checked.

      \return It returns true if the parameters are OK and false otherwise.

      \note Error log messages must be generated. No exceptions generated.
    */
    bool CheckParameters(const TePDIParameters& params) const;

    /*!
      \brief This method returns the input image. i.e. the image being segmented.

      \return The input image.
    */
    const TePDITypes::TePDIRasterPtrType& getInputImage() const;

    /*!
      \brief This method returns the set of found regions.

      \return The set of found regions.
    */
    const std::map<std::size_t, Region*>& getRegions() const;

    /*!
      \brief This method returns the labelled image. i.e. the image that contains the regions identifiers.

      \return The labelled image.
    */
    const TePDITypes::TePDIRasterPtrType& getLabelledImage() const;

    /*!
      \brief This method returns the input bands.

      \return The input bands.
    */
    const std::vector<std::size_t>& getUsedBands() const;

    /*!
      \brief This method adds the given outputter to the current algorithm implementation.

      \param outputter The outputter that will be added.
    */
    void addOutputter(AbstractOutputter* outputter);

    /*! \brief This method sets if the image hierarchical pyramid must be notified. */
    void outputPyramid(bool on);

    /*! \brief This method sets if the intermediate results must be notified. */
    void notifyIntermediateResults(bool on);

  protected:

    /*!
      \brief This method reset the internal state to the initial state.

      \param params The new parameters referente at initial state.
    */
    void ResetState(const TePDIParameters& params);

    /*!
      \brief This method runs the current algorithm implementation.

      \return It returns true if ok and false otherwise.
    */
    bool RunImplementation();

  private:

    /*! \brief This method initializes the internal MultiSeg parameters. */
    void initializeParameters();

    /*! \brief This method initializes the merger that will be used based on input MultiSeg parameters. */
    void initializeMerger();

    /** @name Region Growing */
    //@{

    void initializeRegions(const TePDITypes::TePDIRasterPtrType& image);

    void executeRegionGrowing(std::map<std::size_t, Region*>& regions, bool usingRandomSeeds = false, std::size_t maxIterations = 100);

    std::size_t mergeRegions(std::map<std::size_t, Region*>& regions);

    std::size_t mergeRegionsRandomly(std::map<std::size_t, Region*>& regions);

    std::size_t mergeSmallRegions();

    Region* getRegion(const std::size_t& id);

    Region* getClosestRegion(Region* region, bool useAllNeighbours = false);

    std::list<Region*> getClosestRegions(Region* region);

    void updateNeighborhoodAfterMerge(Region* region, Region* merged);

    //@}

    /** @name Border Adjustments  */
    //@{

    void updateRegionStatistics(const TePDITypes::TePDIRasterPtrType& image);

    void adjustRegionBorders(const TePDITypes::TePDIRasterPtrType& image);

    void adjustRegionBorders(Region* region, const TePDITypes::TePDIRasterPtrType& image, Pixels& alreadyAdjustedPixels);

    bool isBorderPixel(const std::size_t& lin, const std::size_t& col, const std::size_t& regionId,
                       std::size_t& neighbourLin, std::size_t& neighbourCol, std::size_t& neighbourRegionId,
                       BorderPixelType& neighbourPixelType,
                       const std::size_t& lastLin, const std::size_t& lastCol);

    std::size_t computeBorderDestiny(const std::size_t& linA, const std::size_t& colA, Region* rA,
                                     const std::size_t& linB, const std::size_t& colB, Region* rB,
                                     const TePDITypes::TePDIRasterPtrType& image);
    //@}

    /** @name Resegmentation */
    //@{

    void splitRegions(const TePDITypes::TePDIRasterPtrType& image, std::map<std::size_t, Region*>& newRegions);

    void invalidateRegionPixels(Region* region);

    void splitRegion(Region* region, const TePDITypes::TePDIRasterPtrType& image, std::map<std::size_t, Region*>& newRegions);

    //@}

    /** @name Minimum Area  */
    //@{

    void processSmallRegions();

    //@}

    /** @name Auxiliary Methods */
    //@{

    void updateLabelledImage(Region* region, Region* merged);

    void removeRegion(Region* region, bool linkNeighbourhood = false);

    void getPixelValues(const std::size_t& lin, const std::size_t& col,
                        std::vector<double>& pixel,
                        const TePDITypes::TePDIRasterPtrType& image);

    void resizeRegions();

    //@}

    /*! \brief This method updates the thresholds values based on the current level. */
    void updateThresholds(const std::size_t& currentLevel);

    /*! \brief This method notifies for each registered outputter the MultiSeg results. */
    void notifyResult();

  private:

    /** @name Parameters
     *  MultiSeg Algorithm Parameters.
    */
    //@{

    TePDITypes::TePDIRasterPtrType m_inputImage;        //!< Input image.

    std::vector<std::size_t> m_bands;                   //!< The input bands.

    ImageModelRepresentation m_imageModel;              //!< Defines the input image model representation - (Cartoon or Texture).
    ImageType m_imageType;                              //!< Defines the type of the input image - (Radar or Optical).
    ImageFormat m_imageRadarFormat;                     //!< Defines the type of the radar input image - (Amplitude, Intensity or dB).

    std::size_t m_levels;                               //!< The maximum number of levels.
    double m_similarity;                                //!< Expressed in dB case ImageType == Radar or gray scale case ImageType == Optical.
    std::size_t m_minArea;                              //!< Region pixel size minimum value.
    double m_ENL;                                       //!< Number of looks. Required when ImageType == Radar and ImageModelRepresentation == Cartoon.
    double m_confidenceLevel;                           //!< Required when ImageType == Radar and ImageModelRepresentation == Texture. Or ImageType == Optical.
    double m_cv;                                        //!< Coefficient of variation. Required when ImageType == Radar and ImageModelRepresentation == Texture. Or ImageType == Optical.
    
    //@}

    TePDITypes::TePDIRasterPtrType  m_labelledImage;    //!< The labelled image that will be generated.
    std::map<std::size_t, Region*> m_regions;           //!< The set of regions.

    AbstractMerger* m_merger;                           //!< The merge that will be used.

    CVTable m_cvTable;                                  //!< The table of Coefficient of Variation.

    std::size_t m_similarityIncreaseStep;
    bool m_enableMutualBestFitting;                     //!< A flag that indicates if the mutual best fitting is necessary to merging two regions.
    bool m_growUntilStop;                               //!< A flag that indicates if the region grows until stop during the region growing process.
    std::size_t m_nSamples;                             //!< The number of samples used to compute region statistics.
    bool m_considerRegionVsRegion;                      //!< A flag that indicates if the region vs. region tests is considered or not.

    std::size_t m_currentLevel;                         //!< The current level being segmented.
    double m_currentSimilarity;                         //!< The current similarity threshold.
    double m_currentCV;                                 //!< The current coefficient of variation.
    double m_currentENL;                                //!< The current number of looks.

    Pyramid* m_pyramid;                                 //!< The image hierarchical pyramid.
    std::vector<AbstractOutputter*> m_outputters;       //!< The set of outputters.

    bool m_outputPyramid;                               //!< A flag that indicates if the image hierarchical pyramid must be outputted.
    bool m_notifyIntermediateResults;                   //!< A flag that indicates if the intermediate results must be outputted.
};

#endif // __MULTISEG_INTERNAL_MULTISEG_H
