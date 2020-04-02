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

  \brief Main file of MultiSeg user-interface.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg UI
#include "MultiSegWidget.h"

// TerraLib 
#include <terralib/kernel/TeException.h>

// Qt
#include <QtGui/QApplication>
#include <QtGui/QPlastiqueStyle>

// STL
#include <iostream>

int main(int argc, char** argv)
{
  QApplication app(argc, argv);

  QApplication::setStyle(new QPlastiqueStyle);

  std::cout << std::endl << ":: MultiSeg ::" << std::endl << std::endl;

  try
  {
    MultiSegWidget msw;
    msw.show();

    app.exec();
  }
  catch(const TeException& e)
  {
    std::cout << std::endl << "An exception has occurred: " << e.message() << std::endl;

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return EXIT_FAILURE;
  }
  catch(...)
  {
    std::cout << std::endl << "An unexpected exception has occurred!" << std::endl;

    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
