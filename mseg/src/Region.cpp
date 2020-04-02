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
  \file Region.cpp

  \brief This class represents an area in the image being segmented. i.e. a region.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "Region.h"

// STL
#include <algorithm>
#include <iostream>
#include <cmath>

Region::Region(const std::size_t& id, const std::vector<double>& pixel,
               const std::size_t& lin, const std::size_t& col)
  : m_id(id),
    m_size(1),
    m_mean(pixel),
    m_xStart(col),
    m_xBound(col + 1),
    m_yStart(lin),
    m_yBound(lin + 1)
{
  m_variance.resize(pixel.size(), 0.0);
  m_cv.resize(pixel.size(), 0.0);
}

Region::~Region()
{
}

const std::size_t& Region::getId() const
{
  return m_id;
}

const std::size_t& Region::getSize() const
{
  return m_size;
}

void Region::setSize(const std::size_t& size)
{
  m_size = size;
}

void Region::updateBounds(const std::size_t& scale,
                          const std::size_t& maxLin,
                          const std::size_t& maxCol)
{
  if(m_xStart)
    m_xStart = m_xStart * scale - 1;

  if(m_yStart)
    m_yStart = m_yStart * scale - 1;

  m_xBound = std::min(m_xBound * scale, maxCol);
  m_yBound = std::min(m_yBound * scale, maxLin);
}

void Region::updateBounds(Region* region)
{
  m_xStart = std::min(m_xStart, region->m_xStart);
  m_yStart = std::min(m_yStart, region->m_yStart);
  m_xBound = std::max(m_xBound, region->m_xBound);
  m_yBound = std::max(m_yBound, region->m_yBound);
}

void Region::updateXStart(const std::size_t& x)
{
  m_xStart = std::min(m_xStart, x);
}

void Region::updateYStart(const std::size_t& y)
{
  m_yStart = std::min(m_yStart, y);
}

void Region::updateXBound(const std::size_t& x)
{
  m_xBound = std::max(m_xBound, x);
}

void Region::updateYBound(const std::size_t& y)
{
  m_yBound = std::max(m_yBound, y);
}

const std::size_t& Region::getXStart() const
{
  return m_xStart;
}

const std::size_t& Region::getYStart() const
{
  return m_yStart;
}

const std::size_t& Region::getXBound() const
{
  return m_xBound;
}

const std::size_t& Region::getYBound() const
{
  return m_yBound;
}

std::vector<double>& Region::getMean()
{
  return m_mean;
}

std::vector<double>& Region::getVariance()
{
  return m_variance;
}

std::vector<double>& Region::getCV()
{
  return m_cv;
}

void Region::setMean(const std::vector<double>& mean)
{
  m_mean = mean;
}

void Region::setVariance(const std::vector<double>& variance)
{
  m_variance = variance;
}

void Region::setCV(const std::vector<double>& cv)
{
  m_cv = cv;
}

void Region::addNeighbour(Region* region)
{
  if(isNeighbour(region))
    return;

  m_neighbours.push_back(region);
}

void Region::removeNeighbour(Region* region)
{
  m_neighbours.remove(region);
}

std::list<Region*>& Region::getNeighbours()
{
  return m_neighbours;
}

bool Region::isNeighbour(Region* region)
{
  std::list<Region*>::iterator it = std::find(m_neighbours.begin(),
                                              m_neighbours.end(),
                                              region);
  return it != m_neighbours.end();
}

std::size_t Region::getNBands() const
{
  return m_mean.size();
}
