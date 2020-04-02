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
  \file Config.h

  \brief Configuration flags for the MultiSeg implementation.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_CONFIG_H
#define __MULTISEG_INTERNAL_CONFIG_H

/** @name DLL/LIB Module
 *  Flags for building MultiSeg as a DLL or as a Static Library
 */
//@{

/*!
  \def MSEGEXPORT

  \brief You can use this macro in order to export/import classes and functions from this code.

  \note If you want to compile MultiSeg as DLL in Windows, remember to insert MSEGDLL into the project's list of defines.

  \note If you want to compile MultiSeg as an Static Library under Windows, remember to insert the MSEGSTATIC flag into the project list of defines.
*/
#ifdef WIN32

  #ifdef _MSC_VER 
    #pragma warning( disable : 4251 )
    #pragma warning( disable : 4275 )
    #pragma warning( disable : 4290 )
  #endif

  #ifdef MSEGSTATIC
    #define MSEGEXPORT                          // Don't need to export/import... it is a static library
  #elif MSEGDLL
    #define MSEGEXPORT  __declspec(dllexport)   // export DLL information
  #else
    #define MSEGEXPORT  __declspec(dllimport)   // import DLL information
  #endif 
#else
  #define MSEGSTATIC
#endif

//@}

#endif  // __MULTISEG_INTERNAL_CONFIG_H
