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
  \file Main.cpp

  \brief Main file of MultiSeg Operator for InterIMAGE application.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg Operator
#include "MultiSegOp.h"

// STL
#include <iostream>

int main(int argc, char** argv)
{
   std::cout << ":: MultiSeg Operator for InterIMAGE ::" << std::endl << std::endl;

   return mseg::op::Run(argc, argv);
}
