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
  \file Pyramid.h

  \brief This class represents an image hierarchical pyramid.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_PYRAMID_H
#define __MULTISEG_INTERNAL_PYRAMID_H

// MultiSeg
#include "Config.h"

// TerraLib PDI
#include <terralib/image_processing/TePDITypes.hpp>
#include <terralib/image_processing/TePDIStatistic.hpp>

// STL
#include <vector>

/*!
  \class Pyramid

  \brief This class represents an image hierarchical pyramid.
*/
class MSEGEXPORT Pyramid
{
  public:

    /*!
      \brief Constructor.

      \param image            The input image.
      \param nLevels          The pyramid number of levels.
      \param progressEnabled  A flag that indicates if the progress must be enabled.
    */
    Pyramid(const TePDITypes::TePDIRasterPtrType& image, const std::size_t& nLevels, const bool& progressEnabled = false);

   /*!
      \brief Constructor.

      \param image            The input image.
      \param nLevels          The pyramid number of levels.
      \param bands            The input image bands that will be considered.
      \param progressEnabled  A flag that indicates if the progress must be enabled.
    */
    Pyramid(const TePDITypes::TePDIRasterPtrType& image, const std::size_t& nLevels, const std::vector<std::size_t>& bands, const bool& progressEnabled = false);

    /*! \brief Destructor. */
    ~Pyramid();

    /*!
      \brief This method returns the pyramid number of levels.

      \return The pyramid number of levels.
    */
    std::size_t getNLevels() const;

    /*!
      \brief This method returns the i-th level of the hierarchical pyramid.
      
      \param i The requested level.

      \return The i-th level of the hierarchical pyramid.
    */
    TePDITypes::TePDIRasterPtrType getLevel(const std::size_t& i) const;

    /*!
      \brief This method releases the i-th level of the hierarchical pyramid.
      
      \param i The level that will be released.
    */
    void releaseLevel(const std::size_t& i);

    /*!
      \brief Static method that resizes the given image based on the number of lines and columns.
      
      \param image The image that will be resized.
      \param nlin  The new number of lines.
      \param ncol  The new number of columns.

      \return The resized image.
    */
    static TePDITypes::TePDIRasterPtrType resize(TePDITypes::TePDIRasterPtrType& image,
                                                 const std::size_t& nlin, const std::size_t& ncol);

     /*!
      \brief Static method that resizes the given image based on the new parameters.
      
      \param image  The image that will be resized.
      \param params The new image parameters.

      \return The resized image.
    */
    static TePDITypes::TePDIRasterPtrType resize(TePDITypes::TePDIRasterPtrType& labelledImage, TeRasterParams params);

    /*!
      \brief This method computes the statistical values of the i-th level of the hierarchical pyramid.
      
      \param i The level that will be considered.

      \return The statistical values of the i-th level of the hierarchical pyramid.
    */
    TePDIStatistic* buildStats(const std::size_t& i);

  private:

    /*! \brief Internal method that builds the hierarchical pyramid. */
    void build();

    /*! \brief Internal method that builds the hierarchical pyramid. */
    void build(TePDITypes::TePDIRasterPtrType& previousLevel, TePDITypes::TePDIRasterPtrType& newLevel);

  private:

    std::vector<TePDITypes::TePDIRasterPtrType> m_levels; //!< The pyramid number of levels.
    std::vector<std::size_t> m_bands;                     //!< The input image bands used to build the pyramid.
    bool m_progressEnabled;                               //!< A flag that indicates if the progress must be enabled.
};

#endif // __MULTISEG_INTERNAL_PYRAMID_H
