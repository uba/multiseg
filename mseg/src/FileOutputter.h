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
  \file FileOutputter.h

  \brief A class that outputs the results of MultiSeg algorithm to file.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_FILEOUTPUTTER_H
#define __MULTISEG_INTERNAL_FILEOUTPUTTER_H

// MutiSeg
#include "Config.h"
#include "Enums.h"
#include "AbstractOutputter.h"

// STL
#include <map>
#include <string>

/*!
  \class FileOutputter

  \brief A class that outputs the results of MultiSeg algorithm to file.

  \sa AbstractOutputter
*/
class MSEGEXPORT FileOutputter : public AbstractOutputter
{
  public:

    /*!
      \brief Constructor.

      \param resizeResults A flag that indicates if the results must be resized.
    */
    FileOutputter(bool resizeResults = false);

    /*! \brief Destructor. */
    ~FileOutputter();

    void outputPyramid(const Pyramid& pyramid);

    void output(const MultiSeg& mseg, const std::size_t& currentLevel);

    void setInputImageFileName(const std::string& name);

    void setOutputDir(const std::string& dir);

    void setOutputFileNames(const std::map<OutputResultType, std::string>& names);

    void useNumberOfRegionsSuffix(bool value);

  private:

    std::string m_inputImageFileName;                           //!< The input image file name.
    std::string m_outputDir;                                    //!< The output directory.
    std::map<OutputResultType, std::string> m_outputFileNames;  //!< The output file names.
    bool m_resizeResults;                                       //!< A flag that indicates if the results must be resized.
    bool m_useNumberOfRegionsSuffix;                            //!< A flag that indicates if the number of regions must be appended to file names.
};

#endif // __MULTISEG_INTERNAL_FILEOUTPUTTER_H
