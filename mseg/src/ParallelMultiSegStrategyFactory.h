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
  \file ParallelMultiSegStrategyFactory.h

  \brief Multi-threaded image segmenter MultiSeg strategy factory.

  \author Douglas Uba <douglas@dpi.inpe.br>

  \note It is a prototype under development!
*/

#ifndef __MULTISEG_INTERNAL_PARALLELMULTISEGSTRATEGYFACTORY_H
#define __MULTISEG_INTERNAL_PARALLELMULTISEGSTRATEGYFACTORY_H

// MultiSeg
#include "Config.h"

// TerraLib PDI
#include <terralib/image_processing/TePDIParaSegStrategyFactory.hpp>

// Forward declaration
class TePDIParaSegStrategy;

class MSEGEXPORT ParallelMultiSegStrategyFactory : public TePDIParaSegStrategyFactory
{
  public :

    ParallelMultiSegStrategyFactory();

    ~ParallelMultiSegStrategyFactory();

    // overloaded
    TePDIParaSegStrategy* build(const TePDIParaSegStrategyFactoryParams& arg);
};

#endif // __MULTISEG_INTERNAL_PARALLELMULTISEGSTRATEGYFACTORY_H
