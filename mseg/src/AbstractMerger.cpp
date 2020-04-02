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
  \file AbstractMerger.cpp

  \brief The merger class definition.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "AbstractMerger.h"
#include "Region.h"

// STL
#include <cassert>

AbstractMerger::AbstractMerger()
  : m_strictMode(true)
{
}

AbstractMerger::~AbstractMerger()
{
}

void AbstractMerger::setParam(const std::string& name, const double& value)
{
  m_params[name] = value;
}

void AbstractMerger::setParams(const std::map<std::string, double>& params)
{
  m_params = params;
}

double AbstractMerger::getParam(const std::string& name) const
{
  std::map<std::string, double>::const_iterator it = m_params.find(name);

  assert(it != m_params.end());

  return it->second;
}

const std::map<std::string, double>& AbstractMerger::getParams() const
{
  return m_params;
}

bool AbstractMerger::predicate(Region* r1, Region* r2) const
{
  assert(r1);
  assert(r2);
  assert(r1->getNBands() ==  r2->getNBands());

  // Count the number of valid predicates
  std::size_t valids = 0;

  // Evaluates the predicate for each band
  for(std::size_t i = 0; i < r1->getNBands(); ++i)
  {
    bool isValid = predicate(r1, r2, i);

    if(isValid == false && m_strictMode == true)
      return false;

    if(isValid == true && m_strictMode == false)
      return true;

    if(isValid == true)
      ++valids;
  }

  if(valids > 0)
    return true;

  return false;
}

double AbstractMerger::getDissimilarity(const std::vector<double>& p, Region* r) const
{
  double value = 0.0;
  
  for(std::size_t i = 0; i < p.size(); ++i)
    value += getDissimilarity(p, r, i);

  return value;
}

bool AbstractMerger::isHomogenous(Region* r) const
{
  assert(r);

  // Count the number of valid predicates
  std::size_t homogenous = 0;

  // Evaluates the homogeneity for each band
  for(std::size_t i = 0; i < r->getNBands(); ++i)
  {
    bool isHomo = isHomogenous(r, i);

    if(isHomo == false && m_strictMode == true)
      return false;

    if(isHomo == true && m_strictMode == false)
      return true;

    if(isHomo == true)
      ++homogenous;
  }

  if(homogenous > 0)
    return true;

  return false;
}

double AbstractMerger::getSquaredEuclideanDistance(Region* r1, Region* r2) const
{
  assert(r1);
  assert(r2);

  return getSquaredEuclideanDistance(r1->getMean(), r2->getMean());
}

double AbstractMerger::getSquaredEuclideanDistance(const std::vector<double>& p1,
                                                   const std::vector<double>& p2) const
{
  assert(p1.size() == p2.size());

  double distance = 0.0;

  for(std::size_t i = 0; i < p1.size(); ++i)
    distance += abs(p1[i] - p2[i]);

  return distance;
}

double AbstractMerger::getEuclideanDistance(Region* r1, Region* r2) const
{
  double distance = getSquaredEuclideanDistance(r1->getMean(), r2->getMean());

  return sqrt(distance);
}

double AbstractMerger::getEuclideanDistance(const std::vector<double>& p1,
                                            const std::vector<double>& p2) const
{
  double distance = getSquaredEuclideanDistance(p1, p2);

  return sqrt(distance);
}
