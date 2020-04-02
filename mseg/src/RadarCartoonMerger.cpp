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
  \file RadarCartoonMerger.cpp

  \brief This class implements the merger to radar cartoon images.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "RadarCartoonMerger.h"
#include "Region.h"

// Boost
#include <boost/math/distributions/students_t.hpp>

// STL
#include <cassert>
#include <cmath>

RadarCartoonMerger::RadarCartoonMerger()
  : EuclideanMerger()
{
}

RadarCartoonMerger::~RadarCartoonMerger()
{
}

bool RadarCartoonMerger::predicate(Region* r1, Region* r2, const std::size_t& band) const
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

  // region vs. pixel
  if(r1->getSize() > 1 && r2->getSize() == 1)
  {
    // Gets the pre-computed vcritic factor
    double vcriticFactor = getParam("vcritic_factor");

    // r1 is the region
    double regionMean = mean1[band];

    // r2 is the pixel
    double pixelValue = mean2[band];

    // Computes the vcritic
    double vcritic =  vcriticFactor * regionMean;

    if(pixelValue > vcritic)
      return false;
    else
      return true;
  }

  // region vs. region
  assert(r1->getSize() > 1 && r2->getSize() > 1);

  // Gets the current ENL
  double enl = getParam("ENL");

  // Gets the mean A
  double meanA = mean1[band];

  // Gets the mean B
  double meanB = mean2[band];

  // Computes the variance A and B (estimate)
  double varianceA = (meanA * meanA) / enl;
  double varianceB = (meanB * meanB) / enl;

  // Gets the confidence level
  double confidenceLevel = getParam("confidence_level");

  // The probability
  double probability = 1 - confidenceLevel;

  // Degrees of Freedom (t-Student)
  double dof = r1->getSize() + r2->getSize() - 2.0;

  double rootVarianceAB = sqrt(((((r1->getSize() - 1) * varianceA) + ((r2->getSize() - 1) * varianceB)) / (dof))
                               * ((1.0 / static_cast<double>(r1->getSize())) + (1.0 / static_cast<double>(r2->getSize()))));

  assert(rootVarianceAB != 0.0);

  double tValue = std::abs(meanA - meanB) / rootVarianceAB;

  boost::math::students_t_distribution<double> tStudent(dof);

  double p = 1.0 - boost::math::cdf(tStudent, tValue);

  if(p >= probability)
    return true;
  else
    return false;
}

double RadarCartoonMerger::getDissimilarity(const std::vector<double>& p, Region* r, const std::size_t& band) const
{
  // Gets the region mean
  const std::vector<double>& mean = r->getMean();
  assert(band < mean.size());

  // Gets the squared euclidean distance between the pixel and the region
  double distance = EuclideanMerger::getDissimilarity(p, r, band);

  // Gets the current ENL
  double enl = getParam("ENL");

  double n = mean[band] / sqrt(enl);

  return distance / n;
}
