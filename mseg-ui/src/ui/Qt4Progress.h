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
  \file Qt4Progress.h

  \brief This class implements a progress bar using Qt4 support.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEGUI_INTERNAL_QT4PROGRESS_H
#define __MULTISEGUI_INTERNAL_QT4PROGRESS_H

// TerraLib
#include <terralib/kernel/TeDefines.h>
#include <terralib/kernel/TeProgress.h>

// Qt
#include <QtGui/QProgressDialog>

// STL
#include <string>

/*!
  \class Qt4Progress

  \brief This class implements a progress bar using Qt4 support.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/
class Qt4Progress : public QProgressDialog, public TeProgressBase
{
  public:

    /*!
      \brief Constructor.

      \param parent The progress bar parent.
      \param flags  The widget flags.
    */
    Qt4Progress(QWidget* parent = 0, Qt::WindowFlags flags = 0);

    /*! \brief Destructor. */
    ~Qt4Progress();

    /* \brief This method resets the progress bar. */
    void reset();

    /*! \brief This method sets the total number of steps to n. */
    void setTotalSteps(int steps);

    /*! \brief This method sets the progress bar message. */
    void setMessage(const std::string& text);

    /*! \brief This method gets the progress bar message. */
    std::string getMessage();

    /* \brief This method sets the current amount of progress made to n. */
    void setProgress(int steps);

    /* \brief  This method returns true whether the process was cancelled. */
    bool wasCancelled();

    /*! \brief This method sets the progress bar caption. */
    void setCaption(const std::string& cap);

    /* \brief This method cancels the progress bar. */
    void cancel();

  protected:

    void enterEvent(QEvent* e);

    void leaveEvent(QEvent* e);

  private:

    std::size_t m_numberOfCursors;
};

#endif // __MULTISEGUI_INTERNAL_QT4PROGRESS_H
