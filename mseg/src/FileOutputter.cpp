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
  \file FileOutputter.cpp

  \brief A class that outputs the results of MultiSeg algorithm to file.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "FileOutputter.h"
#include "Utils.h"

// TerraLib
#include <terralib/image_processing/TePDIUtils.hpp>

// Boost
#include <boost/lexical_cast.hpp>

FileOutputter::FileOutputter(bool resizeResults)
  : AbstractOutputter(),
    m_resizeResults(resizeResults),
    m_useNumberOfRegionsSuffix(true)
{
}

FileOutputter::~FileOutputter()
{
}

void FileOutputter::outputPyramid(const Pyramid& pyramid)
{
  for(std::size_t i = 0; i < pyramid.getNLevels(); ++i)
    TePDIUtils::TeRaster2Geotiff(pyramid.getLevel(i), m_outputDir + "/" + m_inputImageFileName + "_pyramid_level_" + Te2String(i) + ".tif", false);
}

void FileOutputter::output(const MultiSeg& mseg, const std::size_t& currentLevel)
{
  // Suffix string
  std::string suffix = "_level_" + boost::lexical_cast<std::string>(currentLevel);

  if(m_useNumberOfRegionsSuffix)
    suffix += "_nreg_" + boost::lexical_cast<std::string>(mseg.getRegions().size());

  // The current level output file names
  std::map<OutputResultType, std::string> names = m_outputFileNames;
  names[LabelledImage] += suffix;
  names[CartoonImage] += suffix;
  names[Vector]  += suffix;

  // Saves the result
  Utils::SaveResult(mseg, m_outputDir, names, m_resizeResults);
}

void FileOutputter::setInputImageFileName(const std::string& name)
{
  m_inputImageFileName = name;
}

void FileOutputter::setOutputDir(const std::string& dir)
{
  m_outputDir = dir;
}

void FileOutputter::setOutputFileNames(const std::map<OutputResultType, std::string>& names)
{
  m_outputFileNames = names;
}

void FileOutputter::useNumberOfRegionsSuffix(bool value)
{
  m_useNumberOfRegionsSuffix = value;
}
