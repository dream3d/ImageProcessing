/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * Copyright (c) 2013 Dr. Joseph C. Tucker (UES, Inc.)
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
 * Neither the name of Joseph C. Tucker, Michael A. Groeber, Michael A. Jackson,
 * UES, Inc., the US Air Force, BlueQuartz Software nor the names of its contributors
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
 *  This code was written under United States Air Force Contract number
 *                   FA8650-07-D-5800 and FA8650-10-D-5226
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "ItkConvertArrayTo8BitImage.h"

#include "SIMPLib/ITK/itkSupportConstants.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/SIMPLibVersion.h"

#include "ImageProcessing/ImageProcessingVersion.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkConvertArrayTo8BitImage::ItkConvertArrayTo8BitImage()
: m_SelectedArrayPath("", "", "")
, m_NewArrayArrayName("")
, m_NewArray(nullptr)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkConvertArrayTo8BitImage::~ItkConvertArrayTo8BitImage() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImage::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::Defaults::AnyPrimitive, 3, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array To Convert", SelectedArrayPath, FilterParameter::RequiredArray, ItkConvertArrayTo8BitImage, req));
  }
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_STRING_FP("Converted Attribute Array", NewArrayArrayName, FilterParameter::CreatedArray, ItkConvertArrayTo8BitImage));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImage::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setNewArrayArrayName(reader->readString("NewArrayArrayName", getNewArrayArrayName() ) );
  setSelectedArrayPath( reader->readDataArrayPath( "SelectedArrayPath", getSelectedArrayPath() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImage::initialize()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImage::dataCheck()
{
  DataArrayPath tempPath;
  setErrorCondition(0);
  setWarningCondition(0);

  if(m_SelectedArrayPath.isEmpty() == true)
  {
    setErrorCondition(-11000);
    notifyErrorMessage(getHumanLabel(), "An array from the DataContainer must be selected.", getErrorCondition());
  }
  else
  {
    IDataArray::Pointer inputData = getDataContainerArray()->getPrereqIDataArrayFromPath<IDataArray, AbstractFilter>(this, getSelectedArrayPath());
    if (getErrorCondition() < 0) { return; }
    else
    {
      if(inputData->getNumberOfComponents() > 1)
      {
        QString ss = QObject::tr("Data Array '%1' cannot have more than 1 component").arg(m_SelectedArrayPath.getDataArrayName());
        setErrorCondition(-11002);
        notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
        return;
      }
      QVector<size_t> dims(1, 1);
      tempPath.update(m_SelectedArrayPath.getDataContainerName(), m_SelectedArrayPath.getAttributeMatrixName(), getNewArrayArrayName() );
      m_NewArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint8_t>, AbstractFilter, uint8_t>(this, tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
      if(nullptr != m_NewArrayPtr.lock())                   /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
      { m_NewArray = m_NewArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
      if(getErrorCondition() < 0) { return; }

      ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getSelectedArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
      if(getErrorCondition() < 0 || nullptr == image.get()) { return; }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImage::preflight()
{
  setInPreflight(true);
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template<typename T>
void scaleArray(IDataArray::Pointer inputData, uint8_t* newArray)
{
  typename DataArray<T>::Pointer inputArray = std::dynamic_pointer_cast<DataArray<T>>(inputData);
  if (nullptr == inputArray)
  {
    return;
  }

  T* inputArrayPtr = inputArray->getPointer(0);
  size_t numPoints = inputArray->getNumberOfTuples();

  float min = 1000000.0f;
  float max = 0.0f;
  float value;
  float scaledValue;
  for (size_t i = 0; i < numPoints; i++)
  {
    value = inputArrayPtr[i];
    if(value > max) { max = value; }
    if(value < min) { min = value; }
  }

  for (size_t i = 0; i < numPoints; i++)
  {
    scaledValue = (inputArrayPtr[i] - min) / (max - min);
    if(scaledValue < 0.0) { scaledValue = 0.0f; }
    else if(scaledValue > 1.0f) { scaledValue = 1.0f; }
    scaledValue *= 255.0f;
    newArray[i] = static_cast<uint8_t>(scaledValue);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImage::execute()
{
  setErrorCondition(0);
  setWarningCondition(0);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(m_SelectedArrayPath.getDataContainerName());

  QString ss;

  IDataArray::Pointer inputData = getDataContainerArray()->getDataContainer(m_SelectedArrayPath.getDataContainerName())->getAttributeMatrix(m_SelectedArrayPath.getAttributeMatrixName())->getAttributeArray(m_SelectedArrayPath.getDataArrayName());

  QString dType = inputData->getTypeAsString();
  IDataArray::Pointer p = IDataArray::NullPointer();
  if (dType.compare("int8_t") == 0)
  {
    scaleArray<int8_t>(inputData, m_NewArray);
  }
  else if (dType.compare("uint8_t") == 0)
  {
    scaleArray<uint8_t>(inputData, m_NewArray);
  }
  else if (dType.compare("int16_t") == 0)
  {
    scaleArray<int16_t>(inputData, m_NewArray);
  }
  else if (dType.compare("uint16_t") == 0)
  {
    scaleArray<uint16_t>(inputData, m_NewArray);
  }
  else if (dType.compare("int32_t") == 0)
  {
    scaleArray<int32_t>(inputData, m_NewArray);
  }
  else if (dType.compare("uint32_t") == 0)
  {
    scaleArray<uint32_t>(inputData, m_NewArray);
  }
  else if (dType.compare("int64_t") == 0)
  {
    scaleArray<int64_t>(inputData, m_NewArray);
  }
  else if (dType.compare("uint64_t") == 0)
  {
    scaleArray<uint64_t>(inputData, m_NewArray);
  }
  else if (dType.compare("float") == 0)
  {
    scaleArray<float>(inputData, m_NewArray);
  }
  else if (dType.compare("double") == 0)
  {
    scaleArray<double>(inputData, m_NewArray);
  }
  else if (dType.compare("bool") == 0)
  {
    scaleArray<bool>(inputData, m_NewArray);
  }

  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkConvertArrayTo8BitImage::newFilterInstance(bool copyFilterParameters) const
{
  ItkConvertArrayTo8BitImage::Pointer filter = ItkConvertArrayTo8BitImage::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkConvertArrayTo8BitImage::getCompiledLibraryName() const
{
  return ImageProcessingConstants::ImageProcessingBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkConvertArrayTo8BitImage::getBrandingString() const
{
  return "ImageProcessing";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkConvertArrayTo8BitImage::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream <<  ImageProcessing::Version::Major() << "." << ImageProcessing::Version::Minor() << "." << ImageProcessing::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkConvertArrayTo8BitImage::getGroupName() const
{ return SIMPL::FilterGroups::Unsupported; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QUuid ItkConvertArrayTo8BitImage::getUuid()
{
  return QUuid("{52f9e4c4-4e1c-55a3-a316-30c9e99c1216}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkConvertArrayTo8BitImage::getSubGroupName() const
{ return "Misc"; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkConvertArrayTo8BitImage::getHumanLabel() const
{ return "Convert Array to 8 Bit Image (ImageProcessing)"; }

