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
#include "ItkDiscreteGaussianBlur.h"

#include "itkDiscreteGaussianImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include <QtCore/QString>

#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "SIMPLib/ITK/itkBridge.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkDiscreteGaussianBlur::ItkDiscreteGaussianBlur()
: m_SelectedCellArrayPath("", "", "")
, m_NewCellArrayName("")
, m_SaveAsNewArray(true)
, m_Stdev(2.0)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkDiscreteGaussianBlur::~ItkDiscreteGaussianBlur() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkDiscreteGaussianBlur::setupFilterParameters()
{
  FilterParameterVectorType parameters;

  QStringList linkedProps;
  linkedProps << "NewCellArrayName";
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Save as New Array", SaveAsNewArray, FilterParameter::Parameter, ItkDiscreteGaussianBlur, linkedProps));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt8, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Blur", SelectedCellArrayPath, FilterParameter::RequiredArray, ItkDiscreteGaussianBlur, req));
  }
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Blurred Array", NewCellArrayName, SelectedCellArrayPath, SelectedCellArrayPath, FilterParameter::CreatedArray, ItkDiscreteGaussianBlur));

  parameters.push_back(SIMPL_NEW_FLOAT_FP("Standard Deviation", Stdev, FilterParameter::Parameter, ItkDiscreteGaussianBlur));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkDiscreteGaussianBlur::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  setSaveAsNewArray( reader->readValue( "SaveAsNewArray", getSaveAsNewArray() ) );
  setStdev( reader->readValue( "Stdev", getStdev() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkDiscreteGaussianBlur::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkDiscreteGaussianBlur::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  QVector<size_t> dims(1, 1);
  m_SelectedCellArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter>(this, getSelectedCellArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if(nullptr != m_SelectedCellArrayPtr.lock())                            /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_SelectedCellArray = m_SelectedCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() < 0)
  {
    return;
  }

  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCode() < 0 || nullptr == image.get())
  {
    return;
  }

  if(!m_SaveAsNewArray)
  {
    m_NewCellArrayName = "thisIsATempName";
  }
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter, ImageProcessingConstants::DefaultPixelType>(
      this, tempPath, 0, dims, "", DataArrayID31);
  if(nullptr != m_NewCellArrayPtr.lock())                       /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkDiscreteGaussianBlur::preflight()
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
void ItkDiscreteGaussianBlur::execute()
{
  //int err = 0;
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath().getAttributeMatrixName();

  //wrap m_RawImageData as itk::image
  ImageProcessingConstants::DefaultImageType::Pointer inputImage = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_SelectedCellArray);

  //create Gaussian blur filter
  typedef itk::DiscreteGaussianImageFilter< ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType > GaussianFilterType;
  GaussianFilterType::Pointer gaussianFilter = GaussianFilterType::New();
  gaussianFilter->SetInput(inputImage);
  gaussianFilter->SetVariance(m_Stdev * m_Stdev);

  //convert result back to uint8
  typedef itk::RescaleIntensityImageFilter<ImageProcessingConstants::FloatImageType, ImageProcessingConstants::DefaultImageType> RescaleImageType;
  RescaleImageType::Pointer rescaleFilter = RescaleImageType::New();
  rescaleFilter->SetInput(gaussianFilter->GetOutput());
  rescaleFilter->SetOutputMinimum(0);
  rescaleFilter->SetOutputMaximum(255);


  //have filter write to dream3d array instead of creating its own buffer
  ITKUtilitiesType::SetITKFilterOutput(rescaleFilter->GetOutput(), m_NewCellArrayPtr.lock());

  //execute filters
  gaussianFilter->Update();
  rescaleFilter->Update();

  //array name changing/cleanup
  if(!m_SaveAsNewArray)
  {
    AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(m_SelectedCellArrayPath.getAttributeMatrixName());
    attrMat->removeAttributeArray(m_SelectedCellArrayPath.getDataArrayName());
    bool check = attrMat->renameAttributeArray(m_NewCellArrayName, m_SelectedCellArrayPath.getDataArrayName()) != 0u;
    if(!check)
    {

    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkDiscreteGaussianBlur::newFilterInstance(bool copyFilterParameters) const
{
  ItkDiscreteGaussianBlur::Pointer filter = ItkDiscreteGaussianBlur::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkDiscreteGaussianBlur::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkDiscreteGaussianBlur::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QUuid ItkDiscreteGaussianBlur::getUuid()
{
  return QUuid("{73ad993e-a36e-5978-bda2-ccee69d36186}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkDiscreteGaussianBlur::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkDiscreteGaussianBlur::getHumanLabel() const
{ return "Discrete Gaussian Blur (ImageProcessing)"; }

