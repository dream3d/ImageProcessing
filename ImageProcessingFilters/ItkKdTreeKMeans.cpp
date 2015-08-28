/* ============================================================================
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
 * Neither the names of any of the DREAM3D Consortium contributors
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
#include "ItkKdTreeKMeans.h"

#include "itkScalarImageKmeansImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkImageKmeansModelEstimator.h"

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/Common/TemplateHelpers.hpp"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersWriter.h"
#include "DREAM3DLib/FilterParameters/IntFilterParameter.h"
#include "DREAM3DLib/FilterParameters/BooleanFilterParameter.h"
#include "DREAM3DLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "DREAM3DLib/FilterParameters/StringFilterParameter.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "DREAM3DLib/FilterParameters/SeparatorFilterParameter.h"

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
      return (boost::dynamic_pointer_cast<DataArrayType>(p).get() != NULL);
    }

    // -----------------------------------------------------------------------------
    //
    // -----------------------------------------------------------------------------
    void Execute(IDataArray::Pointer inputIDataArray, Int32ArrayType::Pointer classLabelsArray, int32_t numClasses)
    {
      typename DataArrayType::Pointer inputDataPtr = boost::dynamic_pointer_cast<DataArrayType>(inputIDataArray);

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
      typedef typename ClassifierType::MembershipFunctionVectorObjectType MembershipFunctionVectorObjectType;
      typedef typename ClassifierType::MembershipFunctionVectorType MembershipFunctionVectorType;
      typedef itk::Statistics::DistanceToCentroidMembershipFunction<MeasurementVectorType> MembershipFunctionType;
      typedef typename MembershipFunctionType::Pointer MembershipFunctionPointer;
      typedef std::vector<MembershipFunctionPointer> MembershipFunctionVectorPointer;

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
  m_SelectedCellArray(NULL),
  m_NewCellArray(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkKdTreeKMeans::~ItkKdTreeKMeans()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKdTreeKMeans::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(IntFilterParameter::New("Number of Classes", "Classes", getClasses(), FilterParameter::Parameter));
  DataArraySelectionFilterParameter::RequirementType req;
  parameters.push_back(DataArraySelectionFilterParameter::New("Attribute Array to Classify", "SelectedCellArrayPath", getSelectedCellArrayPath(), FilterParameter::RequiredArray, req));
  parameters.push_back(StringFilterParameter::New("Class Labels", "NewCellArrayName", getNewCellArrayName(), FilterParameter::CreatedArray));
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
int ItkKdTreeKMeans::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(SelectedCellArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(NewCellArrayName)
  DREAM3D_FILTER_WRITE_PARAMETER(Classes)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKdTreeKMeans::dataCheck()
{
  setErrorCondition(0);

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
  if( NULL != m_NewCellArrayPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
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
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  EXECUTE_TEMPLATE(this, itkKdTreeKMeansTemplate, m_SelectedCellArrayPtr.lock(), m_SelectedCellArrayPtr.lock(), m_NewCellArrayPtr.lock(), m_Classes)

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkKdTreeKMeans::newFilterInstance(bool copyFilterParameters)
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
const QString ItkKdTreeKMeans::getCompiledLibraryName()
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKdTreeKMeans::getGroupName()
{return DREAM3D::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKdTreeKMeans::getSubGroupName()
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKdTreeKMeans::getHumanLabel()
{return "ITK: K-d Tree K Means";}

