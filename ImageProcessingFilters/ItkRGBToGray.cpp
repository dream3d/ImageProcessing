/* ============================================================================
 * Copyright (c) 2014 William Lenthe
 * Copyright (c) 2016 BlueQuartz Software, LLC
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
#include "ItkRGBToGray.h"

#include <string>

// thresholding filter
#include "itkUnaryFunctorImageFilter.h"

#include "SIMPLib/Common/TemplateHelpers.hpp"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatVec3FilterParameter.h"
#include "SIMPLib/FilterParameters/MultiDataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

// ImageProcessing Plugin
#include "ImageProcessing/ImageProcessingHelpers.hpp"
#include "ItkBridge.h"

/**
 * @brief This is a private implementation for the filter that handles the actual algorithm implementation details
 * for us like figuring out if we can use this private implementation with the data array that is assigned.
 */
template <typename T> class RGBToGrayPrivate
{
public:
  typedef DataArray<T> DataArrayType;

  RGBToGrayPrivate()
  {
  }
  virtual ~RGBToGrayPrivate()
  {
  }

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
  void static Execute(ItkRGBToGray* filter, IDataArray::Pointer inputIDataArray, IDataArray::Pointer outputIDataArray, FloatVec3_t weights, DataContainer::Pointer m, QString attrMatName)
  {
    typename DataArrayType::Pointer inputDataPtr = std::dynamic_pointer_cast<DataArrayType>(inputIDataArray);
    typename DataArrayType::Pointer outputDataPtr = std::dynamic_pointer_cast<DataArrayType>(outputIDataArray);

    // Get the Raw Pointer to the Allocated Memory region (array) for the input and output arrays
    T* inputData = static_cast<T*>(inputDataPtr->getPointer(0));
    T* outputData = static_cast<T*>(outputDataPtr->getPointer(0));

    size_t numVoxels = inputDataPtr->getNumberOfTuples();

    // set weighting
    double mag = weights.x + weights.y + weights.z;

    // Define all the typedefs that are needed
    typedef ItkBridge<T> ItkBridgeType;
    typedef typename ItkBridgeType::RGBImageType RGBImageType;
    typedef typename RGBImageType::Pointer RGBImagePointerType;
    typedef typename RGBImageType::PixelType RGBImagePixelType;
    typedef typename ItkBridgeType::ScalarImageType ScalarImageType;
    typedef typename ScalarImageType::PixelType ScalarImagePixelType;
    // define Fucntor Typedef
    typedef ImageProcessing::Functor::Luminance<RGBImagePixelType, ScalarImagePixelType> LuminanceFunctorType;
    // define filters typedef
    typedef itk::UnaryFunctorImageFilter<RGBImageType, ScalarImageType, LuminanceFunctorType> RGBToGrayType;

    // wrap input as itk image
    RGBImagePointerType inputImage = ItkBridgeType::template Dream3DtoITKImportFilter<RGBImagePixelType>(m, attrMatName, inputData)->GetOutput();

    // convert to gray
    typename RGBToGrayType::Pointer itkFilter = RGBToGrayType::New();
    itkFilter->GetFunctor().SetRWeight(weights.x / mag);
    itkFilter->GetFunctor().SetGWeight(weights.y / mag);
    itkFilter->GetFunctor().SetBWeight(weights.z / mag);
    itkFilter->SetInput(inputImage);
    itkFilter->GetOutput()->GetPixelContainer()->SetImportPointer(outputData, numVoxels, false);
    try
    {
      itkFilter->Update();
    } catch(itk::ExceptionObject& err)
    {
      filter->setErrorCondition(-5);
      QString ss = QObject::tr("Failed to convert image. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
      filter->notifyErrorMessage(filter->getHumanLabel(), ss, filter->getErrorCondition());
    }
  }

private:
  RGBToGrayPrivate(const RGBToGrayPrivate&); // Copy Constructor Not Implemented
  void operator=(const RGBToGrayPrivate&);   // Operator '=' Not Implemented
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkRGBToGray::ItkRGBToGray()
: AbstractFilter()
//  m_SelectedCellArrayArrayPath("", "", ""),
//  m_NewCellArrayName(""),
, m_OutputAttributeMatrixName("")
, m_OutputArrayPrefix("GrayScale_")
//, m_SelectedCellArray(nullptr)
//, m_NewCellArray(nullptr)
{
  m_ColorWeights.x = 0.2125f;
  m_ColorWeights.y = 0.7154f;
  m_ColorWeights.z = 0.0721f;
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkRGBToGray::~ItkRGBToGray() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkRGBToGray::setupFilterParameters()
{
  FilterParameterVector parameters;

  parameters.push_back(SIMPL_NEW_FLOAT_VEC3_FP("Color Weighting", ColorWeights, FilterParameter::Parameter, ItkRGBToGray));
  parameters.push_back(SIMPL_NEW_STRING_FP("Output Array Prefix", OutputArrayPrefix, FilterParameter::Parameter, ItkRGBToGray));
  MultiDataArraySelectionFilterParameter::RequirementType req;
  req.dcGeometryTypes = IGeometry::Types(1, IGeometry::Type::Image);
  req.amTypes = AttributeMatrix::Types(1, AttributeMatrix::Type::Cell);
  parameters.push_back(SIMPL_NEW_MDA_SELECTION_FP("Input Attribute Arrays", InputDataArrayVector, FilterParameter::RequiredArray, ItkRGBToGray, req));
  parameters.push_back(SIMPL_NEW_STRING_FP("Output Cell Attribute Matrix", OutputAttributeMatrixName, FilterParameter::CreatedArray, ItkRGBToGray));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkRGBToGray::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setColorWeights(reader->readFloatVec3("ColorWeights", getColorWeights()));
  setInputDataArrayVector(reader->readDataArrayPathVector("InputDataArrayVector", getInputDataArrayVector()));
  setOutputAttributeMatrixName(reader->readString("OutputAttributeMatrixName", getOutputAttributeMatrixName()));
  setOutputArrayPrefix(reader->readString("OutputArrayPrefix", getOutputArrayPrefix()));

  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkRGBToGray::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkRGBToGray::dataCheck()
{
  setErrorCondition(0);
  setWarningCondition(0);
  if(DataArrayPath::ValidateVector(getInputDataArrayVector()) == false)
  {
    setErrorCondition(-62000);
    QString ss = QObject::tr("All Attribute Arrays must belong to the same Data Container and Attribute Matrix");
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  if(getOutputArrayPrefix().isEmpty())
  {
    setErrorCondition(-62002);
    QString message = QObject::tr("Using a prefix (even a single alphanumeric value) is required so that the output Xdmf files can be written correctly");
    notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
  }

  if(getInputDataArrayVector().isEmpty())
  {
    setErrorCondition(-62003);
    QString message = QObject::tr("At least one Attribute Array must be selected");
    notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
    return;
  }

  DataArrayPath inputAMPath = DataArrayPath::GetAttributeMatrixPath(getInputDataArrayVector());

  AttributeMatrix::Pointer inAM = getDataContainerArray()->getPrereqAttributeMatrixFromPath<AbstractFilter>(this, inputAMPath, -301);
  if(getErrorCondition() < 0 || nullptr == inAM.get())
  {
    return;
  }

  // Now create our output attributeMatrix which will contain all of our segmented images
  QVector<size_t> tDims = inAM->getTupleDimensions();
  DataContainerArray::Pointer dca = getDataContainerArray();
  DataContainer::Pointer dc = dca->getDataContainer(inputAMPath.getDataContainerName());
  AttributeMatrix::Pointer outAM = dc->createNonPrereqAttributeMatrix<AbstractFilter>(this, getOutputAttributeMatrixName(), tDims, AttributeMatrix::Type::Cell);
  if(getErrorCondition() < 0 || nullptr == outAM.get())
  {
    return;
  }

  // Get the list of checked array names from the input m_Data arrays list
  QList<QString> arrayNames = DataArrayPath::GetDataArrayNames(getInputDataArrayVector());

  for(int32_t i = 0; i < arrayNames.size(); i++)
  {
    QString daName = arrayNames.at(i);
    QString newName = getOutputArrayPrefix() + arrayNames.at(i);
    inputAMPath.setDataArrayName(daName);

    // getDataContainerArray()->getPrereqArrayFromPath<DataArray<uint8_t>, AbstractFilter>(this, inputAMPath, cDims);
    IDataArray::Pointer iDatArray = dca->getPrereqIDataArrayFromPath<IDataArray, ItkRGBToGray>(this, inputAMPath);
    if(getErrorCondition() < 0)
    {
      return;
    }
    QVector<size_t> outCDims(1, 1);
    outAM->createAndAddAttributeArray<UInt8ArrayType, AbstractFilter, uint8_t>(this, newName, 0, outCDims);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkRGBToGray::preflight()
{
  // These are the REQUIRED lines of CODE to make sure the filter behaves correctly
  setInPreflight(true);              // Set the fact that we are preflighting.
  emit preflightAboutToExecute();    // Emit this signal so that other widgets can do one file update
  emit updateFilterParameters(this); // Emit this signal to have the widgets push their values down to the filter
  dataCheck();                       // Run our DataCheck to make sure everthing is setup correctly
  emit preflightExecuted();          // We are done preflighting this filter
  setInPreflight(false);             // Inform the system this filter is NOT in preflight mode anymore.
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkRGBToGray::execute()
{
  QString ss;
  dataCheck();
  if(getErrorCondition() < 0)
  {
    setErrorCondition(-16000);
    ss = QObject::tr("DataCheck did not pass during execute");
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
  initialize();

  DataArrayPath inputAMPath = DataArrayPath::GetAttributeMatrixPath(getInputDataArrayVector());

  QList<QString> arrayNames = DataArrayPath::GetDataArrayNames(getInputDataArrayVector());
  QListIterator<QString> iter(arrayNames);

  while(iter.hasNext())
  {
    DataArrayPath arrayPath = inputAMPath;
    QString name = iter.next();
    arrayPath.setDataArrayName(name);

    // get volume container
    DataContainer::Pointer m = getDataContainerArray()->getDataContainer(arrayPath.getDataContainerName());
    QString attrMatName = arrayPath.getAttributeMatrixName();
    AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(arrayPath);

    // get input and output data
    IDataArray::Pointer inputData = attrMat->getAttributeArray(arrayPath.getDataArrayName());

    AttributeMatrix::Pointer outAttrMat = m->getAttributeMatrix(getOutputAttributeMatrixName());

    QString newName = getOutputArrayPrefix() + name;
    IDataArray::Pointer outputData = outAttrMat->getAttributeArray(newName);

    // execute type dependant portion using a Private Implementation that takes care of figuring out if
    // we can work on the correct type and actually handling the algorithm execution. We pass in "this" so
    // that the private implementation can get access to the current object to pass up status notifications,
    // progress or handle "cancel" if needed.
    if(RGBToGrayPrivate<int8_t>()(inputData))
    {
      RGBToGrayPrivate<int8_t>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else if(RGBToGrayPrivate<uint8_t>()(inputData))
    {
      RGBToGrayPrivate<uint8_t>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else if(RGBToGrayPrivate<int16_t>()(inputData))
    {
      RGBToGrayPrivate<int16_t>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else if(RGBToGrayPrivate<uint16_t>()(inputData))
    {
      RGBToGrayPrivate<uint16_t>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else if(RGBToGrayPrivate<int32_t>()(inputData))
    {
      RGBToGrayPrivate<int32_t>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else if(RGBToGrayPrivate<uint32_t>()(inputData))
    {
      RGBToGrayPrivate<uint32_t>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else if(RGBToGrayPrivate<int64_t>()(inputData))
    {
      RGBToGrayPrivate<int64_t>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else if(RGBToGrayPrivate<uint64_t>()(inputData))
    {
      RGBToGrayPrivate<uint64_t>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else if(RGBToGrayPrivate<float>()(inputData))
    {
      RGBToGrayPrivate<float>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else if(RGBToGrayPrivate<double>()(inputData))
    {
      RGBToGrayPrivate<double>::Execute(this, inputData, outputData, getColorWeights(), m, attrMatName);
    }
    else
    {
      setErrorCondition(-10001);
      ss = QObject::tr("A Supported DataArray type was not used for an input array.");
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
      return;
    }

    // array name changing/cleanup
    //    AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(m_SelectedCellArrayArrayPath.getAttributeMatrixName());
    //    attrMat->addAttributeArray(getNewCellArrayName(), outputData);
  }

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkRGBToGray::newFilterInstance(bool copyFilterParameters) const
{
  ItkRGBToGray::Pointer filter = ItkRGBToGray::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkRGBToGray::getCompiledLibraryName() const
{
  return ImageProcessingConstants::ImageProcessingBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkRGBToGray::getGroupName() const
{
  return SIMPL::FilterGroups::Unsupported;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QUuid ItkRGBToGray::getUuid()
{
  return QUuid("{e85a1475-63c1-5fc3-af14-b8ca67ac2ed6}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkRGBToGray::getSubGroupName() const
{
  return "Misc";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkRGBToGray::getHumanLabel() const
{
  return "Convert RGB to Grayscale (ImageProcessing)";
}
