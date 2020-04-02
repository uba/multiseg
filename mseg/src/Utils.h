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
  \file Utils.h

  \brief Utility methods related with MultiSeg.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_UTILS_H
#define __MULTISEG_INTERNAL_UTILS_H

// MultiSeg
#include "Enums.h"
#include "MultiSeg.h"

// TerraLib PDI
#include <terralib/image_processing/TePDIParameters.hpp>
#include <terralib/image_processing/TePDITypes.hpp>

// STL
#include <map>
#include <string>
#include <vector>

namespace Utils
{
  /*!
    \brief This method converts an amplitude image to an intensity image.

    \param image The image with amplitude values.

    \return The image with intensity values.
  */
  MSEGEXPORT TePDITypes::TePDIRasterPtrType Amplitude2Intensity(TePDITypes::TePDIRasterPtrType& image);

  /*!
    \brief This method generates a identifier value based on the given line and column numbers.

    \param lin   The line number.
    \param col   The column number.
    \param nCols The total number of columns.

    \return The identifier value.
  */
  MSEGEXPORT std::size_t GenerateId(const std::size_t& lin, const std::size_t& col, const std::size_t& nCols);

  /*!
    \brief This method decodes a identifier value to line and column numbers.

    \param id    The identifier value.
    \param lin   The line number that will be filled.
    \param col   The column number that will be filled.
    \param nCols The total number of columns.

    \return The identifier value.
  */
  MSEGEXPORT void DecodeId(const std::size_t& id, std::size_t& lin, std::size_t& col, const std::size_t& nCols);

  /*!
    \brief This method tokenizes a given string with a delimiter of your own choice.

    \param str        The string to be tokenize.
    \param tokens     The output vector with the tokens.
    \param delimiters The character delimiters.
  */
  MSEGEXPORT void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " ");

  /*!
    \brief This method generates output file names based on MultiSeg algorithm parameters.

    \param params           The MultiSeg algorithm parameters.
    \param inputFilePath    The input image file path.
    \param outputFilesNames The output map with the file names.
    \param separator        The separator that will be used to separate the file name parts.
  */
  MSEGEXPORT void GenerateOutputFilesNames(const TePDIParameters& params, const std::string& inputFilePath, std::map<OutputResultType, std::string>& outputFilesNames, const std::string& separator = "_");

  /*!
    \brief This method saves the results of MultiSeg algorithm to files.

    \param mseg              The MultiSeg algorithm.
    \param outputFilesNames  The output file names.
    \param resizeResults     A flag that indicates if the results must be resized.

    \note This method will save 3 files: - labelled image [ids]; - cartoon image [mean, variance and CV] and - polygons [ShapeFile format].
  */
  MSEGEXPORT void SaveResult(const MultiSeg& mseg, const std::string& outputDir, std::map<OutputResultType, std::string>& outputFilesNames, bool resizeResults = true);

  /*!
    \brief This method computes the maximum levels of hierarchical pyramid based on the given sizes.

    \param nlines      The number of lines.
    \param ncols       The number of columns.
    \param minimumSize The minimum size acceptable.

    \return The maximum levels of hierarchical pyramid based on the given sizes.
  */
  MSEGEXPORT std::size_t ComputeMaxLevels(const std::size_t& nlines, const std::size_t& ncols, const std::size_t& minimumSize = 2);
}

#endif  // __MULTISEG_INTERNAL_UTILS_H
