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

#include "ItkWatershed.h"

#include <QtCore/QString>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/ITK/itkBridge.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

#include "ImageProcessing/ImageProcessingConstants.h"

//ITK Includes
#include "itkGradientMagnitudeImageFilter.h"
#include "itkWatershedImageFilter.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkWatershed::ItkWatershed()
: m_SelectedCellArrayPath("", "", "")
, m_FeatureIdsArrayName(SIMPL::CellData::FeatureIds)
, m_Threshold(0.005f)
, m_Level(0.5f)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkWatershed::~ItkWatershed() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWatershed::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt8, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Image Data", SelectedCellArrayPath, FilterParameter::RequiredArray, ItkWatershed, req));
  }
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Feature Ids", FeatureIdsArrayName, SelectedCellArrayPath, SelectedCellArrayPath, FilterParameter::CreatedArray, ItkWatershed));
  parameters.push_back(SIMPL_NEW_FLOAT_FP("Threshold", Threshold, FilterParameter::Parameter, ItkWatershed));
  parameters.push_back(SIMPL_NEW_FLOAT_FP("Level", Level, FilterParameter::Parameter, ItkWatershed));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWatershed::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setFeatureIdsArrayName( reader->readString( "FeatureIdsArrayName", getFeatureIdsArrayName() ) );
  setThreshold( reader->readValue( "Threshold", getThreshold() ) );
  setLevel( reader->readValue( "Level", getLevel() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWatershed::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWatershed::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  std::vector<size_t> dims(1, 1);
  m_SelectedCellArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>>(this, getSelectedCellArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if(nullptr != m_SelectedCellArrayPtr.lock())                            /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_SelectedCellArray = m_SelectedCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() < 0)
  {
    return;
  }

  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom>(this);
  if(getErrorCode() < 0 || nullptr == image.get())
  {
    return;
  }

  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getFeatureIdsArrayName() );
  m_FeatureIdsPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, 0, dims, "", DataArrayID31);
  if(nullptr != m_FeatureIdsPtr.lock())                     /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWatershed::execute()
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

  //create gradient magnitude filter
  notifyStatusMessage("Calculating Gradient Magnitude");
  typedef itk::GradientMagnitudeImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType >  GradientMagnitudeImageFilterType;
  GradientMagnitudeImageFilterType::Pointer gradientMagnitudeImageFilter = GradientMagnitudeImageFilterType::New();
  gradientMagnitudeImageFilter->SetInput(inputImage);
  gradientMagnitudeImageFilter->Update();

  //watershed image
  notifyStatusMessage("Watershedding");
  typedef itk::WatershedImageFilter<ImageProcessingConstants::DefaultImageType> WatershedFilterType;
  WatershedFilterType::Pointer watershed = WatershedFilterType::New();
  watershed->SetThreshold(m_Threshold);
  watershed->SetLevel(m_Level);
  watershed->SetInput(gradientMagnitudeImageFilter->GetOutput());

  //execute filter
  try
  {
    watershed->Update();
  }
  catch( itk::ExceptionObject& err )
  {
    QString ss = QObject::tr("Failed to execute itk::GradientMagnitudeImageFilter filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
    setErrorCondition(-5, ss);
  }

  //get output and copy to grainids
  typedef itk::Image<ImageProcessingConstants::UInt64PixelType, ImageProcessingConstants::ImageDimension>   WatershedImageType;
  WatershedFilterType::OutputImageType* output = watershed->GetOutput();
  WatershedImageType::RegionType filterRegion = output->GetLargestPossibleRegion();
  typedef itk::ImageRegionConstIterator< WatershedFilterType::OutputImageType > WatershedIteratorType;
  WatershedIteratorType it(output, filterRegion);
  it.GoToBegin();
  int index = 0;
  while(!it.IsAtEnd())
  {
    m_FeatureIds[index] = it.Get();
    ++it;
    ++index;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkWatershed::newFilterInstance(bool copyFilterParameters) const
{
  ItkWatershed::Pointer filter = ItkWatershed::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkWatershed::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkWatershed::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ItkWatershed::getUuid() const
{
  return QUuid("{994eb3c3-dafa-5611-8afe-efdac8c2f7da}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkWatershed::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkWatershed::getHumanLabel() const
{ return "Watershed Filter (ImageProcessing)"; }

// -----------------------------------------------------------------------------
ItkWatershed::Pointer ItkWatershed::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ItkWatershed> ItkWatershed::New()
{
  struct make_shared_enabler : public ItkWatershed
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ItkWatershed::getNameOfClass() const
{
  return QString("ItkWatershed");
}

// -----------------------------------------------------------------------------
QString ItkWatershed::ClassName()
{
  return QString("ItkWatershed");
}

// -----------------------------------------------------------------------------
void ItkWatershed::setSelectedCellArrayPath(const DataArrayPath& value)
{
  m_SelectedCellArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkWatershed::getSelectedCellArrayPath() const
{
  return m_SelectedCellArrayPath;
}

// -----------------------------------------------------------------------------
void ItkWatershed::setFeatureIdsArrayName(const QString& value)
{
  m_FeatureIdsArrayName = value;
}

// -----------------------------------------------------------------------------
QString ItkWatershed::getFeatureIdsArrayName() const
{
  return m_FeatureIdsArrayName;
}

// -----------------------------------------------------------------------------
void ItkWatershed::setThreshold(float value)
{
  m_Threshold = value;
}

// -----------------------------------------------------------------------------
float ItkWatershed::getThreshold() const
{
  return m_Threshold;
}

// -----------------------------------------------------------------------------
void ItkWatershed::setLevel(float value)
{
  m_Level = value;
}

// -----------------------------------------------------------------------------
float ItkWatershed::getLevel() const
{
  return m_Level;
}
