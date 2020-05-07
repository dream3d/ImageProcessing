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

#include "ItkAutoThreshold.h"

//histogram calculation
#include "itkImageToHistogramFilter.h"

//histogram based selectors
#include "itkHistogramThresholdCalculator.h"
#include "itkHuangThresholdCalculator.h"
#include "itkIntermodesThresholdCalculator.h"
#include "itkIsoDataThresholdCalculator.h"
#include "itkKittlerIllingworthThresholdCalculator.h"
#include "itkLiThresholdCalculator.h"
#include "itkMaximumEntropyThresholdCalculator.h"
#include "itkMomentsThresholdCalculator.h"
#include "itkOtsuThresholdCalculator.h"
#include "itkRenyiEntropyThresholdCalculator.h"
#include "itkShanbhagThresholdCalculator.h"
#include "itkTriangleThresholdCalculator.h"
#include "itkYenThresholdCalculator.h"
#include "itkBinaryThresholdImageFilter.h"

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/FilterParameters/BooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/ChoiceFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/ITK/itkBridge.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

#include "ImageProcessing/ImageProcessingConstants.h"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkAutoThreshold::ItkAutoThreshold()
: m_SelectedCellArrayPath("", "", "")
, m_NewCellArrayName("")
, m_SaveAsNewArray(true)
, m_Slice(false)
, m_Method(7)
, m_ManualParameter(128)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkAutoThreshold::~ItkAutoThreshold() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkAutoThreshold::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  {
    ChoiceFilterParameter::Pointer parameter = ChoiceFilterParameter::New();
    parameter->setHumanLabel("Threshold Method");
    parameter->setPropertyName("Method");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(ItkAutoThreshold, this, Method));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(ItkAutoThreshold, this, Method));

    QVector<QString> choices;
    choices.push_back("Huang");
    choices.push_back("Intermodes");
    choices.push_back("IsoData");
    choices.push_back("Kittler Illingworth");
    choices.push_back("Li");
    choices.push_back("Maximum Entropy");
    choices.push_back("Moments");
    choices.push_back("Otsu");
    choices.push_back("Renyi Entropy");
    choices.push_back("Shanbhag");
    choices.push_back("Triangle");
    choices.push_back("Yen");
    parameter->setChoices(choices);
    parameter->setCategory(FilterParameter::Parameter);
    parameters.push_back(parameter);
  }
  parameters.push_back(SIMPL_NEW_BOOL_FP("Slice at a Time", Slice, FilterParameter::Parameter, ItkAutoThreshold));
  QStringList linkedProps;
  linkedProps << "NewCellArrayName";
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Save as New Array", SaveAsNewArray, FilterParameter::Parameter, ItkAutoThreshold, linkedProps));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt8, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Process", SelectedCellArrayPath, FilterParameter::RequiredArray, ItkAutoThreshold, req));
  }
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Threshold Array", NewCellArrayName, SelectedCellArrayPath, SelectedCellArrayPath, FilterParameter::CreatedArray, ItkAutoThreshold));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkAutoThreshold::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  setSaveAsNewArray( reader->readValue( "SaveAsNewArray", getSaveAsNewArray() ) );
  setSlice( reader->readValue( "Slice", getSlice() ) );
  setMethod( reader->readValue( "Method", getMethod() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkAutoThreshold::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkAutoThreshold::dataCheck()
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
  if(getErrorCode() < 0 || nullptr == image)
  {
    return;
  }

  if(!m_SaveAsNewArray)
  {
    m_NewCellArrayName = "thisIsATempName";
  }
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>>(
      this, tempPath, 0, dims, "", DataArrayID31);
  if(nullptr != m_NewCellArrayPtr.lock())                       /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkAutoThreshold::execute()
{
  //int err = 0;
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath().getAttributeMatrixName();

  //get dims
  SizeVec3Type udims = m->getGeometryAs<ImageGeom>()->getDimensions();

  int64_t dims[3] =
  {
    static_cast<int64_t>(udims[0]),
    static_cast<int64_t>(udims[1]),
    static_cast<int64_t>(udims[2]),
  };

  //wrap input as itk image
  ImageProcessingConstants::DefaultImageType::Pointer inputImage = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_SelectedCellArray);

  //define threshold filters
  typedef itk::BinaryThresholdImageFilter <ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType> BinaryThresholdImageFilterType;
  typedef itk::BinaryThresholdImageFilter <ImageProcessingConstants::DefaultSliceType, ImageProcessingConstants::DefaultSliceType> BinaryThresholdImageFilterType2D;

  //define hitogram generator (will make the same kind of histogram for 2 and 3d images
  typedef itk::Statistics::ImageToHistogramFilter<ImageProcessingConstants::DefaultImageType> HistogramGenerator;

  //find threshold value w/ histogram
  itk::HistogramThresholdCalculator< HistogramGenerator::HistogramType, uint8_t >::Pointer calculator;

  typedef itk::HuangThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > HuangCalculatorType;
  typedef itk::IntermodesThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > IntermodesCalculatorType;
  typedef itk::IsoDataThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > IsoDataCalculatorType;
  typedef itk::KittlerIllingworthThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > KittlerIllingowrthCalculatorType;
  typedef itk::LiThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > LiCalculatorType;
  typedef itk::MaximumEntropyThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > MaximumEntropyCalculatorType;
  typedef itk::MomentsThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > MomentsCalculatorType;
  typedef itk::OtsuThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > OtsuCalculatorType;
  typedef itk::RenyiEntropyThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > RenyiEntropyCalculatorType;
  typedef itk::ShanbhagThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > ShanbhagCalculatorType;
  typedef itk::TriangleThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > TriangleCalculatorType;
  typedef itk::YenThresholdCalculator< HistogramGenerator::HistogramType, uint8_t > YenCalculatorType;

  switch(m_Method)
  {
    case 0:
    {
      calculator = HuangCalculatorType::New();
    }
    break;

    case 1:
    {
      calculator = IntermodesCalculatorType::New();
    }
    break;

    case 2:
    {
      calculator = IsoDataCalculatorType::New();
    }
    break;

    case 3:
    {
      calculator = KittlerIllingowrthCalculatorType::New();
    }
    break;

    case 4:
    {
      calculator = LiCalculatorType::New();
    }
    break;

    case 5:
    {
      calculator = MaximumEntropyCalculatorType::New();
    }
    break;

    case 6:
    {
      calculator = MomentsCalculatorType::New();
    }
    break;

    case 7:
    {
      calculator = OtsuCalculatorType::New();
    }
    break;

    case 8:
    {
      calculator = RenyiEntropyCalculatorType::New();
    }
    break;

    case 9:
    {
      calculator = ShanbhagCalculatorType::New();
    }
    break;

    case 10:
    {
      calculator = TriangleCalculatorType::New();
    }
    break;

    case 11:
    {
      calculator = YenCalculatorType::New();
    }
    break;
  }

  if(m_Slice)
  {
    //define 2d histogram generator
    typedef itk::Statistics::ImageToHistogramFilter<ImageProcessingConstants::DefaultSliceType> HistogramGenerator2D;
    HistogramGenerator2D::Pointer histogramFilter2D = HistogramGenerator2D::New();

    //specify number of bins / bounds
    typedef HistogramGenerator2D::HistogramSizeType SizeType;
    SizeType size( 1 );
    size[0] = 255;
    histogramFilter2D->SetHistogramSize( size );
    histogramFilter2D->SetMarginalScale( 10.0 );
    HistogramGenerator2D::HistogramMeasurementVectorType lowerBound( 1 );
    HistogramGenerator2D::HistogramMeasurementVectorType upperBound( 1 );
    lowerBound[0] = 0;
    upperBound[0] = 256;
    histogramFilter2D->SetHistogramBinMinimum( lowerBound );
    histogramFilter2D->SetHistogramBinMaximum( upperBound );

    //wrap output buffer as image
    ImageProcessingConstants::DefaultImageType::Pointer outputImage = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_NewCellArray);

    //loop over slices
    for(int i = 0; i < dims[2]; i++)
    {
      //get slice
      ImageProcessingConstants::DefaultSliceType::Pointer slice = ITKUtilitiesType::ExtractSlice(inputImage, ImageProcessingConstants::ZSlice, i);

      //find histogram
      histogramFilter2D->SetInput( slice );
      histogramFilter2D->Update();
      const HistogramGenerator::HistogramType* histogram = histogramFilter2D->GetOutput();

      //calculate threshold level
      calculator->SetInput(histogram);
      calculator->Update();
      const uint8_t thresholdValue = calculator->GetThreshold();

      //threshold
      BinaryThresholdImageFilterType2D::Pointer thresholdFilter = BinaryThresholdImageFilterType2D::New();
      thresholdFilter->SetInput(slice);
      thresholdFilter->SetLowerThreshold(thresholdValue);
      thresholdFilter->SetUpperThreshold(255);
      thresholdFilter->SetInsideValue(255);
      thresholdFilter->SetOutsideValue(0);
      thresholdFilter->Update();

      //copy back into volume
      ITKUtilitiesType::SetSlice(outputImage, thresholdFilter->GetOutput(), ImageProcessingConstants::ZSlice, i);
    }
  }
  else
  {
    //specify number of bins / bounds
    HistogramGenerator::Pointer histogramFilter = HistogramGenerator::New();
    typedef HistogramGenerator::HistogramSizeType SizeType;
    SizeType size( 1 );
    size[0] = 255;
    histogramFilter->SetHistogramSize( size );
    histogramFilter->SetMarginalScale( 10.0 );
    HistogramGenerator::HistogramMeasurementVectorType lowerBound( 1 );
    HistogramGenerator::HistogramMeasurementVectorType upperBound( 1 );
    lowerBound[0] = 0;
    upperBound[0] = 256;
    histogramFilter->SetHistogramBinMinimum( lowerBound );
    histogramFilter->SetHistogramBinMaximum( upperBound );

    //find histogram
    histogramFilter->SetInput( inputImage );
    histogramFilter->Update();
    const HistogramGenerator::HistogramType* histogram = histogramFilter->GetOutput();

    //calculate threshold level
    calculator->SetInput(histogram);
    calculator->Update();
    const uint8_t thresholdValue = calculator->GetThreshold();

    //threshold
    BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(inputImage);
    thresholdFilter->SetLowerThreshold(thresholdValue);
    thresholdFilter->SetUpperThreshold(255);
    thresholdFilter->SetInsideValue(255);
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->GetOutput()->GetPixelContainer()->SetImportPointer(m_NewCellArray, m_NewCellArrayPtr.lock()->getNumberOfTuples(), false);
    thresholdFilter->Update();
  }

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
AbstractFilter::Pointer ItkAutoThreshold::newFilterInstance(bool copyFilterParameters) const
{
  ItkAutoThreshold::Pointer filter = ItkAutoThreshold::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkAutoThreshold::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkAutoThreshold::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ItkAutoThreshold::getUuid() const
{
  return QUuid("{b1c401e4-cb40-574d-a663-2170b5a7cdaa}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkAutoThreshold::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkAutoThreshold::getHumanLabel() const
{ return "Threshold Image (Auto) (ImageProcessing)"; }

// -----------------------------------------------------------------------------
ItkAutoThreshold::Pointer ItkAutoThreshold::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ItkAutoThreshold> ItkAutoThreshold::New()
{
  struct make_shared_enabler : public ItkAutoThreshold
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ItkAutoThreshold::getNameOfClass() const
{
  return QString("ItkAutoThreshold");
}

// -----------------------------------------------------------------------------
QString ItkAutoThreshold::ClassName()
{
  return QString("ItkAutoThreshold");
}

// -----------------------------------------------------------------------------
void ItkAutoThreshold::setSelectedCellArrayPath(const DataArrayPath& value)
{
  m_SelectedCellArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkAutoThreshold::getSelectedCellArrayPath() const
{
  return m_SelectedCellArrayPath;
}

// -----------------------------------------------------------------------------
void ItkAutoThreshold::setNewCellArrayName(const QString& value)
{
  m_NewCellArrayName = value;
}

// -----------------------------------------------------------------------------
QString ItkAutoThreshold::getNewCellArrayName() const
{
  return m_NewCellArrayName;
}

// -----------------------------------------------------------------------------
void ItkAutoThreshold::setSaveAsNewArray(bool value)
{
  m_SaveAsNewArray = value;
}

// -----------------------------------------------------------------------------
bool ItkAutoThreshold::getSaveAsNewArray() const
{
  return m_SaveAsNewArray;
}

// -----------------------------------------------------------------------------
void ItkAutoThreshold::setSlice(bool value)
{
  m_Slice = value;
}

// -----------------------------------------------------------------------------
bool ItkAutoThreshold::getSlice() const
{
  return m_Slice;
}

// -----------------------------------------------------------------------------
void ItkAutoThreshold::setMethod(unsigned int value)
{
  m_Method = value;
}

// -----------------------------------------------------------------------------
unsigned int ItkAutoThreshold::getMethod() const
{
  return m_Method;
}

// -----------------------------------------------------------------------------
void ItkAutoThreshold::setManualParameter(int value)
{
  m_ManualParameter = value;
}

// -----------------------------------------------------------------------------
int ItkAutoThreshold::getManualParameter() const
{
  return m_ManualParameter;
}
