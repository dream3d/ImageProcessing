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

#include "ItkFindMaxima.h"

#include "SIMPLib/Common/TemplateHelpers.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/ITK/itkBridge.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

// ImageProcessing Plugin
#include "ImageProcessing/ImageProcessingConstants.h"
#include "ImageProcessing/ImageProcessingHelpers.hpp"

#include "itkBinaryImageToLabelMapFilter.h"
#include "itkBinaryThresholdImageFunction.h"
#include "itkFloodFilledImageFunctionConditionalIterator.h"
#include "itkRegionalMaximaImageFilter.h"

/**
 * @brief This is a private implementation for the filter that handles the actual algorithm implementation details
 * for us like figuring out if we can use this private implementation with the data array that is assigned.
 */
template<typename PixelType>
class FindMaximaPrivate
{
  public:
    typedef DataArray<PixelType> DataArrayType;

    FindMaximaPrivate() = default;
    virtual ~FindMaximaPrivate() = default;

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
    void static Execute(ItkFindMaxima* filter, IDataArray::Pointer inputArray, double tolerance, bool* outputData, DataContainer::Pointer m, QString attrMatName)
    {
      typename DataArrayType::Pointer inputArrayPtr = std::dynamic_pointer_cast<DataArrayType>(inputArray);

      //convert array to correct type
      PixelType* inputData = static_cast<PixelType*>(inputArrayPtr->getPointer(0));

      //size_t numVoxels = inputArrayPtr->getNumberOfTuples();

//      typedef ItkBridge<PixelType> ItkBridgeType;

      //wrap input and output as itk image
      typedef itk::Image<PixelType, ImageProcessingConstants::ImageDimension> ImageType;
      typedef itk::Image<bool, ImageProcessingConstants::ImageDimension> BoolImageType;
      typename ImageType::Pointer inputImage = ItkBridge<PixelType>::CreateItkWrapperForDataPointer(m, attrMatName, inputData);
      BoolImageType::Pointer outputImage = ItkBridge<bool>::CreateItkWrapperForDataPointer(m, attrMatName, outputData);

      //find maxima
      std::vector<typename ImageType::IndexType> peakLocations;
      try
      {
        peakLocations = ImageProcessing::LocalMaxima<ImageType>::Find(inputImage, tolerance, true);
      }
      catch( itk::ExceptionObject& err )
      {
        QString ss = QObject::tr("Failed to convert image. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
        filter->setErrorCondition(-5, ss);
      }

      //fill output data with false then set peaks to true
      outputImage->FillBuffer(false);
      for(size_t i = 0; i < peakLocations.size(); i++)
      {
        outputImage->SetPixel(peakLocations[i], true);
      }
    }
  private:
    FindMaximaPrivate(const FindMaximaPrivate&) = delete; // Copy Constructor Not Implemented
    void operator=(const FindMaximaPrivate&) = delete;    // Move assignment Not Implemented
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkFindMaxima::ItkFindMaxima()
: m_SelectedCellArrayPath("", "", "")
, m_Tolerance(1.0)
, m_NewCellArrayName("Maxima")
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkFindMaxima::~ItkFindMaxima() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkFindMaxima::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::Defaults::AnyPrimitive, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Input Attribute Array", SelectedCellArrayPath, FilterParameter::Category::RequiredArray, ItkFindMaxima, req));
  }
  parameters.push_back(SIMPL_NEW_FLOAT_FP("Noise Tolerance", Tolerance, FilterParameter::Category::Parameter, ItkFindMaxima));
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::CreatedArray));
  parameters.push_back(
      SIMPL_NEW_DA_WITH_LINKED_AM_FP("Output Attribute Array", NewCellArrayName, SelectedCellArrayPath, SelectedCellArrayPath, FilterParameter::Category::CreatedArray, ItkFindMaxima));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkFindMaxima::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setTolerance( reader->readValue( "Tolerance", getTolerance() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkFindMaxima::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkFindMaxima::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  //check for required arrays
  std::vector<size_t> compDims(1, 1);
  m_SelectedCellArrayPtr = TemplateHelpers::GetPrereqArrayFromPath()(this, getSelectedCellArrayPath(), compDims);
  if(nullptr != m_SelectedCellArrayPtr.lock())
  {
    m_SelectedCellArray = m_SelectedCellArrayPtr.lock().get();
  }

  //configured created name / location
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );

  DataContainer::Pointer dataContiner = getDataContainerArray()->getPrereqDataContainer(this, getSelectedCellArrayPath().getDataContainerName() );
  if(getErrorCode() < 0)
  {
    return;
  }
  AttributeMatrix::Pointer attrMatrix = dataContiner->getPrereqAttributeMatrix(this, getSelectedCellArrayPath().getAttributeMatrixName(), 80000);
  if(getErrorCode() < 0)
  {
    return;
  }
  IDataArray::Pointer redArrayptr = attrMatrix->getPrereqIDataArray(this, getSelectedCellArrayPath().getDataArrayName(), 80000);
  if(getErrorCode() < 0 || nullptr == redArrayptr.get())
  {
    return;
  }
  ImageGeom::Pointer image = dataContiner->getPrereqGeometry<ImageGeom>(this);
  if(getErrorCode() < 0 || nullptr == image.get())
  {
    return;
  }
  //create new boolean array
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<bool>>(this, tempPath, false, compDims);
  if(nullptr != m_NewCellArrayPtr.lock())
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkFindMaxima::execute()
{
  QString ss;
  dataCheck();
  if(getErrorCode() < 0)
  {
    ss = QObject::tr("DataCheck did not pass during execute");
    setErrorCondition(-12000, ss);
    return;
  }

  //get volume container
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath().getAttributeMatrixName();

  //get input data
  IDataArray::Pointer inputData = m_SelectedCellArrayPtr.lock();

  //execute type dependant portion using a Private Implementation that takes care of figuring out if
  // we can work on the correct type and actually handling the algorithm execution. We pass in "this" so
  // that the private implementation can get access to the current object to pass up status notifications,
  // progress or handle "cancel" if needed.
  if(FindMaximaPrivate<int8_t>()(inputData))
  {
    FindMaximaPrivate<int8_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint8_t>()(inputData) )
  {
    FindMaximaPrivate<uint8_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<int16_t>()(inputData) )
  {
    FindMaximaPrivate<int16_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint16_t>()(inputData) )
  {
    FindMaximaPrivate<uint16_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<int32_t>()(inputData) )
  {
    FindMaximaPrivate<int32_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint32_t>()(inputData) )
  {
    FindMaximaPrivate<uint32_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<int64_t>()(inputData) )
  {
    FindMaximaPrivate<int64_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint64_t>()(inputData) )
  {
    FindMaximaPrivate<uint64_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<float>()(inputData) )
  {
    FindMaximaPrivate<float>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<double>()(inputData) )
  {
    FindMaximaPrivate<double>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else
  {
    ss = QObject::tr("A Supported DataArray type was not used for an input array.");
    setErrorCondition(-10001, ss);
    return;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkFindMaxima::newFilterInstance(bool copyFilterParameters) const
{
  ItkFindMaxima::Pointer filter = ItkFindMaxima::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkFindMaxima::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkFindMaxima::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ItkFindMaxima::getUuid() const
{
  return QUuid("{d2ebf8df-1469-5b77-bfcd-e9e99344749e}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkFindMaxima::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkFindMaxima::getHumanLabel() const
{ return "Find Maxima (ImageProcessing)"; }

// -----------------------------------------------------------------------------
ItkFindMaxima::Pointer ItkFindMaxima::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ItkFindMaxima> ItkFindMaxima::New()
{
  struct make_shared_enabler : public ItkFindMaxima
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ItkFindMaxima::getNameOfClass() const
{
  return QString("ItkFindMaxima");
}

// -----------------------------------------------------------------------------
QString ItkFindMaxima::ClassName()
{
  return QString("ItkFindMaxima");
}

// -----------------------------------------------------------------------------
void ItkFindMaxima::setSelectedCellArrayPath(const DataArrayPath& value)
{
  m_SelectedCellArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkFindMaxima::getSelectedCellArrayPath() const
{
  return m_SelectedCellArrayPath;
}

// -----------------------------------------------------------------------------
void ItkFindMaxima::setTolerance(float value)
{
  m_Tolerance = value;
}

// -----------------------------------------------------------------------------
float ItkFindMaxima::getTolerance() const
{
  return m_Tolerance;
}

// -----------------------------------------------------------------------------
void ItkFindMaxima::setNewCellArrayName(const QString& value)
{
  m_NewCellArrayName = value;
}

// -----------------------------------------------------------------------------
QString ItkFindMaxima::getNewCellArrayName() const
{
  return m_NewCellArrayName;
}
