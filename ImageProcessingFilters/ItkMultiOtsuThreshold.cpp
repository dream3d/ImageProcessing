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

#include "SIMPLib/FilterParameters/BooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/IntFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "SIMPLib/ITK/itkBridge.h"

#include "itkOtsuMultipleThresholdsImageFilter.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkMultiOtsuThreshold::ItkMultiOtsuThreshold()
: m_SelectedCellArrayPath("", "", "")
, m_NewCellArrayName("")
, m_SaveAsNewArray(true)
, m_Slice(false)
, m_Levels(1)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkMultiOtsuThreshold::~ItkMultiOtsuThreshold() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkMultiOtsuThreshold::setupFilterParameters()
{
  FilterParameterVector parameters;

  parameters.push_back(SIMPL_NEW_BOOL_FP("Slice at a Time", Slice, FilterParameter::Parameter, ItkMultiOtsuThreshold));
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Number of Levels", Levels, FilterParameter::Parameter, ItkMultiOtsuThreshold));
  QStringList linkedProps;
  linkedProps << "NewCellArrayName";
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Save as New Array", SaveAsNewArray, FilterParameter::Parameter, ItkMultiOtsuThreshold, linkedProps));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt8, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Process", SelectedCellArrayPath, FilterParameter::RequiredArray, ItkMultiOtsuThreshold, req));
  }
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_STRING_FP("Output Attribute Array", NewCellArrayName, FilterParameter::CreatedArray, ItkMultiOtsuThreshold));

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
void ItkMultiOtsuThreshold::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkMultiOtsuThreshold::dataCheck()
{
  setErrorCondition(0);
  setWarningCondition(0);
  DataArrayPath tempPath;

  QVector<size_t> dims(1, 1);
  m_SelectedCellArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter>(this, getSelectedCellArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if(nullptr != m_SelectedCellArrayPtr.lock())                            /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_SelectedCellArray = m_SelectedCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCondition() < 0) { return; }

  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCondition() < 0 || nullptr == image.get()) { return; }

  if(!m_SaveAsNewArray)
  {
    m_NewCellArrayName = "thisIsATempName";
  }
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter, ImageProcessingConstants::DefaultPixelType>(this, tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if(nullptr != m_NewCellArrayPtr.lock())                       /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
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
  std::tie(udims[0], udims[1], udims[2]) = m->getGeometryAs<ImageGeom>()->getDimensions();

  int64_t dims[3] =
  {
    static_cast<int64_t>(udims[0]),
    static_cast<int64_t>(udims[1]),
    static_cast<int64_t>(udims[2]),
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
        QString ss = QObject::tr("Failed to execute itk::OtsuMultipleThresholdsImageFilter filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
        notifyErrorMessage("", ss, -5);
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
      QString ss = QObject::tr("Failed to execute itk::OtsuMultipleThresholdsImageFilter filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
      notifyErrorMessage("", ss, -5);
    }
  }

  //array name changing/cleanup
  if(!m_SaveAsNewArray)
  {
    AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(m_SelectedCellArrayPath.getAttributeMatrixName());
    attrMat->removeAttributeArray(m_SelectedCellArrayPath.getDataArrayName());
    attrMat->renameAttributeArray(m_NewCellArrayName, m_SelectedCellArrayPath.getDataArrayName());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkMultiOtsuThreshold::newFilterInstance(bool copyFilterParameters) const
{
  /*
  * write code to optionally copy the filter parameters from the current filter into the new instance
  */
  ItkMultiOtsuThreshold::Pointer filter = ItkMultiOtsuThreshold::New();
  if(copyFilterParameters)
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
const QString ItkMultiOtsuThreshold::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkMultiOtsuThreshold::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QUuid ItkMultiOtsuThreshold::getUuid()
{
  return QUuid("{fa281497-2f98-5fc7-bfd7-305dcea866ed}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkMultiOtsuThreshold::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkMultiOtsuThreshold::getHumanLabel() const
{ return "Multi Level Otsu Threshold (ImageProcessing)"; }

