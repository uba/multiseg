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
  \file CVTable.cpp

  \brief This class represents a table of Coefficients of Variation.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "CVTable.h"
#include "Utils.h"

// STL
#include <cassert>
#include <cmath>
#include <fstream>
#include <limits>

CVTable::CVTable()
{
}

CVTable::CVTable(const double& cv)
{
  load(cv);
}

CVTable::~CVTable()
{
}

void CVTable::load(const double& cv)
{
  verifyCV(cv);

  if(cv > 0.999)
  {
    std::cout << "** Note: using confidence level = 100%" << std::endl;
    return;
  }

  if(cv == 0.999)
    load("./tables/tab_01.csv");
  else if(cv == 0.995)
    load("./tables/tab_05.csv");
  else if(cv == 0.99)
    load("./tables/tab_1.csv");
  else if(cv == 0.95)
    load("./tables/tab_5.csv");
  else if(cv == 0.90)
    load("./tables/tab_10.csv");
  else if(cv == 0.85)
    load("./tables/tab_15.csv");
  else if(cv == 0.80)
    load("./tables/tab_20.csv");
}

double CVTable::getCV(std::size_t& ENL, std::size_t& nSamples) const
{
  if(m_table.empty())
    return (std::numeric_limits<double>::max)();

  ENL = static_cast<std::size_t>(min(static_cast<int>(ENL), static_cast<int>(250)));
  std::size_t nChosen = static_cast<std::size_t>(min(static_cast<int>(nSamples), static_cast<int>(9000)));

  // Rounding the 'nSamples' to the numbers used on table
  std::size_t minDiff = std::string::npos;
  for(std::size_t i = 0; i < m_header.size(); ++i)
  {
    std::size_t diffLower = static_cast<std::size_t>(std::abs(static_cast<int>(nSamples - m_header[i])));
    if(diffLower < minDiff)
    {
      minDiff = diffLower;
      nChosen = m_header[i];
    }
  }

  nSamples = nChosen;

  assert(nSamples >= 10 && nSamples <= 9000);

  std::pair<std::size_t, std::size_t> key(ENL, nSamples);

  std::map<std::pair<std::size_t, std::size_t>, double>::const_iterator it = m_table.find(key);

  assert(it != m_table.end());

  return it->second;
}

void CVTable::verifyCV(const double& cv)
{
  bool isValid = cv == 0.99999 || cv == 0.999  || cv == 0.995 || cv == 0.99 ||
                 cv == 0.95    || cv == 0.90   || cv == 0.85  || cv == 0.80;

  TEAGN_TRUE_OR_THROW(isValid, "Invalid confidence level. The allowed values are: 0.99999, 0.999, 0.995, 0.99, 0.95, 0.90, 0.85, 0.80.");
}

void CVTable::load(const std::string& path)
{
  assert(!path.empty());

  std::cout << "Loading " << path << std::endl;

  // Open a .csv file that contains a set of CV values
  std::ifstream file(path.c_str(), std::ifstream::in);

  TEAGN_TRUE_OR_THROW(file.good(), "Error loading table of Coefficients of Variation. The .csv file not exist at tables/ directory.");

  std::string row;
  std::vector<std::string> cvs;
  std::size_t ENL = 0;

  while(file.good())
  {
    // Reads the current line
    std::getline(file, row);

    // Each row = 1 look
    ++ENL;

    // Extracts the CV values
    Utils::Tokenize(row, cvs, ";");
    assert(cvs.size() == 39);

    std::size_t nSamples = 10;
    for(std::size_t i = 0; i < cvs.size(); ++i) // for each CV value
    {
      if(m_header.size() < 39)
        m_header.push_back(nSamples);

      std::pair<std::size_t, std::size_t> key(ENL, nSamples);
      m_table[key] = atof(cvs[i].c_str());

      if(nSamples < 100)
      {
        nSamples += 10;
        continue;
      }

      if(nSamples < 1000)
      {
        nSamples += 50;
        continue;
      }

      if(nSamples < 4000)
      {
        nSamples += 500;
        continue;
      }

      nSamples += 1000;
    }

    cvs.clear();
  }
}
