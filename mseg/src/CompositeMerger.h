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
  \file CompositeMerger.h

  \brief This class implements the composite merger concept.

  \author Douglas Uba <douglas@dpi.inpe.br>

  \note It is a prototype under development!
*/

#ifndef __MULTISEG_INTERNAL_COMPOSITEMERGER_H
#define __MULTISEG_INTERNAL_COMPOSITEMERGER_H

// MutiSeg
#include "Config.h"
#include "AbstractMerger.h"

// STL
#include <vector>

/*!
  \class CompositeMerger

  \brief This class implements the composite merger concept.

  \sa  AbstractMerger, EuclideanMerger, OpticalCartoonMerger, RadarCartoonMerger

  \note It is a prototype under development!
*/
class MSEGEXPORT CompositeMerger : public AbstractMerger
{
  public:

    /*! \brief Default constructor. */
    CompositeMerger();

    /*! \brief Destructor. */
    ~CompositeMerger();

    void setParam(const std::string& name, const double& value);

    void setParams(const std::map<std::string, double>& params);

    void addMerger(AbstractMerger* merger, bool needStatsUpdate);

    void merge(Region* r1, Region* r2) const;

  protected:

    bool predicate(Region* r1, Region* r2, const std::size_t& band) const;

    double getDissimilarity(const std::vector<double>& p, Region* r, const std::size_t& band) const;

    bool isHomogenous(Region* r, const std::size_t& band) const;

  private:

    AbstractMerger* getMerger(const std::size_t& i) const;

  private:

    std::vector<AbstractMerger*> m_mergers; //!< The list of mergers. Each merger is associated with a specific band.
    std::vector<bool> m_needStatsUpdate;    //!< A flag that indicates if a specific merger needs to update its statistical values internally.
};

#endif // __MULTISEG_INTERNAL_COMPOSITEMERGER_H
