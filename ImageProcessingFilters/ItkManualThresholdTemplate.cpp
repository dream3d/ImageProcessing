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
#include <memory>

#include "ItkManualThresholdTemplate.h"

#include <string>

//thresholding filter
#include "itkBinaryThresholdImageFilter.h"

#include "SIMPLib/Common/TemplateHelpers.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/IntFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
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
class ManualThresholdTemplatePrivate
{
  public:
    typedef DataArray<PixelType> DataArrayType;

    ManualThresholdTemplatePrivate() = default;
    virtual ~ManualThresholdTemplatePrivate() = default;

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
    void static Execute(ItkManualThresholdTemplate* filter, IDataArray::Pointer inputIDataArray, IDataArray::Pointer outputIDataArray, PixelType manParameter, DataContainer::Pointer m, QString attrMatName)
    {
      typename DataArrayType::Pointer inputDataPtr = std::dynamic_pointer_cast<DataArrayType>(inputIDataArray);
      typename DataArrayType::Pointer outputDataPtr = std::dynamic_pointer_cast<DataArrayType>(outputIDataArray);

      //convert arrays to correct type
      PixelType* inputData = static_cast<PixelType*>(inputDataPtr->getPointer(0));
      PixelType* outputData = static_cast<PixelType*>(outputDataPtr->getPointer(0));

      size_t numVoxels = inputDataPtr->getNumberOfTuples();

      typedef ItkBridge<PixelType> ItkBridgeType;

      //wrap input as itk image
      typedef itk::Image<PixelType, ImageProcessingConstants::ImageDimension> ImageType;
      typename ImageType::Pointer inputImage = ItkBridgeType::CreateItkWrapperForDataPointer(m, attrMatName, inputData);

      //define threshold filters
      typedef itk::BinaryThresholdImageFilter <ImageType, ImageType> BinaryThresholdImageFilterType;

      //threshold
      typename BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
      thresholdFilter->SetInput(inputImage);
      thresholdFilter->SetLowerThreshold(manParameter);
      thresholdFilter->SetUpperThreshold(0xFF);
      thresholdFilter->SetInsideValue(255);
      thresholdFilter->SetOutsideValue(0);
      thresholdFilter->GetOutput()->GetPixelContainer()->SetImportPointer(outputData, numVoxels, false);
      try
      {
        thresholdFilter->Update();
      }
      catch( itk::ExceptionObject& err )
      {
        QString ss = QObject::tr("Failed to execute itk::BinaryThresholdImageFilter filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
        filter->setErrorCondition(-5, ss);
      }
    }
  private:
    ManualThresholdTemplatePrivate(const ManualThresholdTemplatePrivate&) = delete; // Copy Constructor Not Implemented
    void operator=(const ManualThresholdTemplatePrivate&) = delete;                 // Move assignment Not Implemented
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkManualThresholdTemplate::ItkManualThresholdTemplate()
: m_SelectedCellArrayArrayPath("", "", "")
, m_NewCellArrayName("")
, m_SaveAsNewArray(true)
, m_ManualParameter(128)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkManualThresholdTemplate::~ItkManualThresholdTemplate() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::setupFilterParameters()
{
  FilterParameterVectorType parameters;

  parameters.push_back(SIMPL_NEW_INTEGER_FP("Threshold Value", ManualParameter, FilterParameter::Parameter, ItkManualThresholdTemplate));
  QStringList linkedProps;
  linkedProps << "NewCellArrayName";
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Save as New Array", SaveAsNewArray, FilterParameter::Parameter, ItkManualThresholdTemplate, linkedProps));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt8, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Threshold", SelectedCellArrayArrayPath, FilterParameter::RequiredArray, ItkManualThresholdTemplate, req));
  }
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_STRING_FP("Threshold Array", NewCellArrayName, FilterParameter::CreatedArray, ItkManualThresholdTemplate));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayArrayPath( reader->readDataArrayPath( "SelectedCellArrayArrayPath", getSelectedCellArrayArrayPath() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  setSaveAsNewArray( reader->readValue( "SaveAsNewArray", getSaveAsNewArray() ) );
  setManualParameter( reader->readValue( "ManualParameter", getManualParameter() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  //check for required arrays
  std::vector<size_t> compDims(1, 1);

  m_SelectedCellArrayPtr = TemplateHelpers::GetPrereqArrayFromPath()(this, getSelectedCellArrayArrayPath(), compDims);
  if(nullptr != m_SelectedCellArrayPtr.lock())
  {
    m_SelectedCellArray = m_SelectedCellArrayPtr.lock().get();
  }
  if(getErrorCode() < 0)
  {
    return;
  }

  //configured created name / location
  if(!m_SaveAsNewArray)
  {
    m_NewCellArrayName = "thisIsATempName";
  }
  tempPath.update(getSelectedCellArrayArrayPath().getDataContainerName(), getSelectedCellArrayArrayPath().getAttributeMatrixName(), getNewCellArrayName() );

  // We can safely just get the pointers without checking if they are nullptr because that was effectively done above in the GetPrereqArray call
  DataContainer::Pointer dc = getDataContainerArray()->getPrereqDataContainer(this, getSelectedCellArrayArrayPath().getDataContainerName());
  AttributeMatrix::Pointer am = dc->getPrereqAttributeMatrix(this, getSelectedCellArrayArrayPath().getAttributeMatrixName(), 80000);
  IDataArray::Pointer data = am->getPrereqIDataArray(this, getSelectedCellArrayArrayPath().getDataArrayName(), 80000);
  ImageGeom::Pointer image = dc->getPrereqGeometry<ImageGeom>(this);
  if(getErrorCode() < 0 || nullptr == image.get())
  {
    return;
  }

  m_NewCellArrayPtr = TemplateHelpers::CreateNonPrereqArrayFromArrayType()(this, tempPath, compDims, data);
  if(nullptr != m_NewCellArrayPtr.lock())
  {
    m_NewCellArray = m_NewCellArrayPtr.lock()->getVoidPointer(0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::preflight()
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
template<typename PixelType>
void filter(IDataArray::Pointer inputIDataArray, IDataArray::Pointer outputIDataArray, PixelType manParameter, DataContainer::Pointer m, QString attrMatName)
{
  typedef DataArray<PixelType> DataArrayType;
  typename DataArrayType::Pointer inputDataPtr = std::dynamic_pointer_cast<DataArrayType>(inputIDataArray);
  typename DataArrayType::Pointer outputDataPtr = std::dynamic_pointer_cast<DataArrayType>(outputIDataArray);

  //convert arrays to correct type
  PixelType* inputData = static_cast<PixelType*>(inputDataPtr->getPointer(0));
  PixelType* outputData = static_cast<PixelType*>(outputDataPtr->getPointer(0));

  size_t numVoxels = inputDataPtr->getNumberOfTuples();

  typedef ItkBridge<PixelType> ItkBridgeType;

  //wrap input as itk image
  typedef itk::Image<PixelType, ImageProcessingConstants::ImageDimension> ImageType;
  typename ImageType::Pointer inputImage = ItkBridgeType::CreateItkWrapperForDataPointer(m, attrMatName, inputData);

  //define threshold filters
  typedef itk::BinaryThresholdImageFilter <ImageType, ImageType> BinaryThresholdImageFilterType;

  //threshold
  typename BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(inputImage);
  thresholdFilter->SetLowerThreshold(manParameter);
  thresholdFilter->SetUpperThreshold(255);
  thresholdFilter->SetInsideValue(255);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->GetOutput()->GetPixelContainer()->SetImportPointer(outputData, numVoxels, false);
  thresholdFilter->Update();
}

/**example without itk
template<typename PixelType>
void filter(IDataArray::Pointer inputIDataArray, IDataArray::Pointer outputIDataArray, PixelType manParameter)
{
  typedef DataArray<PixelType> DataArrayType;
  typename DataArrayType::Pointer inputDataPtr = std::dynamic_pointer_cast<DataArrayType>(inputIDataArray);
  typename DataArrayType::Pointer outputDataPtr = std::dynamic_pointer_cast<DataArrayType>(outputIDataArray);

  //convert arrays to correct type
  PixelType* inputData = static_cast<PixelType*>(inputDataPtr->getPointer(0));
  PixelType* outputData = static_cast<PixelType*>(outputDataPtr->getPointer(0));

  size_t numVoxels = inputDataPtr->getNumberOfTuples();

  for(size_t i=0; i<numVoxels; i++) {
    if(inputData[i]>=manParameter) {
      outputData[i]=255;
    } else {
      outputData[i]=0;
    }
  }
}
*/

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::execute()
{
  QString ss;
  dataCheck();
  if(getErrorCode() < 0)
  {
    ss = QObject::tr("DataCheck did not pass during execute");
    setErrorCondition(-15000, ss);
    return;
  }

  //get volume container
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayArrayPath().getAttributeMatrixName();

  //get input and output data
  IDataArray::Pointer inputData = m_SelectedCellArrayPtr.lock();
  IDataArray::Pointer outputData = m_NewCellArrayPtr.lock();

  //execute type dependant portion using a Private Implementation that takes care of figuring out if
  // we can work on the correct type and actually handling the algorithm execution. We pass in "this" so
  // that the private implementation can get access to the current object to pass up status notifications,
  // progress or handle "cancel" if needed.
  if(ManualThresholdTemplatePrivate<int8_t>()(inputData))
  {
    ManualThresholdTemplatePrivate<int8_t>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else if(ManualThresholdTemplatePrivate<uint8_t>()(inputData) )
  {
    ManualThresholdTemplatePrivate<uint8_t>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else if(ManualThresholdTemplatePrivate<int16_t>()(inputData) )
  {
    ManualThresholdTemplatePrivate<int16_t>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else if(ManualThresholdTemplatePrivate<uint16_t>()(inputData) )
  {
    ManualThresholdTemplatePrivate<uint16_t>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else if(ManualThresholdTemplatePrivate<int32_t>()(inputData) )
  {
    ManualThresholdTemplatePrivate<int32_t>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else if(ManualThresholdTemplatePrivate<uint32_t>()(inputData) )
  {
    ManualThresholdTemplatePrivate<uint32_t>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else if(ManualThresholdTemplatePrivate<int64_t>()(inputData) )
  {
    ManualThresholdTemplatePrivate<int64_t>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else if(ManualThresholdTemplatePrivate<uint64_t>()(inputData) )
  {
    ManualThresholdTemplatePrivate<uint64_t>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else if(ManualThresholdTemplatePrivate<float>()(inputData) )
  {
    ManualThresholdTemplatePrivate<float>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else if(ManualThresholdTemplatePrivate<double>()(inputData) )
  {
    ManualThresholdTemplatePrivate<double>::Execute(this, inputData, outputData, getManualParameter(), m, attrMatName);
  }
  else
  {
    ss = QObject::tr("A Supported DataArray type was not used for an input array.");
    setErrorCondition(-10001, ss);
    return;
  }

  //array name changing/cleanup
  AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(m_SelectedCellArrayArrayPath.getAttributeMatrixName());
  if(m_SaveAsNewArray)
  {
    attrMat->insertOrAssign(outputData);
  }
  else
  {
    attrMat->removeAttributeArray(m_SelectedCellArrayArrayPath.getDataArrayName());
    outputData->setName(m_SelectedCellArrayArrayPath.getDataArrayName());
    attrMat->insertOrAssign(outputData);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkManualThresholdTemplate::newFilterInstance(bool copyFilterParameters) const
{
  ItkManualThresholdTemplate::Pointer filter = ItkManualThresholdTemplate::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkManualThresholdTemplate::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkManualThresholdTemplate::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ItkManualThresholdTemplate::getUuid() const
{
  return QUuid("{35de8117-cd91-5971-bc3a-73320cb9f37c}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkManualThresholdTemplate::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkManualThresholdTemplate::getHumanLabel() const
{ return "Threshold Image Template (Manual - Single Level) (ImageProcessing)"; }

// -----------------------------------------------------------------------------
ItkManualThresholdTemplate::Pointer ItkManualThresholdTemplate::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ItkManualThresholdTemplate> ItkManualThresholdTemplate::New()
{
  struct make_shared_enabler : public ItkManualThresholdTemplate
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ItkManualThresholdTemplate::getNameOfClass() const
{
  return QString("ItkManualThresholdTemplate");
}

// -----------------------------------------------------------------------------
QString ItkManualThresholdTemplate::ClassName()
{
  return QString("ItkManualThresholdTemplate");
}

// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::setSelectedCellArrayArrayPath(const DataArrayPath& value)
{
  m_SelectedCellArrayArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkManualThresholdTemplate::getSelectedCellArrayArrayPath() const
{
  return m_SelectedCellArrayArrayPath;
}

// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::setNewCellArrayName(const QString& value)
{
  m_NewCellArrayName = value;
}

// -----------------------------------------------------------------------------
QString ItkManualThresholdTemplate::getNewCellArrayName() const
{
  return m_NewCellArrayName;
}

// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::setSaveAsNewArray(bool value)
{
  m_SaveAsNewArray = value;
}

// -----------------------------------------------------------------------------
bool ItkManualThresholdTemplate::getSaveAsNewArray() const
{
  return m_SaveAsNewArray;
}

// -----------------------------------------------------------------------------
void ItkManualThresholdTemplate::setManualParameter(int value)
{
  m_ManualParameter = value;
}

// -----------------------------------------------------------------------------
int ItkManualThresholdTemplate::getManualParameter() const
{
  return m_ManualParameter;
}
