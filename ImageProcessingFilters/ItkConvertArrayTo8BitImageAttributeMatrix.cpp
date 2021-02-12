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
#include "ItkConvertArrayTo8BitImageAttributeMatrix.h"

#include <QtCore/QTextStream>

#include "SIMPLib/ITK/itkSupportConstants.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/FilterParameters/AttributeMatrixSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"

#include "ImageProcessing/ImageProcessingConstants.h"
#include "ImageProcessing/ImageProcessingVersion.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkConvertArrayTo8BitImageAttributeMatrix::ItkConvertArrayTo8BitImageAttributeMatrix()
: m_AttributeMatrixName(SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellAttributeMatrixName, "")
, m_NewArrayArrayName("")
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkConvertArrayTo8BitImageAttributeMatrix::~ItkConvertArrayTo8BitImageAttributeMatrix() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImageAttributeMatrix::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::Create("Cell Data", FilterParameter::Category::RequiredArray));
  {
    AttributeMatrixSelectionFilterParameter::RequirementType req;
    parameters.push_back(SIMPL_NEW_AM_SELECTION_FP("Cell Attribute Matrix", AttributeMatrixName, FilterParameter::Category::RequiredArray, ItkConvertArrayTo8BitImageAttributeMatrix, req));
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImageAttributeMatrix::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setAttributeMatrixName(reader->readDataArrayPath("AttributeMatrixName", getAttributeMatrixName()));
//  setNewArrayArrayName(reader->readString("NewArrayArrayName", getNewArrayArrayName() ) );
//  setSelectedArrayPath( reader->readDataArrayPath( "SelectedArrayPath", getSelectedArrayPath() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImageAttributeMatrix::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImageAttributeMatrix::dataCheck()
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

    for(int i = 0; i < names.size(); i++)
    {
      tempPath.update(getAttributeMatrixName().getDataContainerName(), getAttributeMatrixName().getAttributeMatrixName(), names[i]);
      IDataArray::Pointer inputData = getDataContainerArray()->getPrereqIDataArrayFromPath(this, tempPath);
      if(getErrorCode() < 0)
      {
        return;
      }

      if(inputData->getNumberOfComponents() > 1)
      {
        QString ss = QObject::tr("Data Array '%1' cannot have more than 1 component").arg(names[i]);
        setErrorCondition(-11002, ss);
        return;
        }
    }


    ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getAttributeMatrixName().getDataContainerName())->getPrereqGeometry<ImageGeom>(this);
    if(getErrorCode() < 0 || nullptr == image.get())
    {
      return;
    }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
