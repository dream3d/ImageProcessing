/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "ItkKdTreeKMeans.h"

#include "itkScalarImageKmeansImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkImageKmeansModelEstimator.h"

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/TemplateHelpers.hpp"
#include "SIMPLib/FilterParameters/IntFilterParameter.h"
#include "SIMPLib/FilterParameters/BooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "ItkBridge.h"

/**
 * @brief The itkKdTreeKMeansTemplate class is a templated wrapper for the itkKdTreeBasedKmeansEstimator class
 */
template<typename DataType>
class itkKdTreeKMeansTemplate
{
  public:
    typedef DataArray<DataType> DataArrayType;

    itkKdTreeKMeansTemplate() {}
    virtual ~itkKdTreeKMeansTemplate() {}

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    bool operator()(IDataArray::Pointer p)
    {
      return (std::dynamic_pointer_cast<DataArrayType>(p).get() != nullptr);
    }

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    void Execute(IDataArray::Pointer inputIDataArray, Int32ArrayType::Pointer classLabelsArray, int32_t numClasses)
    {
      typename DataArrayType::Pointer inputDataPtr = std::dynamic_pointer_cast<DataArrayType>(inputIDataArray);

      DataType* inputData = inputDataPtr->getPointer(0);
      int32_t* classLabels = classLabelsArray->getPointer(0);
      size_t numTuples = inputDataPtr->getNumberOfTuples();

      typedef itk::Vector<DataType, 3> MeasurementVectorType;
      typedef itk::Statistics::ListSample< MeasurementVectorType > SampleType;
      typename SampleType::Pointer sample = SampleType::New();
      sample->SetMeasurementVectorSize(3);

      MeasurementVectorType mv;

      for (size_t i = 0; i < numTuples; i++)
      {
        mv[0] = inputData[3 * i + 0];
        mv[1] = inputData[3 * i + 1];
        mv[2] = inputData[3 * i + 2];
        sample->PushBack(mv);
      }

      typedef itk::Statistics::KdTreeGenerator<SampleType> TreeGeneratorType;
      typename TreeGeneratorType::Pointer treeGenerator = TreeGeneratorType::New();

      treeGenerator->SetSample(sample);
      treeGenerator->SetBucketSize(numTuples);
      treeGenerator->Update();

      typedef typename TreeGeneratorType::KdTreeType TreeType;
      typedef itk::Statistics::KdTreeBasedKmeansEstimator<TreeType> EstimatorType;
      typename EstimatorType::Pointer estimator = EstimatorType::New();

      typename EstimatorType::ParametersType initialMeans(numClasses * 3);
      initialMeans.Fill(0.0);

      estimator->SetParameters(initialMeans);
      estimator->SetKdTree(treeGenerator->GetOutput());
      estimator->SetMaximumIteration(1000);
      estimator->SetCentroidPositionChangesThreshold(0.0);
      estimator->StartOptimization();

      typedef itk::Statistics::SampleClassifierFilter<SampleType> ClassifierType;
      typename ClassifierType::Pointer classifier = ClassifierType::New();

      typedef itk::Statistics::MinimumDecisionRule DecisionRuleType;
      DecisionRuleType::Pointer decisionRule = DecisionRuleType::New();

      classifier->SetDecisionRule(decisionRule);
      classifier->SetNumberOfClasses(numClasses);

      typedef typename ClassifierType::ClassLabelVectorObjectType ClassLabelVectorObjectType;
      typedef typename ClassifierType::ClassLabelVectorType ClassLabelVectorType;
//      typedef typename ClassifierType::MembershipFunctionVectorObjectType MembershipFunctionVectorObjectType;
//      typedef typename ClassifierType::MembershipFunctionVectorType MembershipFunctionVectorType;
      //typedef itk::Statistics::DistanceToCentroidMembershipFunction<MeasurementVectorType> MembershipFunctionType;
      //typedef typename MembershipFunctionType::Pointer MembershipFunctionPointer;
//      typedef std::vector<MembershipFunctionPointer> MembershipFunctionVectorPointer;

      const typename EstimatorType::MembershipFunctionVectorObjectType* kMeansMembershipFunctions = estimator->GetOutput();
      classifier->SetMembershipFunctions(kMeansMembershipFunctions);

      typename ClassLabelVectorObjectType::Pointer classLabelsObject = ClassLabelVectorObjectType::New();
      ClassLabelVectorType& classLabelsVector = classLabelsObject->Get();

      for (size_t i = 0; i < numClasses; i++)
      {
        classLabelsVector.push_back(i + 1);
      }

      classifier->SetClassLabels(classLabelsObject);
      classifier->SetInput(sample);
      classifier->Update();

      const typename ClassifierType::MembershipSampleType* membershipSample = classifier->GetOutput();
      typename ClassifierType::MembershipSampleType::ConstIterator membershipIterator = membershipSample->Begin();

      int32_t index = 0;

      while (membershipIterator != membershipSample->End())
      {
        classLabels[index] = membershipIterator.GetClassLabel();
        ++index;
        ++membershipIterator;
      }
    }
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkKdTreeKMeans::ItkKdTreeKMeans() :
  AbstractFilter(),
  m_SelectedCellArrayPath("", "", ""),
  m_NewCellArrayName("ClassLabels"),
  m_Classes(2),
  m_NewCellArray(nullptr)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkKdTreeKMeans::~ItkKdTreeKMeans() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKdTreeKMeans::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Number of Classes", Classes, FilterParameter::Parameter, ItkKdTreeKMeans));
  DataArraySelectionFilterParameter::RequirementType req;
  parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Classify", SelectedCellArrayPath, FilterParameter::RequiredArray, ItkKdTreeKMeans, req));
  parameters.push_back(SIMPL_NEW_STRING_FP("Class Labels", NewCellArrayName, FilterParameter::CreatedArray, ItkKdTreeKMeans));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKdTreeKMeans::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  setClasses( reader->readValue( "Classes", getClasses() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKdTreeKMeans::initialize()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKdTreeKMeans::dataCheck()
{
  setErrorCondition(0);
  setWarningCondition(0);

  getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom, AbstractFilter>(this, getSelectedCellArrayPath().getDataContainerName());

  if (getClasses() < 2)
  {
    setErrorCondition(-5555);
    notifyErrorMessage(getHumanLabel(), "Must have at least 2 classes", getErrorCondition());
  }

  m_SelectedCellArrayPtr = getDataContainerArray()->getPrereqIDataArrayFromPath<IDataArray, AbstractFilter>(this, getSelectedCellArrayPath()); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if (getErrorCondition() < 0) { return; }

  int32_t numComps = m_SelectedCellArrayPtr.lock()->getNumberOfComponents();

  if (numComps != 3)
  {
    setErrorCondition(-5555);
    QString ss = QObject::tr("Input data has total components %1, but the data must be 3-vectors").arg(numComps);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  QVector<size_t> cDims(1, 1);
  DataArrayPath tempPath(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName());

  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter, int32_t>(this, tempPath, 0, cDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if(nullptr != m_NewCellArrayPtr.lock())                       /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKdTreeKMeans::preflight()
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
void ItkKdTreeKMeans::execute()
{
  setErrorCondition(0);
  setWarningCondition(0);
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  EXECUTE_TEMPLATE(this, itkKdTreeKMeansTemplate, m_SelectedCellArrayPtr.lock(), m_SelectedCellArrayPtr.lock(), m_NewCellArrayPtr.lock(), m_Classes)

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkKdTreeKMeans::newFilterInstance(bool copyFilterParameters) const
{
  ItkKdTreeKMeans::Pointer filter = ItkKdTreeKMeans::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKdTreeKMeans::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKdTreeKMeans::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QUuid ItkKdTreeKMeans::getUuid()
{
  return QUuid("{68d2b4e5-7325-5c9b-b3e0-26c726e8fd6f}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKdTreeKMeans::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKdTreeKMeans::getHumanLabel() const
{ return "K-d Tree K Means (ImageProcessing)"; }

