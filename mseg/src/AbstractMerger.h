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
  \file AbstractMerger.h

  \brief The merger definition.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_ABSTRACTMERGER_H
#define __MULTISEG_INTERNAL_ABSTRACTMERGER_H

// MultiSeg
#include "Config.h"

// STL
#include <map>
#include <string>
#include <vector>

// Forward declaration
class Region;

/*!
  \class AbstractMerger

  \brief The merger definition.

  \sa EuclideanMerger, OpticalCartoonMerger, RadarCartoonMerger, CompositeMerger
*/
class MSEGEXPORT AbstractMerger
{
  public:

    /*! \brief Default constructor. */
    AbstractMerger();

    /*! \brief Virtual destructor. */
    virtual ~AbstractMerger();

    /*!
      \brief This method sets a specific parameter of this merger.

      \param name The parameter name.
      \param value The parameter value.
    */
    virtual void setParam(const std::string& name, const double& value);

    /*!
      \brief This method sets the specific parameter list of this merger.

      \param params The list of parameters.
    */
    virtual void setParams(const std::map<std::string, double>& params);

    /*!
      \brief This method returns the value of parameter identified by the given name.

      \param name The parameter name.

      \retrun The parameter value identified by the given name.
    */
    virtual double getParam(const std::string& name) const;

    /*!
      \brief This method returns the specific parameter list of this merger.

      \retrun The list of parameters.
    */
    virtual const std::map<std::string, double>& getParams() const;

    /** @name Public abstract virtual methods
      * Methods related with the merger metrics. */
    //@{

    /*!
      \brief This method evaluates the predicate of homogeneity between two regions.

      \param r1 The first region.
      \param r2 The second region.

      \retrun It returns true if the two regions are homogeneous and false otherwise.
    */
    virtual bool predicate(Region* r1, Region* r2) const;

    /*!
      \brief This method computes the dissimilarity value between a pixel and a region.

      \param p The pixel values.
      \param r The region.

      \retrun It returns the dissimilarity value between the given pixel and the given region.
    */
    virtual double getDissimilarity(const std::vector<double>& p, Region* r) const;

    /*!
      \brief This method evaluates if the given region is homogeneous or not.

      \param r The region that will be evaluated.

      \retrun It returns true if the given region is homogeneous and false otherwise.
    */
    virtual bool isHomogenous(Region* r) const;

    /*!
      \brief This method performs the merging between two homogeneous regions.

      \param r1 The first region.
      \param r2 The second region.
    */
    virtual void merge(Region* r1, Region* r2) const = 0;

    //@}

  protected:

    /** @name Protected abstract virtual methods
      * Methods related with the merger metrics. */
    //@{

    /*!
      \brief This method evaluates the predicate of homogeneity between two regions considering a specific band.

      \param r1   The first region.
      \param r2   The second region.
      \param band The band that will be used.

      \retrun It returns true if the two regions are homogeneous and false otherwise.
    */
    virtual bool predicate(Region* r1, Region* r2, const std::size_t& band) const = 0;

    /*!
      \brief This method computes the dissimilarity value between a pixel and a region considering a specific band.

      \param p The pixel values.
      \param r The region.
      \param band The band that will be used.

      \retrun It returns the dissimilarity value between the given pixel and the given region.
    */
    virtual double getDissimilarity(const std::vector<double>& p, Region* r, const std::size_t& band) const = 0;

    /*!
      \brief This method evaluates if the given region is homogeneous or not considering a specific band.

      \param r The region that will be evaluated.
      \param band The band that will be used.

      \retrun It returns true if the given region is homogeneous and false otherwise.
    */
    virtual bool isHomogenous(Region* r, const std::size_t& band) const = 0;

    //@}

  public:

    /** @name Auxiliary methods
      * Auxiliary methods related with the mergers. */
    //@{

    /*!
      \brief This method computes the squared euclidean distance between two regions.

      \param r1 The first region.
      \param r2 The second region.

      \retrun The squared euclidean distance between two regions.
    */
    double getSquaredEuclideanDistance(Region* r1, Region* r2) const;

    /*!
      \brief This method computes the squared euclidean distance between two pixels.

      \param p1 The first pixel.
      \param p2 The second pixel.

      \retrun The squared euclidean distance between two pixels.
    */
    double getSquaredEuclideanDistance(const std::vector<double>& p1,
                                       const std::vector<double>& p2) const;

    /*!
      \brief This method computes the euclidean distance between two regions.

      \param r1 The first region.
      \param r2 The second region.

      \retrun The euclidean distance between two regions.
    */
    double getEuclideanDistance(Region* r1, Region* r2) const;

    /*!
      \brief This method computes the euclidean distance between two pixels.

      \param p1 The first pixel.
      \param p2 The second pixel.

      \retrun The euclidean distance between two pixels.
    */
    double getEuclideanDistance(const std::vector<double>& p1,
                                const std::vector<double>& p2) const;

    //@}

  protected:

    std::map<std::string, double> m_params; //!< The specific parameter list of this merger.
    bool m_strictMode;                      //!< A flag that indicates if this merger is on strict mode.

  friend class CompositeMerger;
};

#endif // __MULTISEG_INTERNAL_ABSTRACTMERGER_H
