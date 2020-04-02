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
  \file ParallelMultiSegStrategy.h

  \brief Multi-threaded image segmenter MultiSeg strategy.

  \author Douglas Uba <douglas@dpi.inpe.br>

  \note It is a prototype under development!
*/

#ifndef __MULTISEG_INTERNAL_PARALLELMULTISEGSTRATEGY_H
#define __MULTISEG_INTERNAL_PARALLELMULTISEGSTRATEGY_H

// MultiSeg
#include "Config.h"
#include "MultiSeg.h"

// TerraLib PDI
#include <terralib/image_processing/TePDIParaSegStrategy.hpp>

// STL
#include <vector>

class MSEGEXPORT ParallelMultiSegStrategy : public TePDIParaSegStrategy
{
  public :

    ParallelMultiSegStrategy(const TePDIParaSegStrategyParams& params);

    ~ParallelMultiSegStrategy();

    // overloaded
    bool execute(const RasterDataVecT& rasterDataVector, TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& outSegsMatrix);

    // overloaded
    bool setParameters(const TePDIParameters& params);

    // overloaded
    bool mergeSegments(TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT& centerMatrix,
                       TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* topMatrixPtr,
                       TePDIParaSegSegmentsBlock::SegmentsPointersMatrixT* leftMatrixPtr);

    // overloaded
    double getMemUsageFactor() const;

    // overloaded
    unsigned int getMinimumBlockWH() const;

  private:

    bool checkParameters(const TePDIParameters& params);

  protected :

    /*! A raster generated from the input raster data vector. */
    TePDITypes::TePDIRasterPtrType m_inputRasterPtr;

    /*! Segmentation algorithm parameters */
    TePDIParameters m_segParams;

    /*! Segmenter instance */
    MultiSeg m_segmenter;

    /*! Maximum allowed euclidean distance parameter */
    double m_eucTreshold;

    TeProjection* m_inputProjection;
};

#endif // __MULTISEG_INTERNAL_PARALLELMULTISEGSTRATEGY_H
