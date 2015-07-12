/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "ItkImportImageStack.h"

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Common/TemplateHelpers.hpp"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersWriter.h"

#include "DREAM3DLib/FilterParameters/FloatVec3FilterParameter.h"
#include "DREAM3DLib/FilterParameters/StringFilterParameter.h"
#include "DREAM3DLib/FilterParameters/FileListInfoFilterParameter.h"
#include "DREAM3DLib/FilterParameters/SeparatorFilterParameter.h"
#include "DREAM3DLib/Utilities/FilePathGenerator.h"

#include "ImageProcessing/ImageProcessingConstants.h"
#include "ImageProcessing/ImageProcessingFilters/ItkReadImageImpl.hpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkImportImageStack::ItkImportImageStack() :
  AbstractFilter(),
  m_DataContainerName(DREAM3D::Defaults::ImageDataContainerName),
  m_CellAttributeMatrixName(DREAM3D::Defaults::CellAttributeMatrixName),
  m_ImageDataArrayName(DREAM3D::CellData::ImageData)
{
  m_Origin.x = 0.0f;
  m_Origin.y = 0.0f;
  m_Origin.z = 0.0f;

  m_Resolution.x = 1.0f;
  m_Resolution.y = 1.0f;
  m_Resolution.z = 1.0f;

  m_InputFileListInfo.FileExtension = QString("tif");
  m_InputFileListInfo.StartIndex = 0;
  m_InputFileListInfo.EndIndex = 0;
  m_InputFileListInfo.PaddingDigits = 0;

  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkImportImageStack::~ItkImportImageStack()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImportImageStack::setupFilterParameters()
{
  QVector<FilterParameter::Pointer> parameters;
  parameters.push_back(FileListInfoFilterParameter::New("Input File List", "InputFileListInfo", getInputFileListInfo(), FilterParameter::Parameter));
  parameters.push_back(FloatVec3FilterParameter::New("Origin", "Origin", getOrigin(), FilterParameter::Parameter));
  parameters.push_back(FloatVec3FilterParameter::New("Resolution", "Resolution", getResolution(), FilterParameter::Parameter));
  parameters.push_back(StringFilterParameter::New("Data Container", "DataContainerName", getDataContainerName(), FilterParameter::CreatedArray));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(StringFilterParameter::New("Cell Attribute Matrix", "CellAttributeMatrixName", getCellAttributeMatrixName(), FilterParameter::CreatedArray));
  parameters.push_back(StringFilterParameter::New("Image Data", "ImageDataArrayName", getImageDataArrayName(), FilterParameter::CreatedArray));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImportImageStack::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setDataContainerName(reader->readString("DataContainerName", getDataContainerName() ) );
  setCellAttributeMatrixName(reader->readString("CellAttributeMatrixName", getCellAttributeMatrixName() ) );
  setImageDataArrayName(reader->readString("ImageDataArrayName", getImageDataArrayName() ) );
  setInputFileListInfo( reader->readFileListInfo("InputFileListInfo", getInputFileListInfo() ) );
  setOrigin( reader->readFloatVec3("Origin", getOrigin()) );
  setResolution( reader->readFloatVec3("Resolution", getResolution()) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ItkImportImageStack::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(FilterVersion)
      DREAM3D_FILTER_WRITE_PARAMETER(DataContainerName)
      DREAM3D_FILTER_WRITE_PARAMETER(CellAttributeMatrixName)
      DREAM3D_FILTER_WRITE_PARAMETER(ImageDataArrayName)
      DREAM3D_FILTER_WRITE_PARAMETER(InputFileListInfo)
      DREAM3D_FILTER_WRITE_PARAMETER(Origin)
      DREAM3D_FILTER_WRITE_PARAMETER(Resolution)
      writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImportImageStack::dataCheck()
{
  setErrorCondition(0);

  DataArrayPath tempPath;
  QString ss;

  if (m_InputFileListInfo.InputPath.isEmpty() == true)
  {
    ss = QObject::tr("The input directory must be set");
    notifyErrorMessage(getHumanLabel(), ss, -13);
    setErrorCondition(-13);
  }

  DataContainer::Pointer m = getDataContainerArray()->createNonPrereqDataContainer<AbstractFilter>(this, getDataContainerName());
  if(getErrorCondition() < 0 || NULL == m.get()) { return; }

  ImageGeom::Pointer image = ImageGeom::CreateGeometry(DREAM3D::Geometry::ImageGeometry);
  m->setGeometry(image);

  bool hasMissingFiles = false;
  bool orderAscending = false;

  if (m_InputFileListInfo.Ordering == 0) { orderAscending = true; }
  else if (m_InputFileListInfo.Ordering == 1) { orderAscending = false; }

  // Now generate all the file names the user is asking for and populate the table
  QVector<QString> fileList = FilePathGenerator::GenerateFileList(m_InputFileListInfo.StartIndex,
                                                                  m_InputFileListInfo.EndIndex, hasMissingFiles, orderAscending,
                                                                  m_InputFileListInfo.InputPath, m_InputFileListInfo.FilePrefix,
                                                                  m_InputFileListInfo.FileSuffix, m_InputFileListInfo.FileExtension,
                                                                  m_InputFileListInfo.PaddingDigits);
  if (fileList.size() == 0)
  {
    QString ss = QObject::tr("No files have been selected for import");
    setErrorCondition(-11);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }
  else
  {
    //read image metadata
    itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(fileList.at(0).toLocal8Bit().constData(), itk::ImageIOFactory::ReadMode);
    if(NULL == imageIO)
    {
      setErrorCondition(-2);
      QString message = QObject::tr("Unable to read image '%1'").arg(fileList.at(0));
      notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
      return;
    }
    imageIO->SetFileName(fileList.at(0).toLocal8Bit().data());
    imageIO->ReadImageInformation();

    //get size of image
    const size_t numDimensions = imageIO->GetNumberOfDimensions();
    int xdim = imageIO->GetDimensions(0);
    int ydim = imageIO->GetDimensions(1);
    int zdim = fileList.size(); // the z Dimension is the number of images in the list
    if(3 != numDimensions)
    {
      if(2 == numDimensions)
      {
        //allow 2 dimensional images (as 3d image with size 1 in the z direction)
      }
      else
      {
        QString message = QObject::tr("3 dimensional image required (slected image dimensions: %1)").arg(numDimensions);
        setErrorCondition(-3);
        notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
        return;
      }
    }

    DataArrayPath createdPath;
    createdPath.update(getDataContainerName(), getCellAttributeMatrixName(), getImageDataArrayName() );

    image->setDimensions(static_cast<size_t>(xdim), static_cast<size_t>(ydim), static_cast<size_t>(zdim));
    image->setResolution(m_Resolution.x, m_Resolution.y, m_Resolution.z);
    image->setOrigin(m_Origin.x, m_Origin.y, m_Origin.z);

    QVector<size_t> tDims(3, 0);
    tDims[0] = xdim;
    tDims[1] = ydim;
    tDims[2] = zdim;

    m->createNonPrereqAttributeMatrix<AbstractFilter>(this, getCellAttributeMatrixName(), tDims, DREAM3D::AttributeMatrixType::Cell);

    //check pixel type (scalar, vector, etc) for support
    QVector<size_t> componentDims(1, 0);
    itk::ImageIOBase::IOPixelType pixelType = imageIO->GetPixelType();

    switch(pixelType)
    {

    case itk::ImageIOBase::SCALAR:
      componentDims[0] = 1;
      break;
    case itk::ImageIOBase::RGB:
      componentDims[0] = 3;
      break;
    case itk::ImageIOBase::RGBA:
      componentDims[0] = 4;
      break;
    default:
      setErrorCondition(-80001);
      notifyErrorMessage(getHumanLabel(), "The Pixel Type of the image is not supported with DREAM3D.", getErrorCondition());
    }

    // Check to make sure everything is OK with reading the image
    if(getErrorCondition() < 0)
    {
      std::string pixelTypeName = itk::ImageIOBase::GetPixelTypeAsString(pixelType);
      QString message = QObject::tr("The pixel type of '%1' (%2) is unsupported").arg(fileList.at(0)).arg(QString::fromStdString(pixelTypeName));
      notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
      return;
    }

    //Now get how the actual image data is stored.
    IDataArray::Pointer data;
    itk::ImageIOBase::IOComponentType componentType = imageIO->GetComponentType();
    if(itk::ImageIOBase::CHAR == componentType)
    {
      data = Int8ArrayType::CreateArray(0, "Temp", false);
    }
    else if(itk::ImageIOBase::UCHAR == componentType)
    {
      data = UInt8ArrayType::CreateArray(0, "Temp", false);
    }
    else if(itk::ImageIOBase::SHORT == componentType)
    {
      data = Int16ArrayType::CreateArray(0, "Temp", false);
    }
    else if(itk::ImageIOBase::USHORT == componentType)
    {
      data = UInt16ArrayType::CreateArray(0, "Temp", false);
    }
    else if(itk::ImageIOBase::INT == componentType)
    {
      data = Int32ArrayType::CreateArray(0, "Temp", false);
    }
    else if(itk::ImageIOBase::UINT == componentType)
    {
      data = UInt32ArrayType::CreateArray(0, "Temp", false);
    }
    else if(itk::ImageIOBase::LONG == componentType)
    {
      data = Int64ArrayType::CreateArray(0, "Temp", false);
    }
    else if(itk::ImageIOBase::ULONG == componentType)
    {
      data = UInt64ArrayType::CreateArray(0, "Temp", false);
    }
    else if(itk::ImageIOBase::FLOAT == componentType)
    {
      data = FloatArrayType::CreateArray(0, "Temp", false);
    }
    else if(itk::ImageIOBase::DOUBLE == componentType)
    {
      data = DoubleArrayType::CreateArray(0, "Temp", false);
    }
    else
    {
      std::string componentTypeName = itk::ImageIOBase::GetComponentTypeAsString(componentType);
      QString message = QObject::tr("The component type type of '%1' (%2) is unsupported").arg(fileList.at(0)).arg(QString::fromStdString(componentTypeName));
      setErrorCondition(-9);
      notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
      return;
    }

    if(getErrorCondition() < 0) { return; }

    m_ImageDataPtr = TemplateHelpers::CreateNonPrereqArrayFromArrayType()(this, createdPath, componentDims, data);
    if( NULL != m_ImageDataPtr.lock().get() )
    {
      m_ImageData = m_ImageDataPtr.lock()->getVoidPointer(0);
    }

  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImportImageStack::preflight()
{
  setInPreflight(true);
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImportImageStack::execute()
{
  QString ss;
  dataCheck();
  if(getErrorCondition() < 0)
  {
    setErrorCondition(-10000);
    ss = QObject::tr("DataCheck did not pass during execute");
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

#if 1
  setErrorCondition(-666);
  ss = QObject::tr("This filter does not currently work correctly.");
  notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  return;
#endif

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());

  m->getGeometryAs<ImageGeom>()->setResolution(m_Resolution.x, m_Resolution.y, m_Resolution.z);
  m->getGeometryAs<ImageGeom>()->setOrigin(m_Origin.x, m_Origin.y, m_Origin.z);

  int64_t z = m_InputFileListInfo.StartIndex;
  int64_t zSpot;

  bool hasMissingFiles = false;
  bool orderAscending = false;

  if (m_InputFileListInfo.Ordering == 0) { orderAscending = true; }
  else if (m_InputFileListInfo.Ordering == 1) { orderAscending = false; }

  // Now generate all the file names the user is asking for and populate the table
  QVector<QString> fileList = FilePathGenerator::GenerateFileList(m_InputFileListInfo.StartIndex,
                                                                  m_InputFileListInfo.EndIndex, hasMissingFiles, orderAscending,
                                                                  m_InputFileListInfo.InputPath, m_InputFileListInfo.FilePrefix,
                                                                  m_InputFileListInfo.FileSuffix, m_InputFileListInfo.FileExtension,
                                                                  m_InputFileListInfo.PaddingDigits);
  if (fileList.size() == 0)
  {
    QString ss = QObject::tr("No files have been selected for import");
    setErrorCondition(-11);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  for (QVector<QString>::iterator filepath = fileList.begin(); filepath != fileList.end(); ++filepath)
  {
    QString imageFName = *filepath;
    QString ss = QObject::tr("Importing file %1").arg(imageFName);
    notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);

    //get input and output data
    IDataArray::Pointer imageData = m_ImageDataPtr.lock();

    //execute type dependant portion using a Private Implementation that takes care of figuring out if
    // we can work on the correct type and actually handling the algorithm execution. We pass in "this" so
    // that the private implementation can get access to the current object to pass up status notifications,
    // progress or handle "cancel" if needed.
    if(ItkReadImagePrivate<int8_t, AbstractFilter>()(imageData))
    {
      ItkReadImagePrivate<int8_t, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else if(ItkReadImagePrivate<uint8_t, AbstractFilter>()(imageData) )
    {
      ItkReadImagePrivate<uint8_t, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else if(ItkReadImagePrivate<int16_t, AbstractFilter>()(imageData) )
    {
      ItkReadImagePrivate<int16_t, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else if(ItkReadImagePrivate<uint16_t, AbstractFilter>()(imageData) )
    {
      ItkReadImagePrivate<uint16_t, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else if(ItkReadImagePrivate<int32_t, AbstractFilter>()(imageData) )
    {
      ItkReadImagePrivate<int32_t, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else if(ItkReadImagePrivate<uint32_t, AbstractFilter>()(imageData) )
    {
      ItkReadImagePrivate<uint32_t, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else if(ItkReadImagePrivate<int64_t, AbstractFilter>()(imageData) )
    {
      ItkReadImagePrivate<int64_t, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else if(ItkReadImagePrivate<uint64_t, AbstractFilter>()(imageData) )
    {
      ItkReadImagePrivate<uint64_t, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else if(ItkReadImagePrivate<float, AbstractFilter>()(imageData) )
    {
      ItkReadImagePrivate<float, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else if(ItkReadImagePrivate<double, AbstractFilter>()(imageData) )
    {
      ItkReadImagePrivate<double, AbstractFilter>::Execute(this, imageFName, imageData);
    }
    else
    {
      setErrorCondition(-10001);
      ss = QObject::tr("A Supported DataArray type was not used for an input array.");
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
      return;
    }



  }


  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getMessagePrefix(), getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkImportImageStack::newFilterInstance(bool copyFilterParameters)
{
  ItkImportImageStack::Pointer filter = ItkImportImageStack::New();
  if(true == copyFilterParameters)
  {
    filter->setFilterParameters(getFilterParameters() );
    // We are going to hand copy all of the parameters because the other way of copying the parameters are going to
    // miss some of them because we are not enumerating all of them.
    DREAM3D_COPY_INSTANCEVAR(DataContainerName)
        DREAM3D_COPY_INSTANCEVAR(CellAttributeMatrixName)
        DREAM3D_COPY_INSTANCEVAR(Resolution)
        DREAM3D_COPY_INSTANCEVAR(Origin)
    #if 0
        DREAM3D_COPY_INSTANCEVAR(ZStartIndex)
        DREAM3D_COPY_INSTANCEVAR(ZEndIndex)
        DREAM3D_COPY_INSTANCEVAR(InputPath)
        DREAM3D_COPY_INSTANCEVAR(FilePrefix)
        DREAM3D_COPY_INSTANCEVAR(FileSuffix)
        DREAM3D_COPY_INSTANCEVAR(FileExtension)
        DREAM3D_COPY_INSTANCEVAR(PaddingDigits)
        DREAM3D_COPY_INSTANCEVAR(RefFrameZDir)
    #endif
        DREAM3D_COPY_INSTANCEVAR(InputFileListInfo)
        DREAM3D_COPY_INSTANCEVAR(ImageStack)
        DREAM3D_COPY_INSTANCEVAR(ImageDataArrayName)
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkImportImageStack::getCompiledLibraryName()
{ return ImageProcessingConstants::ImageProcessingBaseName; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkImportImageStack::getGroupName()
{ return DREAM3D::FilterGroups::IOFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkImportImageStack::getSubGroupName()
{ return DREAM3D::FilterSubGroups::InputFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkImportImageStack::getHumanLabel()
{ return "ITK - Import Images (3D Stack)"; }
