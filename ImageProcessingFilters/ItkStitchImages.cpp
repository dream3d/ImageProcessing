/* ============================================================================
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
 * Neither the names of any of the DREAM3D Consortium contributors
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
#include <memory>

#include "ItkStitchImages.h"

#include "SIMPLib/Common/SIMPLArray.hpp"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/AttributeMatrixSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/DataContainerCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/ITK/itkBridge.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

#include "itkMaskedFFTNormalizedCorrelationImageFilter.h"

#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkPasteImageFilter.h"

#include "ImageProcessing/ImageProcessingConstants.h"
#include "ImageProcessing/ImageProcessingHelpers.hpp"

enum createdPathID : RenameDataPath::DataID_t
{
  AttributeMatrixID21 = 21,

  DataArrayID31 = 31,

  DataContainerID = 1
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkStitchImages::ItkStitchImages()
: m_AttributeMatrixName(SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellFeatureAttributeMatrixName, "")
, m_StitchedCoordinatesArrayPath(SIMPL::Defaults::ImageDataContainerName, "", "")
, m_StitchedVolumeDataContainerName("MontagedImageDataContainer")
, m_StitchedImagesArrayName("Montage")
, m_StitchedAttributeMatrixName("MontageAttributeMatrix")
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkStitchImages::~ItkStitchImages() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkStitchImages::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));

  {
    AttributeMatrixSelectionFilterParameter::RequirementType req;
    parameters.push_back(SIMPL_NEW_AM_SELECTION_FP("Image Tile Attribute Matrix", AttributeMatrixName, FilterParameter::RequiredArray, ItkStitchImages, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req;
    req.daTypes = QVector<QString>(1, SIMPL::TypeNames::Float);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Image Tile Origins", StitchedCoordinatesArrayPath, FilterParameter::RequiredArray, ItkStitchImages, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req;
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Stitched Coordinates Names", AttributeArrayNamesPath, FilterParameter::RequiredArray, ItkStitchImages, req));
  }

  parameters.push_back(SIMPL_NEW_DC_CREATION_FP("Stitched Image Data Container", StitchedVolumeDataContainerName, FilterParameter::CreatedArray, ItkStitchImages));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Montage Attribute Matrix", StitchedAttributeMatrixName, StitchedVolumeDataContainerName, FilterParameter::CreatedArray, ItkStitchImages));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Montage", StitchedImagesArrayName, StitchedVolumeDataContainerName, StitchedAttributeMatrixName, FilterParameter::CreatedArray, ItkStitchImages));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkStitchImages::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setStitchedVolumeDataContainerName(DataArrayPath(reader->readString("StitchedVolumeDataContainerName", getStitchedVolumeDataContainerName().getDataContainerName() ), "", "") );
  setAttributeMatrixName(reader->readDataArrayPath("AttributeMatrixName", getAttributeMatrixName()));
  setStitchedCoordinatesArrayPath(reader->readDataArrayPath("StitchedCoordinatesArrayPath", getStitchedCoordinatesArrayPath()));
  setStitchedImagesArrayName(reader->readString("StitchedImagesArrayName", getStitchedImagesArrayName()));
  setStitchedAttributeMatrixName(reader->readString("StitchedAttributeMatrixName", getStitchedAttributeMatrixName()));
  setAttributeArrayNamesPath(reader->readDataArrayPath("AttributeArrayNamesPath", getAttributeArrayNamesPath()));
  reader->closeFilterGroup();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkStitchImages::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkStitchImages::dataCheck()
{
  clearErrorCode();
  clearWarningCode();

  DataArrayPath tempPath;

  AttributeMatrix::Pointer am = getDataContainerArray()->getAttributeMatrix(m_AttributeMatrixName);

  if (am.get() == nullptr)
  {
    setErrorCondition(-76000, "The attribute matrix has not been selected properly");
    return;
  }

  QList<QString> names = am->getAttributeArrayNames();

  std::vector<size_t> dims(1, 1);

  UInt8ArrayType::Pointer imagePtr = UInt8ArrayType::NullPointer();
  IDataArray::Pointer iDataArray = IDataArray::NullPointer();

  for(int i = 0; i < names.size(); i++)
  {
    tempPath.update(getAttributeMatrixName().getDataContainerName(), getAttributeMatrixName().getAttributeMatrixName(), names[i]);
    iDataArray = getDataContainerArray()->getPrereqArrayFromPath<UInt8ArrayType, AbstractFilter>(this, tempPath, dims);

    imagePtr = std::dynamic_pointer_cast<UInt8ArrayType>(iDataArray);
    if(nullptr == imagePtr)
    {
      setErrorCondition(-76001, "The data was not found");
    }
  }


  m_AttributeArrayNamesPtr = getDataContainerArray()->getPrereqArrayFromPath<StringDataArray, AbstractFilter>(this, getAttributeArrayNamesPath(), dims);
  if(nullptr != m_StitchedCoordinatesPtr.lock())
  {
    if(names.size() != m_StitchedCoordinatesPtr.lock()->getNumberOfTuples() )
    {
      QString ss = QObject::tr("The number of Attribute Array Names (%1) does not match the number of Images (%2)").arg(m_StitchedCoordinatesPtr.lock()->getNumberOfTuples()).arg(names.size());
      setErrorCondition(-76002, ss);
    }
  }

  dims[0] = 2;
  m_StitchedCoordinatesPtr = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType, AbstractFilter>(this, getStitchedCoordinatesArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if(nullptr != m_StitchedCoordinatesPtr.lock())                              /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_StitchedCoordinates = m_StitchedCoordinatesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  DataContainer::Pointer m = getDataContainerArray()->getPrereqDataContainer(this, getStitchedCoordinatesArrayPath().getDataContainerName(), false);

  if(getErrorCode() < 0 || nullptr == m) { return; }

  DataContainer::Pointer m2 = getDataContainerArray()->createNonPrereqDataContainer<AbstractFilter>(this, getStitchedVolumeDataContainerName(), DataContainerID);
  if(getErrorCode() < 0) { return; }

  ImageGeom::Pointer image = ImageGeom::CreateGeometry(SIMPL::Geometry::ImageGeometry);
  m2->setGeometry(image);

  // Keep Spacing the same as original images
  FloatVec3Type spacing = m->getGeometryAs<ImageGeom>()->getSpacing();

  m2->getGeometryAs<ImageGeom>()->setSpacing(spacing);
  //Set origin to zero
  m2->getGeometryAs<ImageGeom>()->setOrigin(FloatVec3Type());

  m2->getGeometryAs<ImageGeom>()->setDimensions(1, 1, 1);

  std::vector<size_t> tDims(1, 0);

  AttributeMatrix::Pointer stitchedAttMat = m2->createNonPrereqAttributeMatrix(this, getStitchedAttributeMatrixName(), tDims, AttributeMatrix::Type::Cell, AttributeMatrixID21);
  if(getErrorCode() < 0) { return; }
  dims[0] = 1;


  tempPath.update(getStitchedVolumeDataContainerName().getDataContainerName(), getStitchedAttributeMatrixName(), getStitchedImagesArrayName() );
  m_StitchedImageArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter, ImageProcessingConstants::DefaultPixelType>(
      this, tempPath, 0, dims, "", DataArrayID31);
  if(nullptr != m_StitchedImageArrayPtr.lock())                             /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_StitchedImageArray = m_StitchedImageArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkStitchImages::preflight()
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
void ItkStitchImages::execute()
{

  int err = 0;
  // typically run your dataCheck function to make sure you can get that far and all your variables are initialized
  dataCheck();
  // Check to make sure you made it through the data check. Errors would have been reported already so if something
  // happens to fail in the dataCheck() then we simply return
  if(getErrorCode() < 0)
  {
    return;
  }
  clearErrorCode();
  clearWarningCode();

  /* If some error occurs this code snippet can report the error up the call chain*/
  if (err < 0)
  {
    QString ss = QObject::tr("Error Importing a Zeiss AxioVision file set.");
    setErrorCondition(-90000, ss);
    return;
  }

  float minx = 1000000.0f;
  float maxx = 0.0f;
  float miny = 1000000.0f;
  float maxy = 0.0f;
  float value;

  AttributeMatrix::Pointer am = getDataContainerArray()->getAttributeMatrix(m_AttributeMatrixName);

  size_t numArrays = m_AttributeArrayNamesPtr.lock()->getNumberOfTuples();
  QList<QString> names;

  for (size_t i = 0; i < numArrays; i++)
  {
    QString tmpName = m_AttributeArrayNamesPtr.lock()->getValue(i);
    names.append(tmpName);
  }

  UInt8ArrayType::Pointer imagePtr = UInt8ArrayType::NullPointer();
  IDataArray::Pointer iDataArray = IDataArray::NullPointer();
  uint8_t* image = nullptr;

  DataArrayPath tempPath;

  // getting the fist data container just to get the dimensions of each image.
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getAttributeMatrixName().getDataContainerName());
  DataContainer::Pointer m2 = getDataContainerArray()->getDataContainer(getStitchedVolumeDataContainerName());

  std::vector<size_t> udims;
  udims = am->getTupleDimensions();

  for (size_t i = 0; i < names.size(); i++)
  {
    value = m_StitchedCoordinates[2 * i];
    if(value > maxx) { maxx = value; }
    if(value < minx) { minx = value; }

    value = m_StitchedCoordinates[2 * i + 1];
    if(value > maxy) { maxy = value; }
    if(value < miny) { miny = value; }
  }


  //    ImageProcessingConstants::UInt8ImageType* image2 = ImageProcessingConstants::UInt8ImageType::New();
  ImageProcessingConstants::UInt8ImageType::Pointer image2 = ImageProcessingConstants::UInt8ImageType::New();
  ImageProcessingConstants::UInt8ImageType::RegionType region;
  ImageProcessingConstants::UInt8ImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;
  start[2] = 0;

  ImageProcessingConstants::UInt8ImageType::SizeType size;
  unsigned int NumRows = udims[0] + abs(int(maxx)) + abs(int(minx));
  unsigned int NumCols = udims[1] + abs(int(maxy)) + abs(int(miny));
  size[0] = NumRows;
  size[1] = NumCols;
  size[2] = 1;

  region.SetSize(size);
  region.SetIndex(start);


  image2->SetRegions(region);
  image2->Allocate();

  using PasteImageFilterType = itk::PasteImageFilter<ImageProcessingConstants::UInt8ImageType, ImageProcessingConstants::UInt8ImageType>;
  PasteImageFilterType::Pointer pasteFilter = PasteImageFilterType::New ();

  using WriterType = itk::ImageFileWriter<ImageProcessingConstants::UInt8ImageType>;
  WriterType::Pointer writer = WriterType::New();

  ImageProcessingConstants::UInt8ImageType::IndexType destinationIndex;

  std::vector<size_t> tDims(3);
  tDims[0] = NumRows;
  tDims[1] = NumCols;
  tDims[2] = 1;

  m2->getAttributeMatrix(getStitchedAttributeMatrixName())->resizeAttributeArrays(tDims);
  m2->getGeometryAs<ImageGeom>()->setDimensions(tDims[0], tDims[1], tDims[2]);

  std::vector<size_t> cDims(1, 1);

  // run through all the data containers (images)
  for(size_t i = 0; i < names.size(); i++)
  {
    tempPath.update(getAttributeMatrixName().getDataContainerName(), getAttributeMatrixName().getAttributeMatrixName(), names[i]);
    iDataArray = getDataContainerArray()->getPrereqArrayFromPath<UInt8ArrayType, AbstractFilter>(this, tempPath, cDims);

    imagePtr = std::dynamic_pointer_cast<UInt8ArrayType>(iDataArray);

    if(nullptr != imagePtr.get())
    {
      image = imagePtr->getPointer(0);
      ImageProcessingConstants::ImportUInt8FilterType::Pointer importFilter = ITKUtilitiesType::Dream3DtoITKImportFilter<ImageProcessingConstants::DefaultPixelType>(m, getAttributeMatrixName().getAttributeMatrixName(), image);
      ImageProcessingConstants::UInt8ImageType* currentImage = importFilter->GetOutput();


      destinationIndex[0] = m_StitchedCoordinates[2 * i] + abs(int(minx));
      destinationIndex[1] = m_StitchedCoordinates[2 * i + 1] + abs(int(miny));;
      destinationIndex[2] = 0;

      pasteFilter->InPlaceOn();
      pasteFilter->SetSourceImage(currentImage);
      pasteFilter->SetDestinationImage(image2);
      pasteFilter->SetSourceRegion(currentImage->GetLargestPossibleRegion());
      pasteFilter->SetDestinationIndex(destinationIndex);
      pasteFilter->Update();
      image2 = pasteFilter->GetOutput();
      image2->DisconnectPipeline();
      ITKUtilitiesType::SetITKFilterOutput(pasteFilter->GetOutput(), m_StitchedImageArrayPtr.lock());
      pasteFilter->Update();
    }
  }

