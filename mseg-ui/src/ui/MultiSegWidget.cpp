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
  \file MultiSegWidget.cpp

  \brief This class implements an user-interface to MultiSeg algorithm.

  \author Douglas Uba <douglas@dpi.inpe.br>
*/

// MultiSeg UI
#include "MultiSegWidget.h"
#include "Qt4Progress.h"
#include "ui_MultiSegWidgetForm.h"

// MultiSeg
#include <mseg/FileOutputter.h>
#include <mseg/MultiSeg.h>
#include <mseg/Utils.h>

// TerraLib
#include <terralib/image_processing/TePDIUtils.hpp>

// Qt
#include <QtCore/QFileInfo>
#include <QtCore/QElapsedTimer>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

// STL
#include <algorithm>
#include <map>
#include <vector>

MultiSegWidget::MultiSegWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
    m_ui(new Ui::MultiSegWidgetForm)
{
  m_ui->setupUi(this);

  // Create progress interface support
  Qt4Progress* progress = new Qt4Progress(this);
  progress->setWindowTitle("MultiSeg");
  TeProgress::setProgressInterf(progress);

  initialize();
}

MultiSegWidget::~MultiSegWidget()
{
}

void MultiSegWidget::initialize()
{
  m_imageTypeMap["Radar"] = Radar;
  m_imageTypeMap["Optical"] = Optical;

  m_imageFormatMap["Intensity"] = Intensity;
  m_imageFormatMap["Amplitude"] = Amplitude;

  m_imageModelMap["Cartoon"] = Cartoon;

  m_confidenceLevelMap["100%"] = 1.0;
  m_confidenceLevelMap["99.9%"] = 0.999;
  m_confidenceLevelMap["99.5%"] = 0.995;
  m_confidenceLevelMap["99%"] = 0.99;
  m_confidenceLevelMap["95%"] = 0.95;
  m_confidenceLevelMap["90%"] = 0.90;
  m_confidenceLevelMap["85%"] = 0.85;
  m_confidenceLevelMap["80%"] = 0.80;

  // Setup the user interface initial state (Radar + Cartoon)
  m_ui->m_cvLabel->setVisible(false);
  m_ui->m_cvDoubleSpinBox->setVisible(false);

  // Signals & slots
  connect(m_ui->m_inputImagePushButton, SIGNAL(clicked()), SLOT(onInputImagePushButtonClicked()));
  connect(m_ui->m_outputDirPushButton, SIGNAL(clicked()), SLOT(onOutputDirPushButtonClicked()));
  connect(m_ui->m_imageTypeComboBox, SIGNAL(currentIndexChanged(const QString&)), SLOT(onImageTypeComboBoxCurrentIndexChanged(const QString&)));
  connect(m_ui->m_okPushButton, SIGNAL(clicked()), SLOT(onOkPushButtonClicked()));
  connect(m_ui->m_aboutPushButton, SIGNAL(clicked()), SLOT(onAboutPushButtonClicked()));
  connect(m_ui->m_helpPushButton, SIGNAL(clicked()), SLOT(onHelpPushButtonClicked()));

  adjustSize();
}

void MultiSegWidget::onInputImagePushButtonClicked()
{
  QString path = QFileDialog::getOpenFileName(this, tr("Select the input image file"), "/data/input/", "Image File (*.tif *.tiff *.TIF *.TIFF)");
  
  if(path.isNull())
    return;

  m_ui->m_inputImageLineEdit->setText(path);

  try
  {
    // Gets number of bands
    TePDITypes::TePDIRasterPtrType inputImage(new TeRaster(path.toStdString(), 'r'));
    TEAGN_TRUE_OR_THROW(inputImage->init(), "Unable to init input image.");

    m_ui->m_inputBandsListWidget->clear();

    for(int b = 0; b < inputImage->params().nBands(); ++b)
      m_ui->m_inputBandsListWidget->addItem(QString::number(b));

    // Adjust minimum area values
    m_ui->m_minAreaSpinBox->setMaximum(inputImage->params().nlines_ * inputImage->params().ncols_);
  }
  catch(const TeException& e)
  {
    QString errorMessage = tr("An error has ocurred! Details:");
    errorMessage += e.message().c_str();
    QMessageBox::critical(this, "MultiSeg", errorMessage);
    return;
  }
}

void MultiSegWidget::onOutputDirPushButtonClicked()
{
  QString path = QFileDialog::getExistingDirectory(this, tr("Select the output directory"), "/data/results/");
  
  if(path.isNull())
    return;

  m_ui->m_outputDirLineEdit->setText(path);
}

