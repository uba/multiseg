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
  \file CompositeMerger.cpp

  \brief This class implements the composite merger concept.

  \author Douglas Uba <douglas@dpi.inpe.br>

  \note It is a prototype under development!
*/

// MultiSeg
#include "CompositeMerger.h"

// STL
#include <cassert>

CompositeMerger::CompositeMerger()
  : AbstractMerger()
{
}

CompositeMerger::~CompositeMerger()
{
  for(std::size_t i = 0; i < m_mergers.size(); ++i)
    delete m_mergers[i];
}

void CompositeMerger::setParam(const std::string& name, const double& value)
{
  for(std::size_t i = 0; i < m_mergers.size(); ++i)
    m_mergers[i]->setParam(name, value);
}

void CompositeMerger::setParams(const std::map<std::string, double>& params)
{
  for(std::size_t i = 0; i < m_mergers.size(); ++i)
    m_mergers[i]->setParams(params);
}

void CompositeMerger::addMerger(AbstractMerger* merger, bool needStatsUpdate)
{
  m_mergers.push_back(merger);
  m_needStatsUpdate.push_back(needStatsUpdate);
}

void CompositeMerger::merge(Region* r1, Region* r2) const
{
  for(std::size_t i = 0; i < m_mergers.size(); ++i)
    if(m_needStatsUpdate[i] == true)
      m_mergers[i]->merge(r1, r2);
}

bool CompositeMerger::predicate(Region* r1, Region* r2, const std::size_t& band) const
{
  return getMerger(band)->predicate(r1, r2, band);
}

double CompositeMerger::getDissimilarity(const std::vector<double>& p, Region* r, const std::size_t& band) const
{
  return getMerger(band)->getDissimilarity(p, r, band);
}

bool CompositeMerger::isHomogenous(Region* r, const std::size_t& band) const
{
  return getMerger(band)->isHomogenous(r, band);
}

AbstractMerger* CompositeMerger::getMerger(const std::size_t& i) const
{
  assert(i < m_mergers.size());

  AbstractMerger* merger = m_mergers[i];
  assert(merger != 0);

  return merger;
}
