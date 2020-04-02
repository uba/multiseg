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
  \file OpticalCartoonMerger.cpp

  \brief This class implements the merger to optical cartoon images.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "OpticalCartoonMerger.h"
#include "Region.h"

// TerraLib
#include <terralib/kernel/TeUtils.h>

// Boost
#include <boost/math/distributions/students_t.hpp>
#include <boost/math/distributions/normal.hpp>

// STL
#include <cassert>
#include <cmath>

OpticalCartoonMerger::OpticalCartoonMerger()
  : EuclideanMerger()
{
}

OpticalCartoonMerger::~OpticalCartoonMerger()
{
}

bool OpticalCartoonMerger::predicate(Region* r1, Region* r2, const std::size_t& band) const
{
  assert(r1);
  assert(r2);
  assert(r1->getNBands() ==  r2->getNBands());

  // pixel vs. pixel or pixel vs. region
  if(r1->getSize() == 1)
    return EuclideanMerger::predicate(r1, r2, band);

  // Gets the regions mean
  const std::vector<double>& mean1 = r1->getMean();
  const std::vector<double>& mean2 = r2->getMean();

  assert(band < mean1.size());

  // Gets the mean A from given band
  double meanA = mean1[band];

  // Gets the mean B from given band
  double meanB = mean2[band];

  // Image total variance of band b-th
  double totalVariance = getParam("image_variance_" + Te2String(band));

  // Gets the confidence level
  double confidenceLevel = getParam("confidence_level");

  // The probability
  double probability = 1 - confidenceLevel;

  // region vs. pixel
  if(r1->getSize() > 1 && r2->getSize() == 1)
  {
    double zValue = std::abs(meanA - meanB) / sqrt(totalVariance);

    boost::math::normal_distribution<double> normal;

    double p = 1.0 - boost::math::cdf(normal, zValue);

    if(p >= probability)
      return true;
    else
      return false;
  }

  // region vs. region
  assert(r1->getSize() > 1 && r2->getSize() > 1);

  // Degrees of Freedom (t-Student)
  double dof = r1->getSize() + r2->getSize() - 2.0;

  double rootVariance = sqrt(totalVariance * ((1.0 / static_cast<double>(r1->getSize())) + (1.0 / static_cast<double>(r2->getSize()))));
  
  assert(rootVariance != 0.0);

  double tValue = std::abs(meanA - meanB) / rootVariance;

  boost::math::students_t_distribution<double> tStudent(dof);

  double p = 1.0 - boost::math::cdf(tStudent, tValue);

  if(p >= probability)
    return true;
  else
    return false;
}

double OpticalCartoonMerger::getDissimilarity(const std::vector<double>& p, Region* r, const std::size_t& band) const
{
  // Image total variance of band b-th
  double totalVariance = getParam("image_variance_" + Te2String(band));

  // Gets the squared euclidean distance between the pixel and the region
  double distance = EuclideanMerger::getDissimilarity(p, r, band);

  return distance / totalVariance;
}
