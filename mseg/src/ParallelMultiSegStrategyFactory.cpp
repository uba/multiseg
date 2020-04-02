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
  \file ParallelMultiSegStrategyFactory.cpp

  \brief Multi-threaded image segmenter MultiSeg strategy factory.

  \author Douglas Uba <douglas@dpi.inpe.br>

  \note It is a prototype under development!
*/

// MultiSeg
#include "ParallelMultiSegStrategy.h"
#include "ParallelMultiSegStrategyFactory.h"

namespace
{
  ParallelMultiSegStrategyFactory sm_parallelMultiSegStrategyFactoryInstance;
}

ParallelMultiSegStrategyFactory::ParallelMultiSegStrategyFactory()
  : TePDIParaSegStrategyFactory("MultiSeg")
{
}

ParallelMultiSegStrategyFactory::~ParallelMultiSegStrategyFactory()
{
}

TePDIParaSegStrategy* ParallelMultiSegStrategyFactory::build(const TePDIParaSegStrategyFactoryParams& arg)
{
  return new ParallelMultiSegStrategy(arg.stratParams_);
}