#if 0
  writer->SetFileName( "/Users/megnashah/Desktop/testImage.tiff");
  writer->SetInput( image2 );
  writer->Update();
#endif








}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkStitchImages::newFilterInstance(bool copyFilterParameters) const
{
  ItkStitchImages::Pointer filter = ItkStitchImages::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkStitchImages::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkStitchImages::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ItkStitchImages::getUuid() const
{
  return QUuid("{aed7a137-bf2f-5bbc-b5e6-bf5db18e46c2}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkStitchImages::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkStitchImages::getHumanLabel() const
{ return "Stitch Images (ImageProcessing)"; }

// -----------------------------------------------------------------------------
ItkStitchImages::Pointer ItkStitchImages::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ItkStitchImages> ItkStitchImages::New()
{
  struct make_shared_enabler : public ItkStitchImages
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ItkStitchImages::getNameOfClass() const
{
  return QString("ItkStitchImages");
}

// -----------------------------------------------------------------------------
QString ItkStitchImages::ClassName()
{
  return QString("ItkStitchImages");
}

// -----------------------------------------------------------------------------
void ItkStitchImages::setAttributeMatrixName(const DataArrayPath& value)
{
  m_AttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkStitchImages::getAttributeMatrixName() const
{
  return m_AttributeMatrixName;
}

// -----------------------------------------------------------------------------
void ItkStitchImages::setStitchedCoordinatesArrayPath(const DataArrayPath& value)
{
  m_StitchedCoordinatesArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkStitchImages::getStitchedCoordinatesArrayPath() const
{
  return m_StitchedCoordinatesArrayPath;
}

// -----------------------------------------------------------------------------
void ItkStitchImages::setAttributeArrayNamesPath(const DataArrayPath& value)
{
  m_AttributeArrayNamesPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkStitchImages::getAttributeArrayNamesPath() const
{
  return m_AttributeArrayNamesPath;
}

// -----------------------------------------------------------------------------
void ItkStitchImages::setStitchedVolumeDataContainerName(const DataArrayPath& value)
{
  m_StitchedVolumeDataContainerName = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkStitchImages::getStitchedVolumeDataContainerName() const
{
  return m_StitchedVolumeDataContainerName;
}

// -----------------------------------------------------------------------------
void ItkStitchImages::setStitchedImagesArrayName(const QString& value)
{
  m_StitchedImagesArrayName = value;
}

// -----------------------------------------------------------------------------
QString ItkStitchImages::getStitchedImagesArrayName() const
{
  return m_StitchedImagesArrayName;
}

// -----------------------------------------------------------------------------
void ItkStitchImages::setStitchedAttributeMatrixName(const QString& value)
{
  m_StitchedAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString ItkStitchImages::getStitchedAttributeMatrixName() const
{
  return m_StitchedAttributeMatrixName;
}
