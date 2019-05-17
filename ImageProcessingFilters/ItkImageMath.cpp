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
#include "ItkImageMath.h"

#include "SIMPLib/ITK/itkBridge.h"
#include "itkAddImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkDivideImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkLogImageFilter.h"
#include "itkMaximumImageFilter.h"
#include "itkMinimumImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkSqrtImageFilter.h"
#include "itkSquareImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkUnaryFunctorImageFilter.h"

#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/DoubleFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/ChoiceFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

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
ItkImageMath::ItkImageMath()
: m_SelectedCellArrayPath("", "", "")
, m_NewCellArrayName("")
, m_SaveAsNewArray(true)
, m_Operator(0)
, m_Value(1)
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkImageMath::~ItkImageMath() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImageMath::setupFilterParameters()
{
  FilterParameterVectorType parameters;

  QStringList linkedProps;
  linkedProps << "NewCellArrayName";
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Save as New Array", SaveAsNewArray, FilterParameter::Parameter, ItkImageMath, linkedProps));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt8, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Process", SelectedCellArrayPath, FilterParameter::RequiredArray, ItkImageMath, req));
  }
  {
    ChoiceFilterParameter::Pointer parameter = ChoiceFilterParameter::New();
    parameter->setHumanLabel("Operator");
    parameter->setPropertyName("Operator");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(ItkImageMath, this, Operator));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(ItkImageMath, this, Operator));

    QVector<QString> choices;
    choices.push_back("Add");
    choices.push_back("Subtract");
    choices.push_back("Multiply");
    choices.push_back("Divide");
    choices.push_back("Min");
    choices.push_back("Max");
    choices.push_back("Gamma");
    choices.push_back("Log");
    choices.push_back("Exp");
    choices.push_back("Square");
    choices.push_back("Square Root");
    choices.push_back("Invert");
    parameter->setChoices(choices);
    parameter->setCategory(FilterParameter::Parameter);
    parameters.push_back(parameter);
  }
  parameters.push_back(SIMPL_NEW_DOUBLE_FP("Value", Value, FilterParameter::Parameter, ItkImageMath));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Output Attribute Array", NewCellArrayName, SelectedCellArrayPath, SelectedCellArrayPath, FilterParameter::CreatedArray, ItkImageMath));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImageMath::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setOperator( reader->readValue( "Operator", getOperator() ) );
  setValue( reader->readValue( "Value", getValue() ) );
  setSaveAsNewArray( reader->readValue( "SaveAsNewArray", getSaveAsNewArray() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  reader->closeFilterGroup();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImageMath::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkImageMath::dataCheck()
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
void ItkImageMath::preflight()
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
void ItkImageMath::execute()
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

  //define filter types
  typedef itk::AddImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType, ImageProcessingConstants::FloatImageType> AddType;
  typedef itk::SubtractImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType, ImageProcessingConstants::FloatImageType> SubtractType;
  typedef itk::MultiplyImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType, ImageProcessingConstants::FloatImageType> MultiplyType;
  typedef itk::DivideImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType, ImageProcessingConstants::FloatImageType> DivideType;
  typedef itk::MinimumImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType, ImageProcessingConstants::FloatImageType> MinType;
  typedef itk::MaximumImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType, ImageProcessingConstants::FloatImageType> MaxType;
  typedef itk::BinaryFunctorImageFilter< ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType, ImageProcessingConstants::FloatImageType, ImageProcessing::Functor::Gamma<ImageProcessingConstants::FloatPixelType> > GammaType;
  typedef itk::LogImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType> LogType;
  typedef itk::ExpImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType> ExpType;
  typedef itk::SquareImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType> SquareType;
  typedef itk::SqrtImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::FloatImageType> SqrtType;
  typedef itk::InvertIntensityImageFilter<ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType> InvertType;

  //set up filter to cap image range + round
  typedef itk::UnaryFunctorImageFilter< ImageProcessingConstants::FloatImageType, ImageProcessingConstants::DefaultImageType, ImageProcessing::Functor::LimitsRound<ImageProcessingConstants::FloatPixelType, ImageProcessingConstants::DefaultPixelType> > LimitsRoundType;
  LimitsRoundType::Pointer limitsRound = LimitsRoundType::New();

  //apply selected operation
  switch(m_Operator)
  {
    case 0://add
    {
      AddType::Pointer add = AddType::New();
      add->SetInput1(inputImage);
      add->SetConstant2(m_Value);
      limitsRound->SetInput(add->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 1://subtract
    {
      SubtractType::Pointer subtract = SubtractType::New();
      subtract->SetInput1(inputImage);
      subtract->SetConstant2(m_Value);
      limitsRound->SetInput(subtract->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 2://multiply
    {
      MultiplyType::Pointer multiply = MultiplyType::New();
      multiply->SetInput1(inputImage);
      multiply->SetConstant2(m_Value);
      limitsRound->SetInput(multiply->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 3://divide
    {
      DivideType::Pointer divide = DivideType::New();
      divide->SetInput1(inputImage);
      divide->SetConstant2(m_Value);
      limitsRound->SetInput(divide->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 4://min
    {
      MinType::Pointer minimum = MinType::New();
      minimum->SetInput1(inputImage);
      minimum->SetConstant2(m_Value);
      limitsRound->SetInput(minimum->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 5://max
    {
      MaxType::Pointer maximum = MaxType::New();
      maximum->SetInput1(inputImage);
      maximum->SetConstant2(m_Value);
      limitsRound->SetInput(maximum->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 6://gamma
    {
      GammaType::Pointer gamma = GammaType::New();
      gamma->SetInput1(inputImage);
      gamma->SetConstant2(m_Value);
      limitsRound->SetInput(gamma->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 7://log
    {
      LogType::Pointer logfilter = LogType::New();
      logfilter->SetInput(inputImage);
      limitsRound->SetInput(logfilter->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 8://exp
    {
      ExpType::Pointer expfilter = ExpType::New();
      expfilter->SetInput(inputImage);
      limitsRound->SetInput(expfilter->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 9://square
    {
      SquareType::Pointer square = SquareType::New();
      square->SetInput(inputImage);
      limitsRound->SetInput(square->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 10://squareroot
    {
      SqrtType::Pointer sqrtfilter = SqrtType::New();
      sqrtfilter->SetInput(inputImage);
      limitsRound->SetInput(sqrtfilter->GetOutput());
      ITKUtilitiesType::SetITKFilterOutput(limitsRound->GetOutput(), m_NewCellArrayPtr.lock());
      limitsRound->Update();
    }
    break;

    case 11://invert
    {
      InvertType::Pointer invert = InvertType::New();
      invert->SetInput(inputImage);
      ITKUtilitiesType::SetITKFilterOutput(invert->GetOutput(), m_NewCellArrayPtr.lock());
      invert->Update();
    }
    break;
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
AbstractFilter::Pointer ItkImageMath::newFilterInstance(bool copyFilterParameters) const
{
  ItkImageMath::Pointer filter = ItkImageMath::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkImageMath::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkImageMath::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QUuid ItkImageMath::getUuid()
{
  return QUuid("{57b7367b-c4f1-56e0-b47f-e61418479b03}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkImageMath::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkImageMath::getHumanLabel() const
{ return "Image Math (ImageProcessing)"; }

