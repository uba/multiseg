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
  \file Pyramid.cpp

  \brief This class represents an image hierarchical pyramid.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "Pyramid.h"

// TerraLib
#include <terralib/kernel/TeRaster.h>
#include <terralib/kernel/TeRasterRemap.h>
#include <terralib/image_processing/TePDIPIManager.hpp>
#include <terralib/image_processing/TePDIParameters.hpp>
#include <terralib/image_processing/TePDIUtils.hpp>

// STL
#include <cassert>

Pyramid::Pyramid(const TePDITypes::TePDIRasterPtrType& image, const std::size_t& nLevels, const bool& progressEnabled)
  : m_progressEnabled(progressEnabled)
{
  m_levels.resize(nLevels + 1);
  m_levels[0] = image;

  if(nLevels > 0)
    build();
}

Pyramid::Pyramid(const TePDITypes::TePDIRasterPtrType& image, const std::size_t& nLevels, const std::vector<std::size_t>& bands, const bool& progressEnabled)
  : m_bands(bands),
    m_progressEnabled(progressEnabled)
{
  m_levels.resize(nLevels + 1);
  m_levels[0] = image;

  build();
}

Pyramid::~Pyramid()
{
}

std::size_t Pyramid::getNLevels() const
{
  return m_levels.size();
}

TePDITypes::TePDIRasterPtrType Pyramid::getLevel(const std::size_t& i) const
{
  assert(i < m_levels.size());
  return m_levels[i];
}

void Pyramid::releaseLevel(const std::size_t& i)
{
  assert(i < m_levels.size());
  m_levels[i].reset(0);
}

TePDITypes::TePDIRasterPtrType Pyramid::resize(TePDITypes::TePDIRasterPtrType& image,
                                               const std::size_t& nlin, const std::size_t& ncol)
{
  TeBox bbox = image->params().boundingBox();

  TeRasterParams params = image->params();
  params.decoderIdentifier_ = "SMARTMEM";
  params.mode_ = 'w';
  params.resolution_ = params.resolution_ / 2;
  params.boundingBoxLinesColumns(bbox.x1_, bbox.y1_, bbox.x2_, bbox.y2_, nlin, ncol);

  TeRaster* resized = new TeRaster(params);
  resized->init();

  TeRasterRemap remap;
  remap.setInterpolation(1);
  remap.setInput(image.nakedPointer());
  remap.setOutput(resized);
  remap.apply();

  return TePDITypes::TePDIRasterPtrType(resized);
}

TePDITypes::TePDIRasterPtrType Pyramid::resize(TePDITypes::TePDIRasterPtrType& labelledImage, TeRasterParams params)
{
  params.decoderIdentifier_ = "SMARTMEM";
  params.mode_ = 'w';
  params.nBands(1);
  params.setDataType(TeUNSIGNEDLONG);

  TeRaster* resized = new TeRaster(params);
  resized->init();

  TeRasterRemap remap;
  remap.setInterpolation(1);
  remap.setInput(labelledImage.nakedPointer());
  remap.setOutput(resized);
  remap.apply();

  return TePDITypes::TePDIRasterPtrType(resized);
}

TePDIStatistic* Pyramid::buildStats(const std::size_t& i)
{
  assert(!m_bands.empty());
  assert(i < m_levels.size());

  TePDIParameters statParams;

  // Input Raster & input bands
  TePDITypes::TePDIRasterVectorType rasters;
  std::vector<int> bands;

  for(std::size_t b = 0; b < m_bands.size(); ++b)
  {
    rasters.push_back(m_levels[i]);
    bands.push_back(m_bands[b]);
  }

  statParams.SetParameter("rasters", rasters);
  statParams.SetParameter("bands", bands);

  TePDIStatistic* stat = new TePDIStatistic;
  stat->ToggleProgInt(false);
  stat->Reset(statParams);

  return stat;
}

void Pyramid::build()
{
  TePDIPIManager progress("Building hierarchical pyramid", m_levels.size() - 1, m_progressEnabled);

  for(std::size_t i = 1; i < m_levels.size(); ++i)
  {
    // Gets the previous level
    TePDITypes::TePDIRasterPtrType previousLevel = m_levels[i - 1];

    // Gets the previous bounding box
    TeBox bbox = previousLevel->params().boundingBox();

    // Adjust new level parameters
    TeRasterParams params = previousLevel->params();
    params.decoderIdentifier_ = "SMARTMEM";
    params.mode_ = 'w';
    params.resolution_ = params.resolution_ * 2;
    params.boundingBoxResolution(bbox.x1_, bbox.y1_,bbox.x2_, bbox.y2_,
                                 params.resx_ * 2.0, params.resy_ * 2.0);

    // Create the new level
    TePDITypes::TePDIRasterPtrType newLevel(new TeRaster(params));
    newLevel->init();

    build(previousLevel, newLevel);

    // Store!
    m_levels[i] = TePDITypes::TePDIRasterPtrType(newLevel);

    progress.Update(i);
  }
}

void Pyramid::build(TePDITypes::TePDIRasterPtrType& previousLevel, TePDITypes::TePDIRasterPtrType& newLevel)
{
  int nLines = newLevel->params().nlines_;
  int nCols = newLevel->params().ncols_;
  int nBands = newLevel->params().nBands();

  double value = 0.0;

  for(int lin = 0; lin < nLines; ++lin)
  {
    int linToRead = lin * 2;
    for(int col = 0; col < nCols; ++col)
    {
      int colToRead = col * 2;
      for(int b = 0; b < nBands; ++b)
      {
        double mean = 0.0;
        std::size_t nPixels = 0;

        if(previousLevel->getElement(colToRead, linToRead, value, b))
        {
          mean += value;
          ++nPixels;
        }

        if(previousLevel->getElement(colToRead + 1, linToRead, value, b))
        {
          mean += value;
          ++nPixels;
        }

        if(previousLevel->getElement(colToRead, linToRead + 1, value, b))
        {
          mean += value;
          ++nPixels;
        }

        if(previousLevel->getElement(colToRead + 1, linToRead + 1, value, b))
        {
          mean += value;
          ++nPixels;
        }

        assert(nPixels != 0);

        newLevel->setElement(col, lin, mean / static_cast<double>(nPixels), b);
      }
    }
  }
}
