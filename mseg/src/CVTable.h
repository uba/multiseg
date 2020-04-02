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
  \file CVTable.h

  \brief This class represents a table of Coefficients of Variation.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_CVTABLE_H
#define __MULTISEG_INTERNAL_CVTABLE_H

// MultiSeg
#include "Config.h"

// STL
#include <map>
#include <string>
#include <utility>
#include <vector>

/*!
  \class CVTable

  \brief This class represents a table of Coefficients of Variation.
*/
class MSEGEXPORT CVTable
{
  public:

    /*! \brief Default constructor. */
    CVTable();

    /*!
      \brief Constructor.

      \param confidenceLevel The confidence level of the table.

      \note The given confidence level will be used to loading the table values from a .csv file.

      \note The allowed values are: 0.99999, 0.999, 0.995, 0.99, 0.95, 0.90, 085, 0.80.

      \note The method load will be called.
    */
    CVTable(const double& cv);

    /*! \brief Destructor. */
    ~CVTable();

    /*!
      \brief This method loads the table values from a .csv file from the given confidence level.

      \param confidenceLevel The confidence level of the table.

      \note The allowed values are: 0.99999, 0.999, 0.995, 0.99, 0.95, 0.90, 0.85, 0.80.
    */
    void load(const double& cv);

    /*!
      \brief This method returns the coefficient of variation considering the number of looks and the number of samples.

      \param ENL      The number of looks.
      \param nSamples The number of samples.

      \note The coefficient of variation considering the number of looks and the number of samples.
    */
    double getCV(std::size_t& ENL, std::size_t& nSamples) const;

  private:

    /*!
      \brief This method verifies if the given coefficient of variation is valid.

      \param confidenceLevel The confidence level that will be verified.

      \note The allowed values are: 0.99999, 0.999, 0.995, 0.99, 0.95, 0.90, 0.85, 0.80.
    */
    void verifyCV(const double& cv);

    /*!
      \brief This method loads the table values from a .csv file.

      \param path The path of .csv file.
    */
    void load(const std::string& path);

  private:

    std::vector<std::size_t> m_header;                             //!< The table header (Number of Samples).
    std::map<std::pair<std::size_t, std::size_t>, double> m_table; //!< The table struct [ENL, N Samples] -> cv.
};

#endif // __MULTISEG_INTERNAL_CVTABLE_H
