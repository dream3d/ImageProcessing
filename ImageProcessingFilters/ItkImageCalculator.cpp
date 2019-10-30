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
#include <memory>

#include "ItkImageCalculator.h"

#include "itkAbsoluteValueDifferenceImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkAndImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkDivideImageFilter.h"
#include "itkMaximumImageFilter.h"
#include "itkMinimumImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkOrImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkXorImageFilter.h"

#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/ChoiceFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/ITK/itkBridge.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

#include "ImageProcessing/ImageProcessingConstants.h"
#include "ImageProcessing/ImageProcessingHelpers.hpp"

/* Create Enumerations to allow the created Attribute Arrays to take part in renaming */
enum createdPathID : RenameDataPath::DataID_t
{
  DataArrayID30 = 30,
  DataArrayID31 = 31,
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkImageCalculator::ItkImageCalculator()
: m_SelectedCellArrayPath1("", "", "")
, m_SelectedCellArrayPath2("", "", "")
, m_NewCellArrayName("")
, m_Operator(0)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkImageCalculator::~ItkImageCalculator() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImageCalculator::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt8, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("First Attribute Array to Process", SelectedCellArrayPath1, FilterParameter::RequiredArray, ItkImageCalculator, req));
  }
  {
    ChoiceFilterParameter::Pointer parameter = ChoiceFilterParameter::New();
    parameter->setHumanLabel("Operator");
    parameter->setPropertyName("Operator");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(ItkImageCalculator, this, Operator));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(ItkImageCalculator, this, Operator));

    QVector<QString> choices;
    choices.push_back("Add");
    choices.push_back("Subtract");
    choices.push_back("Multiply");
    choices.push_back("Divide");
    choices.push_back("AND");
    choices.push_back("OR");
    choices.push_back("XOR");
    choices.push_back("Min");
    choices.push_back("Max");
    choices.push_back("Mean");
    choices.push_back("Difference");
    parameter->setChoices(choices);
    parameter->setCategory(FilterParameter::Parameter);
    parameters.push_back(parameter);
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt8, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Second Array to Process", SelectedCellArrayPath2, FilterParameter::RequiredArray, ItkImageCalculator, req));
  }
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Output Attribute Array", NewCellArrayName, SelectedCellArrayPath1, SelectedCellArrayPath1, FilterParameter::CreatedArray, ItkImageCalculator));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImageCalculator::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath1( reader->readDataArrayPath( "SelectedCellArrayPath1", getSelectedCellArrayPath1() ) );
  setOperator( reader->readValue( "Operator", getOperator() ) );
  setSelectedCellArrayPath2( reader->readDataArrayPath( "SelectedCellArrayPath2", getSelectedCellArrayPath2() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImageCalculator::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImageCalculator::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  std::vector<size_t> dims(1, 1);
  m_SelectedCellArray1Ptr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter>(this, getSelectedCellArrayPath1(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if(nullptr != m_SelectedCellArray1Ptr.lock())                             /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_SelectedCellArray1 = m_SelectedCellArray1Ptr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() < 0)
  {
    return;
  }

  ImageGeom::Pointer image1 = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath1().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCode() < 0 || nullptr == image1.get())
  {
    return;
  }

  m_SelectedCellArray2Ptr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter>(this, getSelectedCellArrayPath2(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if(nullptr != m_SelectedCellArray2Ptr.lock())                             /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_SelectedCellArray2 = m_SelectedCellArray2Ptr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() < 0)
  {
    return;
  }

  ImageGeom::Pointer image2 = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath2().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCode() < 0 || nullptr == image2.get())
  {
    return;
  }

  tempPath.update(getSelectedCellArrayPath1().getDataContainerName(), getSelectedCellArrayPath1().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter, ImageProcessingConstants::DefaultPixelType>(
      this, tempPath, 0, dims, "", DataArrayID31);
  if(nullptr != m_NewCellArrayPtr.lock())                       /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImageCalculator::preflight()
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
void ItkImageCalculator::execute()
{
  //int err = 0;
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath1().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath1().getAttributeMatrixName();

  //wrap m_RawImageData as itk::image
  ImageProcessingConstants::DefaultImageType::Pointer inputImage1 = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_SelectedCellArray1);
  ImageProcessingConstants::DefaultImageType::Pointer inputImage2 = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_SelectedCellArray2);

  //define filters
  typedef itk::AddImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType> AddType;//
  typedef itk::SubtractImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType> SubtractType;//
  typedef itk::MultiplyImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType> MultiplyType;//
  typedef itk::DivideImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType> DivideType;//
  typedef itk::AndImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType> AndType;
  typedef itk::OrImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType> OrType;
  typedef itk::XorImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType> XorType;
  typedef itk::MinimumImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType> MinType;
  typedef itk::MaximumImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType> MaxType;
  typedef itk::BinaryFunctorImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType, ImageProcessing::Functor::Mean<ImageProcessingConstants::DefaultPixelType> > MeanType;
  typedef itk::AbsoluteValueDifferenceImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType> DifferenceType;

  //set up filters to cap image ranges
  typedef itk::UnaryFunctorImageFilter< ImageProcessingConstants::FloatImageType, ImageProcessingConstants::DefaultImageType, ImageProcessing::Functor::LimitsRound<ImageProcessingConstants::FloatPixelType, ImageProcessingConstants::DefaultPixelType> > LimitsRoundType;
  LimitsRoundType::Pointer limitsRound = LimitsRoundType::New();

  //set up and run selected filter
  switch(m_Operator)
  {
    case 0://add
    {
      AddType::Pointer add = AddType::New();
      add->SetInput1(inputImage1);
      add->SetInput2(inputImage2);
      limitsRound->SetInput(add->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 1://subtract
    {
      SubtractType::Pointer subtract = SubtractType::New();
      subtract->SetInput1(inputImage1);
      subtract->SetInput2(inputImage2);
      limitsRound->SetInput(subtract->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 2://multiply
    {
      MultiplyType::Pointer multiply = MultiplyType::New();
      multiply->SetInput1(inputImage1);
      multiply->SetInput2(inputImage2);
      limitsRound->SetInput(multiply->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 3://divide
    {
      DivideType::Pointer divide = DivideType::New();
      divide->SetInput1(inputImage1);
      divide->SetInput2(inputImage2);
      limitsRound->SetInput(divide->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 4://and
    {
      AndType::Pointer andfilter = AndType::New();
      andfilter->SetInput1(inputImage1);
      andfilter->SetInput2(inputImage2);
      ITKUtilitiesType::SetITKFilterOutput(andfilter->GetOutput(), m_NewCellArrayPtr.lock());
      andfilter->Update();
    }
    break;

    case 5://or
    {
      OrType::Pointer orfilter = OrType::New();
      orfilter->SetInput1(inputImage1);
      orfilter->SetInput2(inputImage2);
      ITKUtilitiesType::SetITKFilterOutput(orfilter->GetOutput(), m_NewCellArrayPtr.lock());
      orfilter->Update();
    }
    break;

    case 6://xor
    {
      XorType::Pointer xorfilter = XorType::New();
      xorfilter->SetInput1(inputImage1);
      xorfilter->SetInput2(inputImage2);
      ITKUtilitiesType::SetITKFilterOutput(xorfilter->GetOutput(), m_NewCellArrayPtr.lock());
      xorfilter->Update();
    }
    break;

    case 7://min
    {
      MinType::Pointer minimum = MinType::New();
      minimum->SetInput1(inputImage1);
      minimum->SetInput2(inputImage2);
      ITKUtilitiesType::SetITKFilterOutput(minimum->GetOutput(), m_NewCellArrayPtr.lock());
      minimum->Update();
    }
    break;

    case 8://max
    {
      MaxType::Pointer maximum = MaxType::New();
      maximum->SetInput1(inputImage1);
      maximum->SetInput2(inputImage2);
      ITKUtilitiesType::SetITKFilterOutput(maximum->GetOutput(), m_NewCellArrayPtr.lock());
      maximum->Update();
    }
    break;

    case 9://mean
    {
      MeanType::Pointer mean = MeanType::New();
      mean->SetInput1(inputImage1);
      mean->SetInput2(inputImage2);
      limitsRound->SetInput(mean->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 10://difference
    {
      DifferenceType::Pointer difference = DifferenceType::New();
      difference->SetInput1(inputImage1);
      difference->SetInput2(inputImage2);
      limitsRound->SetInput(difference->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkImageCalculator::newFilterInstance(bool copyFilterParameters) const
{
  /*
  * write code to optionally copy the filter parameters from the current filter into the new instance
  */
  ItkImageCalculator::Pointer filter = ItkImageCalculator::New();
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
    //    filter->setOutputFile(getOutputFile());
    //    filter->setZStartIndex(getZStartIndex());
    //    filter->setZEndIndex(getZEndIndex());
    //    filter->setZResolution(getZResolution());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkImageCalculator::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkImageCalculator::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ItkImageCalculator::getUuid() const
{
  return QUuid("{d0916970-0294-5970-aa76-efbee321b56f}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkImageCalculator::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkImageCalculator::getHumanLabel() const
{ return "Image Calculator (ImageProcessing)"; }

// -----------------------------------------------------------------------------
ItkImageCalculator::Pointer ItkImageCalculator::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ItkImageCalculator> ItkImageCalculator::New()
{
  struct make_shared_enabler : public ItkImageCalculator
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ItkImageCalculator::getNameOfClass() const
{
  return QString("ItkImageCalculator");
}

// -----------------------------------------------------------------------------
QString ItkImageCalculator::ClassName()
{
  return QString("ItkImageCalculator");
}

// -----------------------------------------------------------------------------
void ItkImageCalculator::setSelectedCellArrayPath1(const DataArrayPath& value)
{
  m_SelectedCellArrayPath1 = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkImageCalculator::getSelectedCellArrayPath1() const
{
  return m_SelectedCellArrayPath1;
}

// -----------------------------------------------------------------------------
void ItkImageCalculator::setSelectedCellArrayPath2(const DataArrayPath& value)
{
  m_SelectedCellArrayPath2 = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkImageCalculator::getSelectedCellArrayPath2() const
{
  return m_SelectedCellArrayPath2;
}

// -----------------------------------------------------------------------------
void ItkImageCalculator::setNewCellArrayName(const QString& value)
{
  m_NewCellArrayName = value;
}

// -----------------------------------------------------------------------------
QString ItkImageCalculator::getNewCellArrayName() const
{
  return m_NewCellArrayName;
}

// -----------------------------------------------------------------------------
void ItkImageCalculator::setOperator(unsigned int value)
{
  m_Operator = value;
}

// -----------------------------------------------------------------------------
unsigned int ItkImageCalculator::getOperator() const
{
  return m_Operator;
}