template<typename T>
void scaleArray2(IDataArray::Pointer inputData, uint8_t* newArray)
{
  typename DataArray<T>::Pointer inputArray = std::dynamic_pointer_cast<DataArray<T>>(inputData);
  if (nullptr == inputArray.get())
  {
    return;
  }

  T* inputArrayPtr = inputArray->getPointer(0);
  size_t numPoints = inputArray->getNumberOfTuples();

  T min = std::numeric_limits<T>::max();
  T max = 0.0f;
  T value;
  T scaledValue;
  for (size_t i = 0; i < numPoints; i++)
  {
    value = inputArrayPtr[i];
    if(value > max) { max = value; }
    if(value < min) { min = value; }
  }

  float delta = (max - min);
  if(delta < 0.0000001)
  {
    for (size_t i = 0; i < numPoints; i++)
    {
      scaledValue = inputArrayPtr[i];
      if(scaledValue < 0.0) { scaledValue = 0.0f; }
      else if(scaledValue > 1.0f) { scaledValue = 1.0f; }
      scaledValue *= 255.0f;
      newArray[i] = static_cast<uint8_t>(scaledValue);
    }
  }
  else
  {
    for (size_t i = 0; i < numPoints; i++)
    {
      scaledValue = (inputArrayPtr[i] - min) / delta;
      if(scaledValue < 0.0) { scaledValue = 0.0f; }
      else if(scaledValue > 1.0f) { scaledValue = 1.0f; }
      scaledValue *= 255.0f;
      newArray[i] = static_cast<uint8_t>(scaledValue);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImageAttributeMatrix::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  AttributeMatrix::Pointer am = getDataContainerArray()->getAttributeMatrix(m_AttributeMatrixName);
  QList<QString> names = am->getAttributeArrayNames();
  DataArrayPath tempPath;
  std::vector<size_t> dims(1, 1);

  for(size_t i = 0; i < names.size(); i++)
  {

    m_NewArrayArrayName = names[i] + "8bit";
    tempPath.update(getAttributeMatrixName().getDataContainerName(), getAttributeMatrixName().getAttributeMatrixName(), getNewArrayArrayName() );
    m_NewArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint8_t>>(this, tempPath, 0, dims, "", DataArrayID31);
    if(nullptr != m_NewArrayPtr.lock())
    { m_NewArray = m_NewArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

    IDataArray::Pointer inputData = getDataContainerArray()->getDataContainer(getAttributeMatrixName().getDataContainerName())->getAttributeMatrix(getAttributeMatrixName().getAttributeMatrixName())->getAttributeArray(names[i]);


    QString dType = inputData->getTypeAsString();
    IDataArray::Pointer p = IDataArray::NullPointer();
    if (dType.compare("int8_t") == 0)
    {
      scaleArray2<int8_t>(inputData, m_NewArray);
    }
    else if (dType.compare("uint8_t") == 0)
    {
      scaleArray2<uint8_t>(inputData, m_NewArray);
    }
    else if (dType.compare("int16_t") == 0)
    {
      scaleArray2<int16_t>(inputData, m_NewArray);
    }
    else if (dType.compare("uint16_t") == 0)
    {
      scaleArray2<uint16_t>(inputData, m_NewArray);
    }
    else if (dType.compare("int32_t") == 0)
    {
      scaleArray2<int32_t>(inputData, m_NewArray);
    }
    else if (dType.compare("uint32_t") == 0)
    {
      scaleArray2<uint32_t>(inputData, m_NewArray);
    }
    else if (dType.compare("int64_t") == 0)
    {
      scaleArray2<int64_t>(inputData, m_NewArray);
    }
    else if (dType.compare("uint64_t") == 0)
    {
      scaleArray2<uint64_t>(inputData, m_NewArray);
    }
    else if (dType.compare("float") == 0)
    {
      scaleArray2<float>(inputData, m_NewArray);
    }
    else if (dType.compare("double") == 0)
    {
      scaleArray2<double>(inputData, m_NewArray);
    }
//      else if (dType.compare("bool") == 0)
//      {
//        scaleArray2<bool>(inputData, m_NewArray);
//      }

    am->removeAttributeArray(names[i]);
    am->renameAttributeArray(names[i] + "8bit", names[i]);


  }





}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkConvertArrayTo8BitImageAttributeMatrix::newFilterInstance(bool copyFilterParameters) const
{
  ItkConvertArrayTo8BitImageAttributeMatrix::Pointer filter = ItkConvertArrayTo8BitImageAttributeMatrix::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkConvertArrayTo8BitImageAttributeMatrix::getCompiledLibraryName() const
{
  return ImageProcessingConstants::ImageProcessingBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkConvertArrayTo8BitImageAttributeMatrix::getBrandingString() const
{
  return "ImageProcessing";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkConvertArrayTo8BitImageAttributeMatrix::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream <<  ImageProcessing::Version::Major() << "." << ImageProcessing::Version::Minor() << "." << ImageProcessing::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkConvertArrayTo8BitImageAttributeMatrix::getGroupName() const
{ return SIMPL::FilterGroups::Unsupported; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ItkConvertArrayTo8BitImageAttributeMatrix::getUuid() const
{
  return QUuid("{cd075a60-93a9-52b4-ace6-84342b742c0a}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkConvertArrayTo8BitImageAttributeMatrix::getSubGroupName() const
{ return "Misc"; }


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkConvertArrayTo8BitImageAttributeMatrix::getHumanLabel() const
{ return "Convert Array to 8 Bit Image Attribute Matrix (ImageProcessing)"; }

// -----------------------------------------------------------------------------
ItkConvertArrayTo8BitImageAttributeMatrix::Pointer ItkConvertArrayTo8BitImageAttributeMatrix::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ItkConvertArrayTo8BitImageAttributeMatrix> ItkConvertArrayTo8BitImageAttributeMatrix::New()
{
  struct make_shared_enabler : public ItkConvertArrayTo8BitImageAttributeMatrix
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ItkConvertArrayTo8BitImageAttributeMatrix::getNameOfClass() const
{
  return QString("ItkConvertArrayTo8BitImageAttributeMatrix");
}

// -----------------------------------------------------------------------------
QString ItkConvertArrayTo8BitImageAttributeMatrix::ClassName()
{
  return QString("ItkConvertArrayTo8BitImageAttributeMatrix");
}

// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImageAttributeMatrix::setAttributeMatrixName(const DataArrayPath& value)
{
  m_AttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkConvertArrayTo8BitImageAttributeMatrix::getAttributeMatrixName() const
{
  return m_AttributeMatrixName;
}

// -----------------------------------------------------------------------------
void ItkConvertArrayTo8BitImageAttributeMatrix::setNewArrayArrayName(const QString& value)
{
  m_NewArrayArrayName = value;
}

// -----------------------------------------------------------------------------
QString ItkConvertArrayTo8BitImageAttributeMatrix::getNewArrayArrayName() const
{
  return m_NewArrayArrayName;
}
