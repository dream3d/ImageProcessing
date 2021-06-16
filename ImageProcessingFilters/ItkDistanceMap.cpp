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
#include "FindMaxima.h"

#include <limits>

//thresholding filter
#include "itkValuedRegionalMaximaImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"

#include "SIMPLib/Common/TemplateHelpers.h"
#include "SIMPLib/Common/TemplateHelpers.hpp"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

#include "ItkBridge.h"

#include "ImageProcessing/ImageProcessingConstants.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

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
    void static Execute(FindMaxima* filter, IDataArray::Pointer inputArray, double MinValue, bool* outputData, DataContainer::Pointer m, QString attrMatName)
    {
      typename DataArrayType::Pointer inputArrayPtr = std::dynamic_pointer_cast<DataArrayType>(inputArray);

      //convert array to correct type
      PixelType* inputData = static_cast<PixelType*>(inputArrayPtr->getPointer(0));

      size_t numVoxels = inputArrayPtr->getNumberOfTuples();

      typedef ItkBridge<PixelType> ItkBridgeType;

      //wrap input as itk image
      typedef itk::Image<PixelType, ImageProcessingConstants::ImageDimension> ImageType;
      typedef itk::Image<bool, ImageProcessingConstants::ImageDimension> BoolImageType;
      typename ImageType::Pointer inputImage = ItkBridgeType::CreateItkWrapperForDataPointer(m, attrMatName, inputData);

      //define filters
      typedef itk::ValuedRegionalMaximaImageFilter<ImageType, ImageType> RegionalMaximaType;
      typedef itk::BinaryThresholdImageFilter <ImageType, BoolImageType> ThresholdType;

      //find maxima
      typename RegionalMaximaType::Pointer maxima = RegionalMaximaType::New();
      maxima->SetInput(inputImage);

      //threshold
      typename ThresholdType::Pointer threshold = ThresholdType::New();
      threshold->SetInput(maxima->GetOutput());
      threshold->SetLowerThreshold((PixelType)MinValue);
      threshold->SetUpperThreshold(std::numeric_limits<PixelType>::max());
      threshold->SetInsideValue(true);
      threshold->SetOutsideValue(false);
      threshold->GetOutput()->GetPixelContainer()->SetImportPointer(outputData, numVoxels, false);

      try
      {
        threshold->Update();
      }
      catch( itk::ExceptionObject& err )
      {
        QString ss = QObject::tr("Failed to convert image. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
        filter->setErrorCondition(-5, ss);
      }
    }
  private:
    FindMaximaPrivate(const FindMaximaPrivate&) = delete; // Copy Constructor Not Implemented
    void operator=(const FindMaximaPrivate&) = delete;    // Operator '=' Not Implemented
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindMaxima::FindMaxima()
: m_SelectedCellArrayPath("", "", "")
, m_NewCellArrayName("Maxima")
, m_MinValue(1.0)
, m_SelectedCellArray(nullptr)
, m_NewCellArray(nullptr)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindMaxima::~FindMaxima() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMaxima::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(DataArraySelectionFilterParameter::Create("Input Array", "SelectedCellArrayPath", getSelectedCellArrayPath(), FilterParameter::Category::Uncategorized,
                                                                 SIMPL_BIND_SETTER(ItkDistanceMap, this, SelectedCellArrayPath), SIMPL_BIND_GETTER(ItkDistanceMap, this, SelectedCellArrayPath)));
  parameters.push_back(SIMPL_NEW_FLOAT_FP("Minimum Peak Intensity", MinValue, FilterParameter::Category::Uncategorized, ItkDistanceMap));
  parameters.push_back(SIMPL_NEW_STRING_FP("Created Array Name", NewCellArrayName, FilterParameter::Category::Uncategorized, ItkDistanceMap));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMaxima::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setMinValue( reader->readValue( "MinValue", getMinValue() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMaxima::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMaxima::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  //check for required arrays
  std::vector<size_t> compDims(1, 1);
  m_SelectedCellArrayPtr = TemplateHelpers::GetPrereqArrayFromPath<AbstractFilter>()(this, getSelectedCellArrayPath(), compDims);
  if(nullptr != m_SelectedCellArrayPtr.lock())
  {
    m_SelectedCellArray = m_SelectedCellArrayPtr.lock().get();
  }
  if(getErrorCondition() < 0) { return; }

  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCondition() < 0 || nullptr == image.get()) { return; }

  //configured created name / location
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );

  DataContainer::Pointer dataContiner = getDataContainerArray()->getPrereqDataContainer(this, getSelectedCellArrayPath().getDataContainerName() );
  IDataArray::Pointer redArrayptr = attrMatrix->getPrereqIDataArray<IDataArray, AbstractFilter>(this, getSelectedCellArrayPath().getDataArrayName(), 80000);

  //create new boolean array
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<bool>, AbstractFilter, bool>(this, tempPath, 0, compDims, "", DataArrayID31);
  if(nullptr != m_NewCellArrayPtr.lock())
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMaxima::preflight()
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
void FindMaxima::execute()
{
  QString ss;
  dataCheck();
  if(getErrorCondition() < 0)
  {
    ss = QObject::tr("DataCheck did not pass during execute");
    setErrorCondition(-10000, ss);
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
    FindMaximaPrivate<int8_t>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint8_t>()(inputData) )
  {
    FindMaximaPrivate<uint8_t>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<int16_t>()(inputData) )
  {
    FindMaximaPrivate<int16_t>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint16_t>()(inputData) )
  {
    FindMaximaPrivate<uint16_t>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<int32_t>()(inputData) )
  {
    FindMaximaPrivate<int32_t>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint32_t>()(inputData) )
  {
    FindMaximaPrivate<uint32_t>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<int64_t>()(inputData) )
  {
    FindMaximaPrivate<int64_t>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint64_t>()(inputData) )
  {
    FindMaximaPrivate<uint64_t>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<float>()(inputData) )
  {
    FindMaximaPrivate<float>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<double>()(inputData) )
  {
    FindMaximaPrivate<double>::Execute(this, inputData, m_MinValue, m_NewCellArray, m, attrMatName);
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
AbstractFilter::Pointer FindMaxima::newFilterInstance(bool copyFilterParameters) const
{
  FindMaxima::Pointer filter = FindMaxima::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindMaxima::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindMaxima::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindMaxima::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString FindMaxima::getHumanLabel() const
{ return "Find Maxima (ImageProcessing)"; }