void MultiSegWidget::onImageTypeComboBoxCurrentIndexChanged(const QString& text)
{
  ImageType type = m_imageTypeMap.find(text).value();
  
  switch(type)
  {
    case Radar:
    {
      m_ui->m_enlLabel->setVisible(true);
      m_ui->m_enlDoubleSpinBox->setVisible(true);

      m_ui->m_radarImageFormatLabel->setVisible(true);
      m_ui->m_radarImageFormatComboBox->setVisible(true);

      m_ui->m_confidenceLevelLabel->setVisible(true);
      m_ui->m_confidenceLevelComboBox->setVisible(true);

      m_ui->m_similarityDoubleSpinBox->setSuffix(" dB");

      m_ui->m_cvLabel->setVisible(false);
      m_ui->m_cvDoubleSpinBox->setVisible(false);
    }
    break;

    case Optical:
    {
      m_ui->m_enlLabel->setVisible(false);
      m_ui->m_enlDoubleSpinBox->setVisible(false);

      m_ui->m_radarImageFormatLabel->setVisible(false);
      m_ui->m_radarImageFormatComboBox->setVisible(false);

      m_ui->m_confidenceLevelLabel->setVisible(true);
      m_ui->m_confidenceLevelComboBox->setVisible(true);

      m_ui->m_similarityDoubleSpinBox->setSuffix("");

      m_ui->m_cvLabel->setVisible(true);
      m_ui->m_cvDoubleSpinBox->setVisible(true);
    }
    break;
  }

  adjustSize();
}

void MultiSegWidget::onOkPushButtonClicked()
{
  if(m_ui->m_inputImageLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, "MultiSeg", tr("Select the input image first."));
    onInputImagePushButtonClicked();
    return;
  }

  if(m_ui->m_inputBandsListWidget->selectedItems().empty())
  {
    QMessageBox::information(this, "MultiSeg", tr("Select the input bands first."));
    m_ui->m_inputBandsListWidget->setFocus();
    return;
  }

  if(m_ui->m_outputDirLineEdit->text().isEmpty())
  {
    QMessageBox::information(this, "MultiSeg", tr("Select the output directory first."));
    onOutputDirPushButtonClicked();
    return;
  }

  // The MultiSeg parameters
  TePDIParameters params;

  setCursor(Qt::WaitCursor);

  QElapsedTimer timer;
  qint64 elapsedTime;

  try
  {
    // Gets the TerraLib PDI Parameters
    getPDIParameters(params);

    // The file outputter
    FileOutputter fileOutputter(m_ui->m_resizeIntermediateResultsCheckBox->isChecked());

    // Output directory
    QString ouputDir = m_ui->m_outputDirLineEdit->text();
    fileOutputter.setOutputDir(ouputDir.toStdString());

    // Input image file name
    QFileInfo inputImageFileInfo(m_ui->m_inputImageLineEdit->text());
    fileOutputter.setInputImageFileName(inputImageFileInfo.baseName().toStdString());

    // Generates the output file names
    std::map<OutputResultType, std::string> outputFileNames;
    Utils::GenerateOutputFilesNames(params, m_ui->m_inputImageLineEdit->text().toStdString(), outputFileNames);
    fileOutputter.setOutputFileNames(outputFileNames);

    // MultiSeg Segmenter
    MultiSeg mseg;

    // Adjust options
    mseg.ToggleProgInt(m_ui->m_enableProgressCheckBox->isChecked());
    mseg.outputPyramid(m_ui->m_savePyramidLevelsCheckBox->isChecked());
    mseg.notifyIntermediateResults(m_ui->m_saveIntermediateResultsCheckBox->isChecked());

    // Defines the outputters
    mseg.addOutputter(&fileOutputter);

    // Reseting...
    TEAGN_TRUE_OR_THROW(mseg.Reset(params), "TerraLib PDI Algorithm reset failed.");

    timer.start();

    // Run!
    TEAGN_TRUE_OR_THROW(mseg.Apply(), "TerraLib PDI Algorithm apply error.");

    elapsedTime = timer.elapsed() / 1000.0;
  }
  catch(const TeException& e)
  {
    setCursor(Qt::ArrowCursor);
    QString errorMessage = tr("An error has ocurred! Details:");
    errorMessage += e.message().c_str();
    QMessageBox::critical(this, "MultiSeg", errorMessage);
    return;
  }

  setCursor(Qt::ArrowCursor);
  QMessageBox::information(this, "MultiSeg", tr("Segmentation done! Elapsed time: ") + QString::number(elapsedTime) + tr(" seconds."));
}

