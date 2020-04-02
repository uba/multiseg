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
  \file Enums.h

  \brief Enumeration values related with MultiSeg.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

#ifndef __MULTISEG_INTERNAL_ENUMS_H
#define __MULTISEG_INTERNAL_ENUMS_H

/** @name Enumeration
  * Enumeration values related with MultiSeg.
*/
//@{

  /*!
    \enum ImageModelRepresentation
    \brief Defines the image model representation that will be used on segmentation process.
  */
  enum ImageModelRepresentation
  {
    Cartoon, /*!< Cartoon Model. */
    Texture  /*!< Texture Model. */
  };

  /*!
    \enum ImageType
    \brief Defines the type of the image that that will be segmented.
  */
  enum ImageType
  {
    Radar,   /*!< Radar Image .  */
    Optical  /*!< Optical Image. */
  };

  /*!
    \enum ImageFormat
    \brief Defines the type of the radar image that that will be segmented.
  */
  enum ImageFormat
  {
    Amplitude, /*!< Image Amplitude .     */
    Intensity, /*!< Image Intensity.      */
    dB         /*!< Image Decibels (dB) . */
  };

  /*!
    \enum BorderPixelType
    \brief Defines the type of a border pixel.
  */
  enum BorderPixelType
  {
    Top,    /*!< Top border pixel.    */
    Bottom, /*!< Bottom border pixel. */
    Right,  /*!< Right border pixel   */
    Left    /*!< Left border pixel    */
  };

  /*!
    \enum OutputResultType
    \brief Defines the output result types.
  */
  enum OutputResultType
  {
    LabelledImage, /*!< The labelled image.                               */
    CartoonImage,  /*!< The cartoon image. i.e. the mean image.           */
    Vector,        /*!< The vector that represents the region boundaries. */
  };

//@}

#endif // __MULTISEG_INTERNAL_ENUMS_H
