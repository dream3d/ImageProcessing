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

#include "ItkGrayToRGB.h"

//thresholding filter
#include "itkComposeImageFilter.h"

#include "SIMPLib/Common/TemplateHelpers.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/ITK/itkBridge.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

#include "ImageProcessing/ImageProcessingConstants.h"

/**
 * @brief This is a private implementation for the filter that handles the actual algorithm implementation details
 * for us like figuring out if we can use this private implementation with the data array that is assigned.
 */
template<typename PixelType>
class GrayToRGBPrivate
{
  public:
    typedef DataArray<PixelType> DataArrayType;

    GrayToRGBPrivate() = default;
    virtual ~GrayToRGBPrivate() = default;

    // -----------------------------------------------------------------------------
    // Determine if this is the proper type of an array to downcast from the IDataArray
    // -----------------------------------------------------------------------------
    bool operator()(IDataArray::Pointer p)
    {
      return (std::dynamic_pointer_cast<DataArrayType>(p).get() != nullptr);
    }

    // -----------------------------------------------------------------------------
    // This is the actual templated algorithm
    // -----------------------------------------------------------------------------
    void static Execute(ItkGrayToRGB* filter, IDataArray::Pointer redInputIDataArray, IDataArray::Pointer greenInputIDataArray, IDataArray::Pointer blueInputIDataArray, IDataArray::Pointer outputIDataArray, DataContainer::Pointer m, QString attrMatName)
    {
      typename DataArrayType::Pointer redInputDataPtr = std::dynamic_pointer_cast<DataArrayType>(redInputIDataArray);
      typename DataArrayType::Pointer greenInputDataPtr = std::dynamic_pointer_cast<DataArrayType>(greenInputIDataArray);
      typename DataArrayType::Pointer blueInputDataPtr = std::dynamic_pointer_cast<DataArrayType>(blueInputIDataArray);
      typename DataArrayType::Pointer outputDataPtr = std::dynamic_pointer_cast<DataArrayType>(outputIDataArray);

      //convert arrays to correct type
      PixelType* redData = static_cast<PixelType*>(redInputDataPtr->getPointer(0));
      PixelType* greenData = static_cast<PixelType*>(greenInputDataPtr->getPointer(0));
      PixelType* blueData = static_cast<PixelType*>(blueInputDataPtr->getPointer(0));
      PixelType* outputData = static_cast<PixelType*>(outputDataPtr->getPointer(0));

      size_t numVoxels = redInputDataPtr->getNumberOfTuples();

      typedef ItkBridge<PixelType> ItkBridgeType;

      //wrap inputs as itk images
      typedef itk::Image<PixelType, ImageProcessingConstants::ImageDimension> ImageType;
      typename ImageType::Pointer redImage = ItkBridgeType::CreateItkWrapperForDataPointer(m, attrMatName, redData);
      typename ImageType::Pointer greenImage = ItkBridgeType::CreateItkWrapperForDataPointer(m, attrMatName, greenData);
      typename ImageType::Pointer blueImage = ItkBridgeType::CreateItkWrapperForDataPointer(m, attrMatName, blueData);

      //define threshold filters
      typedef itk::ComposeImageFilter<typename ItkBridgeType::ScalarImageType, typename ItkBridgeType::RGBImageType> ComposeRGBType;

      //threshold
      typename ComposeRGBType::Pointer composeRGB = ComposeRGBType::New();
      composeRGB->SetInput(0, redImage);
      composeRGB->SetInput(1, greenImage);
      composeRGB->SetInput(2, blueImage);
      composeRGB->GetOutput()->GetPixelContainer()->SetImportPointer(reinterpret_cast<typename ItkBridgeType::RGBImageType::PixelType*>(outputData), numVoxels, false);

      try
      {
        composeRGB->Update();
      }
      catch( itk::ExceptionObject& err )
      {
        QString ss = QObject::tr("Failed to convert image. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
        filter->setErrorCondition(-5, ss);
      }
    }
  private:
    GrayToRGBPrivate(const GrayToRGBPrivate&) = delete; // Copy Constructor Not Implemented
    void operator=(const GrayToRGBPrivate&) = delete;   // Move assignment Not Implemented
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkGrayToRGB::ItkGrayToRGB() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkGrayToRGB::~ItkGrayToRGB() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkGrayToRGB::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::Defaults::AnyPrimitive, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Red Channel", RedArrayPath, FilterParameter::Category::RequiredArray, ItkGrayToRGB, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::Defaults::AnyPrimitive, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Green Channel", GreenArrayPath, FilterParameter::Category::RequiredArray, ItkGrayToRGB, req));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::Defaults::AnyPrimitive, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Blue Channel", BlueArrayPath, FilterParameter::Category::RequiredArray, ItkGrayToRGB, req));
  }
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("RGB Array", NewCellArrayName, RedArrayPath, RedArrayPath, FilterParameter::Category::CreatedArray, ItkGrayToRGB));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkGrayToRGB::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setRedArrayPath( reader->readDataArrayPath( "RedArrayPath", getRedArrayPath() ) );
  setGreenArrayPath( reader->readDataArrayPath( "GreenArrayPath", getGreenArrayPath() ) );
  setBlueArrayPath( reader->readDataArrayPath( "BlueArrayPath", getBlueArrayPath() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkGrayToRGB::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkGrayToRGB::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  //check for required arrays
  std::vector<size_t> compDims(1, 1);
  m_RedPtr = TemplateHelpers::GetPrereqArrayFromPath()(this, getRedArrayPath(), compDims);
  if(nullptr != m_RedPtr.lock())
  {
    m_Red = m_RedPtr.lock().get();
  }
  m_GreenPtr = TemplateHelpers::GetPrereqArrayFromPath()(this, getGreenArrayPath(), compDims);
  if(nullptr != m_GreenPtr.lock())
  {
    m_Green = m_GreenPtr.lock().get();
  }
  m_BluePtr = TemplateHelpers::GetPrereqArrayFromPath()(this, getBlueArrayPath(), compDims);
  if(nullptr != m_BluePtr.lock())
  {
    m_Blue = m_BluePtr.lock().get();
  }

  //configured created name / location
  tempPath.update(getRedArrayPath().getDataContainerName(), getRedArrayPath().getAttributeMatrixName(), getNewCellArrayName() );

  DataContainer::Pointer redDC = getDataContainerArray()->getPrereqDataContainer(this, getRedArrayPath().getDataContainerName());
  if(getErrorCode() < 0) { return; }

  AttributeMatrix::Pointer redAM = redDC->getPrereqAttributeMatrix(this, getRedArrayPath().getAttributeMatrixName(), 80000);
  if(getErrorCode() < 0)
  {
    return;
  }
  IDataArray::Pointer redArrayptr = redAM->getPrereqIDataArray(this, getRedArrayPath().getDataArrayName(), 80000);
  if(getErrorCode() < 0)
  {
    return;
  }
  ImageGeom::Pointer image = redDC->getPrereqGeometry<ImageGeom>(this);
  if(getErrorCode() < 0 || nullptr == image.get())
  {
    return;
  }

  //create new array of same type
  compDims[0] = 3;
  m_NewCellArrayPtr = TemplateHelpers::CreateNonPrereqArrayFromArrayType()(this, tempPath, compDims, redArrayptr);
  if(nullptr != m_NewCellArrayPtr.lock())
  {
    m_NewCellArray = m_NewCellArrayPtr.lock()->getVoidPointer(0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkGrayToRGB::execute()
{
  QString ss;
  dataCheck();
  if(getErrorCode() < 0)
  {
    ss = QObject::tr("DataCheck did not pass during execute");
    setErrorCondition(-13000, ss);
    return;
  }

  //get volume container
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getRedArrayPath().getDataContainerName());
  QString attrMatName = getRedArrayPath().getAttributeMatrixName();

  //get input and output data
  IDataArray::Pointer redData = m_RedPtr.lock();
  IDataArray::Pointer greenData = m_GreenPtr.lock();
  IDataArray::Pointer blueData = m_BluePtr.lock();
  IDataArray::Pointer outputData = m_NewCellArrayPtr.lock();

  //execute type dependant portion using a Private Implementation that takes care of figuring out if
  // we can work on the correct type and actually handling the algorithm execution. We pass in "this" so
  // that the private implementation can get access to the current object to pass up status notifications,
  // progress or handle "cancel" if needed.
  if(GrayToRGBPrivate<int8_t>()(redData))
  {
    GrayToRGBPrivate<int8_t>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else if(GrayToRGBPrivate<uint8_t>()(redData) )
  {
    GrayToRGBPrivate<uint8_t>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else if(GrayToRGBPrivate<int16_t>()(redData) )
  {
    GrayToRGBPrivate<int16_t>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else if(GrayToRGBPrivate<uint16_t>()(redData) )
  {
    GrayToRGBPrivate<uint16_t>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else if(GrayToRGBPrivate<int32_t>()(redData) )
  {
    GrayToRGBPrivate<int32_t>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else if(GrayToRGBPrivate<uint32_t>()(redData) )
  {
    GrayToRGBPrivate<uint32_t>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else if(GrayToRGBPrivate<int64_t>()(redData) )
  {
    GrayToRGBPrivate<int64_t>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else if(GrayToRGBPrivate<uint64_t>()(redData) )
  {
    GrayToRGBPrivate<uint64_t>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else if(GrayToRGBPrivate<float>()(redData) )
  {
    GrayToRGBPrivate<float>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else if(GrayToRGBPrivate<double>()(redData) )
  {
    GrayToRGBPrivate<double>::Execute(this, redData, greenData, blueData, outputData, m, attrMatName);
  }
  else
  {
    ss = QObject::tr("A Supported DataArray type was not used for an input array.");
    setErrorCondition(-10001, ss);
    return;
  }

  //array name changing/cleanup
  AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(m_RedArrayPath.getAttributeMatrixName());
  attrMat->insertOrAssign(outputData);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkGrayToRGB::newFilterInstance(bool copyFilterParameters) const
{
  ItkGrayToRGB::Pointer filter = ItkGrayToRGB::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkGrayToRGB::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkGrayToRGB::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ItkGrayToRGB::getUuid() const
{
  return QUuid("{d9b598d3-ef06-5776-8f68-362931fa5092}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkGrayToRGB::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkGrayToRGB::getHumanLabel() const
{ return "Convert Grayscale to RGB (Merge Channels) (ImageProcessing)"; }

// -----------------------------------------------------------------------------
ItkGrayToRGB::Pointer ItkGrayToRGB::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ItkGrayToRGB> ItkGrayToRGB::New()
{
  struct make_shared_enabler : public ItkGrayToRGB
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ItkGrayToRGB::getNameOfClass() const
{
  return QString("ItkGrayToRGB");
}

// -----------------------------------------------------------------------------
QString ItkGrayToRGB::ClassName()
{
  return QString("ItkGrayToRGB");
}

// -----------------------------------------------------------------------------
void ItkGrayToRGB::setRedArrayPath(const DataArrayPath& value)
{
  m_RedArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkGrayToRGB::getRedArrayPath() const
{
  return m_RedArrayPath;
}

// -----------------------------------------------------------------------------
void ItkGrayToRGB::setGreenArrayPath(const DataArrayPath& value)
{
  m_GreenArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkGrayToRGB::getGreenArrayPath() const
{
  return m_GreenArrayPath;
}

// -----------------------------------------------------------------------------
void ItkGrayToRGB::setBlueArrayPath(const DataArrayPath& value)
{
  m_BlueArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkGrayToRGB::getBlueArrayPath() const
{
  return m_BlueArrayPath;
}

// -----------------------------------------------------------------------------
void ItkGrayToRGB::setNewCellArrayName(const QString& value)
{
  m_NewCellArrayName = value;
}

// -----------------------------------------------------------------------------
QString ItkGrayToRGB::getNewCellArrayName() const
{
  return m_NewCellArrayName;
}
