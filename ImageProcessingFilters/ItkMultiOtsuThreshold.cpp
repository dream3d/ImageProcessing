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
#include "ItkMultiOtsuThreshold.h"

#include <QtCore/QString>

#include "SIMPLib/FilterParameters/AbstractFilterParametersWriter.h"
#include "SIMPLib/FilterParameters/BooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/IntFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "ItkBridge.h"

#include "itkOtsuMultipleThresholdsImageFilter.h"

// Include the MOC generated file for this class
#include "moc_ItkMultiOtsuThreshold.cpp"



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkMultiOtsuThreshold::ItkMultiOtsuThreshold() :
  AbstractFilter(),
  m_SelectedCellArrayPath("", "", ""),
  m_NewCellArrayName(""),
  m_SaveAsNewArray(true),
  m_Slice(false),
  m_Levels(1),
  m_SelectedCellArray(NULL),
  m_NewCellArray(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkMultiOtsuThreshold::~ItkMultiOtsuThreshold()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkMultiOtsuThreshold::setupFilterParameters()
{
  FilterParameterVector parameters;

  parameters.push_back(BooleanFilterParameter::New("Slice at a Time", "Slice", getSlice(), FilterParameter::Parameter));
  parameters.push_back(IntFilterParameter::New("Number of Levels", "Levels", getLevels(), FilterParameter::Parameter));
  QStringList linkedProps;
  linkedProps << "NewCellArrayName";
  parameters.push_back(LinkedBooleanFilterParameter::New("Save as New Array", "SaveAsNewArray", getSaveAsNewArray(), linkedProps, FilterParameter::Parameter));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt8, 1, SIMPL::AttributeMatrixObjectType::Any);
    parameters.push_back(DataArraySelectionFilterParameter::New("Attribute Array to Process", "SelectedCellArrayPath", getSelectedCellArrayPath(), FilterParameter::RequiredArray, req));
  }
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(StringFilterParameter::New("Output Attribute Array", "NewCellArrayName", getNewCellArrayName(), FilterParameter::CreatedArray));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkMultiOtsuThreshold::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  setSaveAsNewArray( reader->readValue( "SaveAsNewArray", getSaveAsNewArray() ) );
  setSlice( reader->readValue( "Slice", getSlice() ) );
  setLevels( reader->readValue( "Levels", getLevels() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ItkMultiOtsuThreshold::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  SIMPL_FILTER_WRITE_PARAMETER(SelectedCellArrayPath)
  SIMPL_FILTER_WRITE_PARAMETER(NewCellArrayName)
  SIMPL_FILTER_WRITE_PARAMETER(SaveAsNewArray)
  SIMPL_FILTER_WRITE_PARAMETER(Slice)
  SIMPL_FILTER_WRITE_PARAMETER(Levels)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkMultiOtsuThreshold::dataCheck()
{
  setErrorCondition(0);
  DataArrayPath tempPath;

  QVector<size_t> dims(1, 1);
  m_SelectedCellArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter>(this, getSelectedCellArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_SelectedCellArrayPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_SelectedCellArray = m_SelectedCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCondition() < 0) { return; }

  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCondition() < 0 || NULL == image.get()) { return; }

  if(m_SaveAsNewArray == false) { m_NewCellArrayName = "thisIsATempName"; }
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter, ImageProcessingConstants::DefaultPixelType>(this, tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_NewCellArrayPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkMultiOtsuThreshold::preflight()
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
void ItkMultiOtsuThreshold::execute()
{
  //int err = 0;
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath().getAttributeMatrixName();

  //get dims
  size_t udims[3] = {0, 0, 0};
  m->getGeometryAs<ImageGeom>()->getDimensions(udims);
#if (CMP_SIZEOF_SIZE_T == 4)
  typedef int32_t DimType;
#else
  typedef int64_t DimType;
#endif
  DimType dims[3] =
  {
    static_cast<DimType>(udims[0]),
    static_cast<DimType>(udims[1]),
    static_cast<DimType>(udims[2]),
  };

  //wrap input as itk image
  ImageProcessingConstants::DefaultImageType::Pointer inputImage = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_SelectedCellArray);

  if(m_Slice)
  {
    //define 2d histogram generator
    typedef itk::OtsuMultipleThresholdsImageFilter< ImageProcessingConstants::DefaultSliceType, ImageProcessingConstants::DefaultSliceType > ThresholdType;
    ThresholdType::Pointer otsuThresholder = ThresholdType::New();

    //wrap output buffer as image
    ImageProcessingConstants::DefaultImageType::Pointer outputImage = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_NewCellArray);

    //loop over slices
    for(int i = 0; i < dims[2]; i++)
    {
      //get slice
      ImageProcessingConstants::DefaultSliceType::Pointer slice = ITKUtilitiesType::ExtractSlice(inputImage, ImageProcessingConstants::ZSlice, i);

      //threshold
      otsuThresholder->SetInput(slice);
      otsuThresholder->SetNumberOfThresholds(m_Levels);
      otsuThresholder->SetLabelOffset(1);
      //execute filters
      try
      {
        otsuThresholder->Update();
      }
      catch( itk::ExceptionObject& err )
      {
        setErrorCondition(-5);
        QString ss = QObject::tr("Failed to execute itk::OtsuMultipleThresholdsImageFilter filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
        notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
      }

      //copy back into volume
      ITKUtilitiesType::SetSlice(outputImage, otsuThresholder->GetOutput(), ImageProcessingConstants::ZSlice, i);
    }
  }
  else
  {
    typedef itk::OtsuMultipleThresholdsImageFilter< ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType > ThresholdType;
    ThresholdType::Pointer otsuThresholder = ThresholdType::New();
    otsuThresholder->SetInput(inputImage);
    otsuThresholder->SetNumberOfThresholds(m_Levels);
    otsuThresholder->SetLabelOffset(1);

    ITKUtilitiesType::SetITKFilterOutput(otsuThresholder->GetOutput(), m_NewCellArrayPtr.lock());
    //execute filters
    try
    {
      otsuThresholder->Update();
    }
    catch( itk::ExceptionObject& err )
    {
      setErrorCondition(-5);
      QString ss = QObject::tr("Failed to execute itk::OtsuMultipleThresholdsImageFilter filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    }
  }

  //array name changing/cleanup
  if(m_SaveAsNewArray == false)
  {
    AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(m_SelectedCellArrayPath.getAttributeMatrixName());
    attrMat->removeAttributeArray(m_SelectedCellArrayPath.getDataArrayName());
    attrMat->renameAttributeArray(m_NewCellArrayName, m_SelectedCellArrayPath.getDataArrayName());
  }

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkMultiOtsuThreshold::newFilterInstance(bool copyFilterParameters)
{
  /*
  * write code to optionally copy the filter parameters from the current filter into the new instance
  */
  ItkMultiOtsuThreshold::Pointer filter = ItkMultiOtsuThreshold::New();
  if(true == copyFilterParameters)
  {
    /* If the filter uses all the standard Filter Parameter Widgets you can probabaly get
     * away with using this method to copy the filter parameters from the current instance
     * into the new instance
     */
    copyFilterParameterInstanceVariables(filter.get());
    /* If your filter is using a lot of custom FilterParameterWidgets @see ReadH5Ebsd then you
     * may need to copy each filter parameter explicitly plus any other instance variables that
     * are needed into the new instance. Here is some example code from ReadH5Ebsd
     */
    //    SIMPL_COPY_INSTANCEVAR(OutputFile)
    //    SIMPL_COPY_INSTANCEVAR(ZStartIndex)
    //    SIMPL_COPY_INSTANCEVAR(ZEndIndex)
    //    SIMPL_COPY_INSTANCEVAR(ZResolution)
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkMultiOtsuThreshold::getCompiledLibraryName()
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkMultiOtsuThreshold::getGroupName()
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkMultiOtsuThreshold::getSubGroupName()
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkMultiOtsuThreshold::getHumanLabel()
{return "ITK: Multi Level Otsu Threshold";}

