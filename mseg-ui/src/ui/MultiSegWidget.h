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
  \file MultiSegWidget.h

  \brief This class implements an user-interface to MultiSeg algorithm.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEGUI_INTERNAL_MULTISEGWIDGET_H
#define __MULTISEGUI_INTERNAL_MULTISEGWIDGET_H

// MultiSeg
#include <mseg/Enums.h>

// TerraLib
#include <terralib/image_processing/TePDIParameters.hpp>

// Qt
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtGui/QWidget>

// Forward declaraion
namespace Ui { class MultiSegWidgetForm; }

/*!
  \class MultiSegWidget

  \brief This class implements an user-interface to MultiSeg algorithm.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/
class MultiSegWidget : public QWidget
{
  Q_OBJECT

  public:

    /*!
      \brief Constructor.

      \param parent The widget parent.
      \param flags  The widget flags.
    */
    MultiSegWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

    /*! \brief Destructor. */
    ~MultiSegWidget();

  private:

    /*! \brief Initializes the MultiSeg user-interface. */
    void initialize();

  private slots:

    /** @name Protected widget slots.
      * Callback methods related with user actions. */
    //@{

    void onInputImagePushButtonClicked();

    void onOutputDirPushButtonClicked();

    void onImageTypeComboBoxCurrentIndexChanged(const QString& text);

    void onOkPushButtonClicked();

    void onAboutPushButtonClicked();

    void onHelpPushButtonClicked();

    //@}

  private:

    /*!
      \brief Internal method that fills the given MultiSeg algorithm parameters.

      \param params The MultiSeg algorithm parameters that will be filled.
    */
    void getPDIParameters(TePDIParameters& params);

  private:

    Ui::MultiSegWidgetForm* m_ui;                             //!< The widget form.
    QMap<QString, ImageType> m_imageTypeMap;                  //!< Map the user interface string to ImageType (Radar || Optical).
    QMap<QString, ImageFormat> m_imageFormatMap;              //!< Map the user interface string to ImageFormat. (Amplitude || Intensity || dB)
    QMap<QString, ImageModelRepresentation> m_imageModelMap;  //!< Map the user interface string to ImageModelRepresentation. (Cartoon || Texture)
    QMap<QString, double> m_confidenceLevelMap;               //!< Map the user interface string to confidence level. (e.g. 99.9%, 99%, 95%, etc.)
};

#endif  // __MULTISEGUI_INTERNAL_MULTISEGWIDGET_H
