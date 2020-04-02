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
  \file EuclideanMerger.cpp

  \brief This class implements the euclidean merger.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "EuclideanMerger.h"
#include "Region.h"

// STL
#include <cassert>
#include <cmath>

EuclideanMerger::EuclideanMerger()
  : AbstractMerger()
{
}

EuclideanMerger::~EuclideanMerger()
{
}

void EuclideanMerger::merge(Region* r1, Region* r2) const
{
  assert(r1);
  assert(r2);
  assert(r1->getNBands() ==  r2->getNBands());

  // Updating bounds
  r1->updateBounds(r2);

  // Updating Mean
  std::vector<double>& mean1 = r1->getMean();
  const std::vector<double>& mean2 = r2->getMean();

  const std::size_t& size1 = r1->getSize();
  const std::size_t& size2 = r2->getSize();

  for(std::size_t i = 0; i < mean1.size(); ++i)
    mean1[i] = ((mean1[i] * size1) + (mean2[i] * size2)) / ((double)(size1 + size2));

  // Updating size
  r1->setSize(size1 + size2);
}

bool EuclideanMerger::predicate(Region* r1, Region* r2, const std::size_t& band) const
{
  assert(r1);
  assert(r2);
  assert(r1->getNBands() ==  r2->getNBands());

  const std::vector<double>& mean1 = r1->getMean();
  const std::vector<double>& mean2 = r2->getMean();

  assert(band < mean1.size());

  double t = getParam("euclidean_distance_threshold");

  return abs(mean1[band] - mean2[band]) <= t;
}

double EuclideanMerger::getDissimilarity(const std::vector<double>& p, Region* r, const std::size_t& band) const
{
  assert(r);

  const std::vector<double>& mean = r->getMean();

  assert(p.size() == mean.size());
  assert(band < p.size());

  return abs(p[band] - mean[band]);
}

bool EuclideanMerger::isHomogenous(Region* r, const std::size_t& band) const
{
  const std::vector<double>& cv = r->getCV();

  assert(band < cv.size());

  double t = getParam("cv_threshold");

  return cv[band] <= t;
}