void MultiSegWidget::onAboutPushButtonClicked()
{
    std::string msg = "<h1>MultiSeg</h1><br>\
    MultiSeg is free to use while cited the sources.<br>\
    Version: 2.0.1 <br><br> \
    <b>Source reference for this program:</b> <br>\
    UBA, D. M.; DUTRA, L. V.; SOARES, M. D.; COSTA, G. A. O. P. Implementation of a hierarchical segmentation \
    algorithm for radar and optical data using TerraLib. In: SIMPÓSIO BRASILEIRO DE SENSORIAMENTO REMOTO, 17. \
    (SBSR), 2015, João Pessoa. Anais... São José dos Campos: INPE, 2015. p. 4041-4048. Internet. ISBN 978-85-17-0076-8. IBI: \
    <8JMKD3MGP6W34M/3JM4CC8>. Disponível em: http://urlib.net/8JMKD3MGP6W34M/3JM4CC8. <br><br> \
    <b>Theoretical basics of MultiSeg:</b> <br> \
    SOUSA JÚNIOR, M. A.; DUTRA, L. V.; FREITAS, C. C. Desenvolvimento de um Segmentador Incremental Multi-nível (SIM) para imagens ópticas e de radar. \
    In: SIMPÓSIO BRASILEIRO DE SENSORIAMENTO REMOTO, 11. (SBSR)., 2003, Belo Horizonte. Anais... São José dos Campos: INPE, 2003. \
    p. 2293 - 2300. Printed, On-line. ISBN 85-17-00017-X. (INPE-16179-PRE/10782). Disponível em: http://urlib.net/ltid.inpe.br/sbsr/2002/11.17.18.37. <br> \
    <br>\
    Image Processing Division<br> \
    Instituto Nacional de Pesquisas Espaciais - INPE<br> \
    Caixa Postal 515 - 12245-970 - São José dos Campos - SP, Brazil<br> \
    <br>\
    Electrical Engineering Department<br> \
    Pontifícia Universidade Católica do Rio de Janeiro - PUC-Rio<br> \
    Caixa Postal 38097 - 22453-900 - Rio de Janeiro - RJ, Brazil";

  QMessageBox::information(this, tr("About"), msg.c_str());
}

void MultiSegWidget::onHelpPushButtonClicked()
{
  QMessageBox::information(this, "MultiSeg", tr("Sorry, not available yet."));
}

void MultiSegWidget::getPDIParameters(TePDIParameters& params)
{
  // Input Image
  QString inputImagePath = m_ui->m_inputImageLineEdit->text();
  TePDITypes::TePDIRasterPtrType inputImage(new TeRaster(inputImagePath.toStdString(), 'r'));
  TEAGN_TRUE_OR_THROW(inputImage->init(), "Unable to init input image.");
  params.SetParameter("input_image", inputImage);

  // Input bands
  std::vector<std::size_t> bands;
  QList<QListWidgetItem*> selectedBands = m_ui->m_inputBandsListWidget->selectedItems();
  for(int i = 0; i < selectedBands.size(); ++i)
    bands.push_back(static_cast<std::size_t>(m_ui->m_inputBandsListWidget->row(selectedBands.at(i))));

  std::sort(bands.begin(), bands.end());

  params.SetParameter("input_bands", bands);

  // Image Type
  QString typeText = m_ui->m_imageTypeComboBox->currentText();
  ImageType type = m_imageTypeMap.find(typeText).value();
  params.SetParameter("image_type", type);

  // Image Model
  QString modelText = m_ui->m_imageModelComboBox->currentText();
  ImageModelRepresentation model = m_imageModelMap.find(modelText).value();
  params.SetParameter("image_model", model);

  // Radar Image Format
  QString formatText = m_ui->m_radarImageFormatComboBox->currentText();
  ImageFormat format = m_imageFormatMap.find(formatText).value();
  params.SetParameter("image_radar_format", format);

  // Levels
  int levels = m_ui->m_levelsSpinBox->value();
  params.SetParameter("levels", (std::size_t)levels);

  // Similarity
  double similarity = m_ui->m_similarityDoubleSpinBox->value();
  params.SetParameter("similarity", similarity);

  // ENL
  double enl = m_ui->m_enlDoubleSpinBox->value();
  params.SetParameter("ENL", enl);

  // Confidence Level
  QString confidenceLevelText = m_ui->m_confidenceLevelComboBox->currentText();
  double confidenceLevelValue = m_confidenceLevelMap.find(confidenceLevelText).value();
  params.SetParameter("confidence_level", confidenceLevelValue);

  // Minimum Area
  int minArea = m_ui->m_minAreaSpinBox->value();
  params.SetParameter("min_area", (std::size_t)minArea);

  // Coefficient of Variation
  double cv = m_ui->m_cvDoubleSpinBox->value();
  params.SetParameter("cv", cv);
}
