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
#include "ItkReadImage.h"

#include <string>

#include "SIMPLib/Common/TemplateHelpers.h"
#include "SIMPLib/Common/TemplateHelpers.hpp"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/InputFileFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/ITK/itkReadImageImpl.hpp"

enum createdPathID : RenameDataPath::DataID_t
{
  AttributeMatrixID21 = 21,

  DataContainerID = 1
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkReadImage::ItkReadImage()
: m_InputFileName("")
, m_DataContainerName(SIMPL::Defaults::ImageDataContainerName)
, m_CellAttributeMatrixName(SIMPL::Defaults::CellAttributeMatrixName)
, m_ImageDataArrayName(SIMPL::CellData::ImageData)
, m_ImageData(nullptr)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkReadImage::~ItkReadImage() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkReadImage::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SIMPL_NEW_INPUT_FILE_FP("Input File", InputFileName, FilterParameter::Parameter, ItkReadImage, "*.tif *.jpeg *.png *.bmp", "Image"));
  parameters.push_back(SIMPL_NEW_DC_CREATION_FP("Data Container", DataContainerName, FilterParameter::CreatedArray, ItkReadImage));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_STRING_FP("Cell Attribute Matrix", CellAttributeMatrixName, FilterParameter::CreatedArray, ItkReadImage));
  parameters.push_back(SIMPL_NEW_STRING_FP("Image Data", ImageDataArrayName, FilterParameter::CreatedArray, ItkReadImage));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkReadImage::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setInputFileName( reader->readString( "InputFileName", getInputFileName() ) );
  setDataContainerName( reader->readString( "DataContainerName", getDataContainerName() ) );
  setCellAttributeMatrixName( reader->readString( "CellAttributeMatrixName", getCellAttributeMatrixName() ) );
  setImageDataArrayName( reader->readString( "ImageDataArrayName", getImageDataArrayName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkReadImage::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkReadImage::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  //check file name exists
  if(getInputFileName().isEmpty())
  {
    setErrorCondition(-1, "The input file name must be set before executing this filter.");
    return;
  }

  //read image metadata
  itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(getInputFileName().toLocal8Bit().constData(), itk::ImageIOFactory::ReadMode);
  if(nullptr == imageIO)
  {
    QString message = QObject::tr("Unable to read image '%1'").arg(getInputFileName());
    setErrorCondition(-2, message);
    return;
  }
  imageIO->SetFileName(getInputFileName().toLocal8Bit().data());
  imageIO->ReadImageInformation();

  //get size of image
  const size_t numDimensions = imageIO->GetNumberOfDimensions();
  int xdim = imageIO->GetDimensions(0);
  int ydim = imageIO->GetDimensions(1);
  int zdim = 1;
  if(3 != numDimensions)
  {
    if(2 == numDimensions)
    {
      //allow 2 dimensional images (as 3d image with size 1 in the z direction)
    }
    else
    {
      QString message = QObject::tr("3 dimensional image required (slected image dimensions: %1)").arg(numDimensions);
      setErrorCondition(-3, message);
      return;
    }
  }
  else
  {
    zdim = imageIO->GetDimensions(2);
  }

  //determine if container/attribute matrix already exist. if so check size compatibility
  DataArrayPath createdPath;
  DataContainer::Pointer m;
  AttributeMatrix::Pointer cellAttrMat;
  createdPath.update(getDataContainerName(), getCellAttributeMatrixName(), getImageDataArrayName() );

  m = getDataContainerArray()->getDataContainer(getDataContainerName());
  bool createAttributeMatrix = false;

  if(nullptr == m.get()) //datacontainer doesn't exist->create
  {
    m = getDataContainerArray()->createNonPrereqDataContainer<AbstractFilter>(this, getDataContainerName(), DataContainerID);
    ImageGeom::Pointer image = ImageGeom::CreateGeometry(SIMPL::Geometry::ImageGeometry);
    m->setGeometry(image);
    m->getGeometryAs<ImageGeom>()->setDimensions(xdim, ydim, zdim);
    double zRes = 1;
    double zOrigin = 0;
    if(3 == numDimensions)
    {
      zRes = imageIO->GetSpacing(2);
      zOrigin = imageIO->GetOrigin(2);
    }
    m->getGeometryAs<ImageGeom>()->setResolution(imageIO->GetSpacing(0), imageIO->GetSpacing(0), zRes);
    m->getGeometryAs<ImageGeom>()->setOrigin(imageIO->GetOrigin(0), imageIO->GetOrigin(1), zOrigin);
    createAttributeMatrix = true;
    if(getErrorCondition() < 0) { return; }
  }
  else   //datacontainer exists, check if attribute matrix exists
  {
    bool dcExists = m->doesAttributeMatrixExist(getCellAttributeMatrixName());
    ImageGeom::Pointer image = m->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
    if(getErrorCondition() < 0) { return; }

    size_t iDims[3] = { 0, 0, 0 };
    float iRes[3] = { 0.0f, 0.0f, 0.0f };
    float iOrigin[4] = { 0.0f, 0.0f, 0.0f };
    std::tie(iDims[0], iDims[1], iDims[2]) = image->getDimensions();
    image->getResolution(iRes);
    image->getOrigin(iOrigin);

    if(dcExists &&  nullptr != image.get())//attribute matrix exists, check compatibility
    {
      //get matrix
      cellAttrMat = m->getPrereqAttributeMatrix<AbstractFilter>(this, getCellAttributeMatrixName(), false);
      if(getErrorCondition() < 0) { return; }

      //check dimension compatibility
      QVector<size_t> tDims = cellAttrMat->getTupleDimensions();
      if(tDims[0] != xdim || iDims[0] != xdim)
      {
        QString message = QObject::tr("The x size of '%1' (%2) does not match the x size of '%3' (%4)").arg(getInputFileName()).arg(xdim).arg(getDataContainerName() + "/" + getCellAttributeMatrixName()).arg(tDims[0]);
        setErrorCondition(-4, message);
        return;
      }
      if(tDims[1] != ydim || iDims[1] != ydim)
      {
        QString message = QObject::tr("The y size of '%1' (%2) does not match the x size of '%3' (%4)").arg(getInputFileName()).arg(ydim).arg(getDataContainerName() + "/" + getCellAttributeMatrixName()).arg(tDims[1]);
        setErrorCondition(-5, message);
        return;
      }
      if(3 == numDimensions)
      {
        if(tDims[2] != zdim || iDims[2] != zdim)
        {
          QString message = QObject::tr("The z size of '%1' (%2) does not match the x size of '%3' (%4)").arg(getInputFileName()).arg(zdim).arg(getDataContainerName() + "/" + getCellAttributeMatrixName()).arg(tDims[2]);
          setErrorCondition(-6, message);
          return;
        }
      }
      else
      {
        if(tDims[2] != 1 || iDims[2] != 1)
        {
          QString message = QObject::tr("The z size of '%1' (%2) does not match the x size of '%3' (1)").arg(getInputFileName()).arg(zdim).arg(getDataContainerName() + "/" + getCellAttributeMatrixName());
          setErrorCondition(-7, message);
          return;
        }
      }
    }
    else //attribute matrix doesn't exist, create
    {
      createAttributeMatrix = true;
    }
  }

  //image/attribute matrix dimensions
  QVector<size_t> tDims(3, 0);
  tDims[0] = xdim;
  tDims[1] = ydim;
  tDims[2] = zdim;

  //create attribute matrix if needed
  if(createAttributeMatrix)
  {
    //create attribute matrix
    cellAttrMat = m->createNonPrereqAttributeMatrix(this, getCellAttributeMatrixName(), tDims, AttributeMatrix::Type::Cell, AttributeMatrixID21);
    if(getErrorCondition() < 0) { return; }
  }

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
      setErrorCondition(-90001, "The Pixel Type of the image is not supported with DREAM3D.");
  }

  // Check to make sure everything is OK with reading the image
  if(getErrorCondition() < 0)
  {
    std::string pixelTypeName = itk::ImageIOBase::GetPixelTypeAsString(pixelType);
    QString message = QObject::tr("The pixel type of '%1' (%2) is unsupported").arg(getInputFileName()).arg(QString::fromStdString(pixelTypeName));
    setErrorCondition(getErrorCondition(), message);
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
    QString message = QObject::tr("The component type type of '%1' (%2) is unsupported").arg(getInputFileName()).arg(QString::fromStdString(componentTypeName));
    setErrorCondition(-9, message);
    return;
  }

  if(getErrorCondition() < 0) { return; }

  m_ImageDataPtr = TemplateHelpers::CreateNonPrereqArrayFromArrayType()(this, createdPath, componentDims, data);
  if(nullptr != m_ImageDataPtr.lock())
  {
    m_ImageData = m_ImageDataPtr.lock()->getVoidPointer(0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkReadImage::preflight()
{
  // These are the REQUIRED lines of CODE to make sure the filter behaves correctly
  setInPreflight(true); // Set the fact that we are preflighting.
  emit preflightAboutToExecute(); // Emit this signal so that other widgets can do one file update
  emit updateFilterParameters(this); // Emit this signal to have the widgets push their values down to the filter
  dataCheck(); // Run our DataCheck to make sure everthing is setup correctly
  emit preflightExecuted(); // We are done preflighting this filter
  setInPreflight(false); // Inform the system this filter is NOT in preflight mode anymore.
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkReadImage::execute()
{
  QString ss;
  dataCheck();
  if(getErrorCondition() < 0)
  {
    ss = QObject::tr("DataCheck did not pass during execute");
    setErrorCondition(-11000, ss);
    return;
  }

  //get volume container
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());

  //get input and output data
  IDataArray::Pointer imageData = m_ImageDataPtr.lock();
  //std::string fileNameString = getInputFileName().toLocal8Bit().constData();
  //const char* fileNameCStr = fileNameString.c_str();


  // execute type dependant portion using a Private Implementation that takes care of figuring out if
  // we can work on the correct type and actually handling the algorithm execution. We pass in "this" so
  // that the private implementation can get access to the current object to pass up status notifications,
  // progress or handle "cancel" if needed.

  if(ItkReadImagePrivate<double, AbstractFilter>()(imageData) )
  {
    ItkReadImagePrivate<double, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else if (ItkReadImagePrivate<float, AbstractFilter>()(imageData))
  {
      ItkReadImagePrivate<float, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else if (ItkReadImagePrivate<uint64_t, AbstractFilter>()(imageData))
  {
      ItkReadImagePrivate<uint64_t, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else if (ItkReadImagePrivate<int64_t, AbstractFilter>()(imageData))
  {
      ItkReadImagePrivate<int64_t, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else if (ItkReadImagePrivate<uint32_t, AbstractFilter>()(imageData))
  {
      ItkReadImagePrivate<uint32_t, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else if (ItkReadImagePrivate<int32_t, AbstractFilter>()(imageData))
  {
      ItkReadImagePrivate<int32_t, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else if (ItkReadImagePrivate<uint16_t, AbstractFilter>()(imageData))
  {
      ItkReadImagePrivate<uint16_t, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else if (ItkReadImagePrivate<int16_t, AbstractFilter>()(imageData))
  {
      ItkReadImagePrivate<int16_t, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else if (ItkReadImagePrivate<uint8_t, AbstractFilter>()(imageData))
  {
      ItkReadImagePrivate<uint8_t, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else if (ItkReadImagePrivate<int8_t, AbstractFilter>()(imageData))
  {
      ItkReadImagePrivate<int8_t, AbstractFilter>::Execute(this, getInputFileName(), imageData);
  }
  else
  {
    ss = QObject::tr("A Supported DataArray type was not used for an input array.");
    setErrorCondition(-10001, ss);
    return;
  }

  AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(getCellAttributeMatrixName());
  attrMat->insertOrAssign(imageData);

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage("Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkReadImage::newFilterInstance(bool copyFilterParameters) const
{
  ItkReadImage::Pointer filter = ItkReadImage::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkReadImage::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkReadImage::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QUuid ItkReadImage::getUuid()
{
  return QUuid("{b54461ff-13ef-5f98-9311-e0e5bcbef1fd}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkReadImage::getSubGroupName() const
{return "IO";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkReadImage::getHumanLabel() const
{ return "Import Image (ImageProcessing)"; }

