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
  \file AbstractOutputter.h

  \brief Abstract class that outputs results of MultiSeg algorithm.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_ABSTRACTOUTPUTTER_H
#define __MULTISEG_INTERNAL_ABSTRACTOUTPUTTER_H

// MultiSeg
#include "Config.h"
#include "Pyramid.h"
#include "MultiSeg.h"

/*!
  \class AbstractOutputter

  \brief Abstract class that outputs results of MultiSeg algorithm.

  \sa FileOutputter
*/
class MSEGEXPORT AbstractOutputter
{
  public:

    /*! \brief Default constructor. */
    AbstractOutputter() {}

    /*! \brief Virtual destructor. */
    virtual ~AbstractOutputter() {}

    /*!
      \brief This method outputs the given image hierarchical pyramid.

      \param pyramid The image hierarchical pyramid.
    */
    virtual void outputPyramid(const Pyramid& pyramid) = 0;

    /*!
      \brief This method outputs the MultiSeg results considering the informed current level.

      \param mseg         The MultiSeg algorithm.
      \param currentLevel The current level.
    */
    virtual void output(const MultiSeg& mseg, const std::size_t& currentLevel) = 0;
};

#endif // __MULTISEG_INTERNAL_ABSTRACTOUTPUTTER_H
