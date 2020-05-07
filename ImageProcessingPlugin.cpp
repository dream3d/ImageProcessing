/* ============================================================================
 * Copyright (c) 2014 William Lenthe
 * Copyright (c) 2014 DREAM3D Consortium
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of William Lenthe or any of the DREAM3D Consortium contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  This code was partially written under United States Air Force Contract number
 *                              FA8650-10-D-5210
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "ImageProcessingPlugin.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>

#include <itkBMPImageIOFactory.h>
#include <itkJPEGImageIOFactory.h>
#include <itkPNGImageIOFactory.h>
#include <itkTIFFImageIOFactory.h>

#include <QtCore/QTextStream>

#include "SIMPLib/Filtering/FilterFactory.hpp"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/ITK/itkSupportConstants.h"

#include "ImageProcessing/ImageProcessingConstants.h"
#include "ImageProcessing/ImageProcessingVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ImageProcessingPlugin::ImageProcessingPlugin()
: m_Version(ImageProcessing::Version::Package())
, m_CompatibilityVersion(ImageProcessing::Version::Package())
, m_Vendor("Open-Source")
, m_URL("http://www.github.com/dream3d/ImageProcessing")
, m_Location("")
, m_Copyright()
, m_Filters(QList<QString>())
, m_DidLoad(false)
{
  itk::PNGImageIOFactory::RegisterOneFactory();
  itk::TIFFImageIOFactory::RegisterOneFactory();
  itk::JPEGImageIOFactory::RegisterOneFactory();
  itk::BMPImageIOFactory::RegisterOneFactory();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ImageProcessingPlugin::~ImageProcessingPlugin() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getPluginFileName()
{
  return ImageProcessingConstants::ImageProcessingPluginFile;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getPluginDisplayName()
{
  return ImageProcessingConstants::ImageProcessingPluginDisplayName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getPluginBaseName()
{
  return ImageProcessingConstants::ImageProcessingBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getVersion()
{
  return m_Version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getCompatibilityVersion()
{
  return m_CompatibilityVersion;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getVendor()
{
  return m_Vendor;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getURL()
{
  return m_URL;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getLocation()
{
  return m_Location;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getDescription()
{
  QFile licenseFile(":/ImageProcessing/ImageProcessingDescription.txt");
  QFileInfo licenseFileInfo(licenseFile);
  QString text = "<<--Description was not read-->>";

  if(licenseFileInfo.exists())
  {
    if(licenseFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&licenseFile);
      text = in.readAll();
    }
  }
  return text;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getCopyright()
{
  return m_Copyright;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ImageProcessingPlugin::getLicense()
{
  QFile licenseFile(":/ImageProcessing/ImageProcessingLicense.txt");
  QFileInfo licenseFileInfo(licenseFile);
  QString text = "<<--License was not read-->>";

  if(licenseFileInfo.exists())
  {
    if(licenseFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      QTextStream in(&licenseFile);
      text = in.readAll();
    }
  }
  return text;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QMap<QString, QString> ImageProcessingPlugin::getThirdPartyLicenses()
{
  QMap<QString, QString> licenseMap;
  QList<QString> fileStrList;
  fileStrList.push_back(":/ThirdParty/HDF5.txt");

  fileStrList.push_back(":/ThirdParty/Qt.txt");
  fileStrList.push_back(":/ThirdParty/Qwt.txt");

  for(QList<QString>::iterator iter = fileStrList.begin(); iter != fileStrList.end(); iter++)
  {
    QFile file(*iter);
    QFileInfo licenseFileInfo(file);

    if(licenseFileInfo.exists())
    {
      if(file.open(QIODevice::ReadOnly | QIODevice::Text))
      {
        QTextStream in(&file);
        licenseMap.insert(licenseFileInfo.baseName(), in.readAll());
      }
    }
  }

  return licenseMap;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool ImageProcessingPlugin::getDidLoad()
{
  return m_DidLoad;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImageProcessingPlugin::setDidLoad(bool didLoad)
{
  m_DidLoad = didLoad;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImageProcessingPlugin::setLocation(QString filePath)
{
  m_Location = filePath;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImageProcessingPlugin::writeSettings(QSettings& prefs)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImageProcessingPlugin::readSettings(QSettings& prefs)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImageProcessingPlugin::registerFilterWidgets(FilterWidgetManager* fwm)
{
}

#include "ImageProcessingFilters/RegisterKnownFilters.cpp"
