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
  \file Qt4Progress.cpp

  \brief This class implements a progress bar using Qt4 support.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg
#include "Qt4Progress.h"

// Qt
#include <QtCore/QString>
#include <QtGui/QApplication>
#include <QtGui/QCursor>

Qt4Progress::Qt4Progress(QWidget* parent, Qt::WindowFlags flags)
  : QProgressDialog(parent, flags),
    m_numberOfCursors(0)
{
  setMinimumDuration(2000);
}

Qt4Progress::~Qt4Progress()
{
}

void Qt4Progress::reset()
{
  QProgressDialog::reset();

  for(std::size_t i = 0; i < m_numberOfCursors; ++i)
  {
    if(QApplication::overrideCursor())
      QApplication::restoreOverrideCursor();
  }

  m_numberOfCursors = 0;
}

void Qt4Progress::setTotalSteps(int steps)
{
  QProgressDialog::setMaximum(steps);

  if(QApplication::overrideCursor())
  {
    if(QApplication::overrideCursor()->shape() != Qt::WaitCursor)
    {
      QApplication::setOverrideCursor(Qt::WaitCursor);
      m_numberOfCursors++;
    }
  }
  else
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_numberOfCursors++;
  }
}

void Qt4Progress::setMessage(const std::string& text)
{
  QProgressDialog::setLabelText(text.c_str());
}

std::string Qt4Progress::getMessage()
{
  return QProgressDialog::labelText().toLatin1();
}

void Qt4Progress::setProgress(int steps)
{
  QProgressDialog::setValue(steps);
}

bool Qt4Progress::wasCancelled()
{
  return QProgressDialog::wasCanceled();
}

void Qt4Progress::setCaption(const std::string& cap)
{
  QProgressDialog::setWindowTitle(cap.c_str());
}

void Qt4Progress::cancel()
{
  QProgressDialog::cancel();
}

void Qt4Progress::enterEvent(QEvent*)
{
  if(QApplication::overrideCursor())
  {
    if(QApplication::overrideCursor()->shape() != Qt::ArrowCursor)
    {
      QApplication::setOverrideCursor( Qt::ArrowCursor );
      ++m_numberOfCursors;
    }
  }
}

void Qt4Progress::leaveEvent(QEvent*)
{
  if(QApplication::overrideCursor())
  {
    if(QApplication::overrideCursor()->shape() == Qt::ArrowCursor)
    {
      QApplication::restoreOverrideCursor();
      --m_numberOfCursors;
    }
  }
}
