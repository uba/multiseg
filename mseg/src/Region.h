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
  \file Region.h

  \brief This class represents an area in the image being segmented. i.e. a region.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_REGION_H
#define __MULTISEG_INTERNAL_REGION_H

// MultiSeg
#include "Config.h"

// STL
#include <list>
#include <vector>

/*!
  \class Region

  \brief This class represents an area in the image being segmented. i.e. a region.
*/
class MSEGEXPORT Region
{
  public:

    /*!
      \brief It initializes a region. Here, a region is a pixel.

      \param id    Identifier that will be associated with this region.
      \param pixel The pixel values.
      \param lin   The pixel line.
      \param col   The pixel column.
    */
    Region(const std::size_t& id, const std::vector<double>& pixel,
           const std::size_t& lin, const std::size_t& col);

    /*! \brief Virtual destructor. */
    virtual ~Region();

    /*!
      \brief This method returns the region identifier.

      \return The region identifier.
    */
    const std::size_t& getId() const;

    /*!
      \brief This method returns the region size. i.e. the number of pixels of the region.

      \return The region size.
    */
    const std::size_t& getSize() const;

    /*!
      \brief This method sets the region size. i.e. the number of pixels of the region.

      \param size The region size.
    */
    void setSize(const std::size_t& size);

    void updateBounds(const std::size_t& scale, const std::size_t& maxLin, const std::size_t& maxCol);

    /*!
      \brief This method updates the region bounds based on the given region. i.e. performs an union of the bounds.

      \param region The region that will be considered.
    */
    void updateBounds(Region* region);

    /*!
      \brief This method updates the region left X coordinate box.

      \param x The left X coordinate box.
    */
    void updateXStart(const std::size_t& x);

    /*!
      \brief This method updates the region upper Y coordinate box.

      \param y The upper Y coordinate box.
    */
    void updateYStart(const std::size_t& y);

    /*!
      \brief This method updates the region rigth bound X coordinate box.

      \param x The rigth bound X coordinate box.
    */
    void updateXBound(const std::size_t& x);

    /*!
      \brief This method updates the region lower Y coordinate box.

      \param y The lower Y coordinate box.
    */
    void updateYBound(const std::size_t& y);

    /*!
      \brief This method returns the region left X coordinate box.

      \return The region left X coordinate box.
    */
    const std::size_t& getXStart() const;

    /*!
      \brief This method returns the region upper Y coordinate box.

      \return The region upper Y coordinate box over the label image.
    */
    const std::size_t& getYStart() const;

    /*!
      \brief This method returns the region rigth bound X coordinate box.

      \return The region rigth bound X coordinate box.
    */
    const std::size_t& getXBound() const;
    
    /*!
      \brief This method returns the region lower Y coordinate box.

      \return The region lower Y coordinate box.
    */
    const std::size_t& getYBound() const;

    /*!
      \brief This method returns the region mean.

      \return The region mean.
    */
    std::vector<double>& getMean();

    /*!
      \brief This method returns the region variance.

      \return The region variance.
    */
    std::vector<double>& getVariance();

    /*!
      \brief This method returns the region coefficient of variation.

      \return The region coefficient of variation.
    */
    std::vector<double>& getCV();

    /*!
      \brief This method sets the region mean.

      \param mean The region mean.
    */
    void setMean(const std::vector<double>& mean);

    /*!
      \brief This method sets the region variance.

      \param variance The region variance.
    */
    void setVariance(const std::vector<double>& variance);

    /*!
      \brief This method sets the region coefficient of variation.

      \param cv The region coefficient of variation.
    */
    void setCV(const std::vector<double>& cv);

    /*!
      \brief This method adds a neighbour region to this region.

      \param region The region that will be added as neighbour to this region.
    */
    void addNeighbour(Region* region);

    /*!
      \brief This method removes the neighbour region from this region.

      \param region The region that will be removed as neighbour to this region.
    */
    void removeNeighbour(Region* region);

    /*!
      \brief This method returns the list of neighbours from this region.

      \return The list of neighbours from this region.
    */
    std::list<Region*>& getNeighbours();

    /*!
      \brief This method returns if the given region is neighbour of this region.

      \return It returns true if the given region is neighbour of this region anda false otherwise.
    */
    bool isNeighbour(Region* region);

    /*!
      \brief This method returns the region number of bands.

      \return The region number of bands.
    */
    std::size_t getNBands() const;

  protected:

    std::size_t m_id;                         //!< Region id.
    std::size_t m_size;                       //!< Region area (number of pixels).
    std::size_t m_xStart;                     //!< Region left X coordinate box over the label image.
    std::size_t m_yStart;                     //!< Region upper Y coordinate box over the label image.
    std::size_t m_xBound;                     //!< Region right bound X coordinate box over the label image.
    std::size_t m_yBound;                     //!< Region lower bound Y coordinate box over the label image.
    std::vector<double> m_mean;               //!< Region mean values (for each band).
    std::vector<double> m_variance;           //!< Region variance values (for each band).
    std::vector<double> m_cv;                 //!< Region coefficient of variation values (for each band).
    std::list<Region*> m_neighbours;          //!< Neighbours regions.
};

#endif  // __MULTISEG_INTERNAL_REGION_H
