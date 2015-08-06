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
#include "ItkKMeans.h"

#include "itkScalarImageKmeansImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"

#include "DREAM3DLib/Common/Constants.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersWriter.h"
#include "DREAM3DLib/FilterParameters/IntFilterParameter.h"
#include "DREAM3DLib/FilterParameters/BooleanFilterParameter.h"
#include "DREAM3DLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "DREAM3DLib/FilterParameters/StringFilterParameter.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "DREAM3DLib/FilterParameters/SeparatorFilterParameter.h"

#include "ItkBridge.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkKMeans::ItkKMeans() :
  AbstractFilter(),
  m_SelectedCellArrayPath("", "", ""),
  m_NewCellArrayName(""),
  m_SaveAsNewArray(true),
  m_Slice(false),
  m_Classes(2),
  m_SelectedCellArray(NULL),
  m_NewCellArray(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkKMeans::~ItkKMeans()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKMeans::setupFilterParameters()
{
  FilterParameterVector parameters;

  parameters.push_back(IntFilterParameter::New("Number of Classes", "Classes", getClasses(), FilterParameter::Parameter));
  parameters.push_back(BooleanFilterParameter::New("Slice at a Time", "Slice", getSlice(), FilterParameter::Parameter));
  QStringList linkedProps;
  linkedProps << "NewCellArrayName";
  parameters.push_back(LinkedBooleanFilterParameter::New("Save as New Array", "SaveAsNewArray", getSaveAsNewArray(), linkedProps, FilterParameter::Parameter));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  parameters.push_back(DataArraySelectionFilterParameter::New("Attribute Array to Cluster", "SelectedCellArrayPath", getSelectedCellArrayPath(), FilterParameter::RequiredArray));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(StringFilterParameter::New("Clustered Array", "NewCellArrayName", getNewCellArrayName(), FilterParameter::CreatedArray));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKMeans::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  setSaveAsNewArray( reader->readValue( "SaveAsNewArray", getSaveAsNewArray() ) );
  setSlice( reader->readValue( "Slice", getSlice() ) );
  setClasses( reader->readValue( "Classes", getClasses() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ItkKMeans::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(SelectedCellArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(NewCellArrayName)
  DREAM3D_FILTER_WRITE_PARAMETER(SaveAsNewArray)
  DREAM3D_FILTER_WRITE_PARAMETER(Slice)
  DREAM3D_FILTER_WRITE_PARAMETER(Classes)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKMeans::dataCheck()
{
  setErrorCondition(0);
  DataArrayPath tempPath;

  QVector<size_t> dims(1, 1);
  m_SelectedCellArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter>(this, getSelectedCellArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_SelectedCellArrayPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_SelectedCellArray = m_SelectedCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCondition() < 0) { return; }

  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCondition() < 0 || NULL == image.get()) { return; }

  if(m_SaveAsNewArray == false) { m_NewCellArrayName = "thisIsATempName"; }
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter, ImageProcessingConstants::DefaultPixelType>(this, tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_NewCellArrayPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  if(m_Classes < 2)
  {
    QString ss = QObject::tr("Must threshold into at least 2 classes");
    setErrorCondition(-1000);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkKMeans::preflight()
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
void ItkKMeans::execute()
{
  //int err = 0;
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath().getAttributeMatrixName();

  /* Place all your code to execute your filter here. */
  //get dims
  size_t udims[3] = {0, 0, 0};
  m->getGeometryAs<ImageGeom>()->getDimensions(udims);
#if (CMP_SIZEOF_SIZE_T == 4)
  typedef int32_t DimType;
#else
  typedef int64_t DimType;
#endif
  DimType dims[3] =
  {
    static_cast<DimType>(udims[0]),
    static_cast<DimType>(udims[1]),
    static_cast<DimType>(udims[2]),
  };

  //wrap input as itk image
  ImageProcessingConstants::DefaultImageType::Pointer inputImage = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_SelectedCellArray);

  if(m_Slice)
  {
    //define filters
    typedef itk::MinimumMaximumImageCalculator< ImageProcessingConstants::DefaultSliceType > CalculatorType;
    typedef itk::ScalarImageKmeansImageFilter< ImageProcessingConstants::DefaultSliceType, ImageProcessingConstants::DefaultSliceType > KMeansType;

    //wrap output buffer as image
    ImageProcessingConstants::DefaultImageType::Pointer outputImage = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_NewCellArray);

    //loop over slices
    for(int i = 0; i < dims[2]; i++)
    {
      //get slice
      ImageProcessingConstants::DefaultSliceType::Pointer slice = ITKUtilitiesType::ExtractSlice(inputImage, ImageProcessingConstants::ZSlice, i);

      //find max/min
      CalculatorType::Pointer minMaxFilter = CalculatorType::New ();
      minMaxFilter->SetImage(slice);
      minMaxFilter->Compute();
      ImageProcessingConstants::DefaultPixelType range = minMaxFilter->GetMaximum() - minMaxFilter->GetMinimum();

      //set up kmeans filter
      KMeansType::Pointer kMeans = KMeansType::New();
      kMeans->SetInput(slice);
      ImageProcessingConstants::DefaultPixelType meanIncrement = range / m_Classes;
      ImageProcessingConstants::DefaultPixelType mean = range / (2 * m_Classes);
      for(int j = 0; j < m_Classes; j++)
      {
        kMeans->AddClassWithInitialMean(mean);
        mean = mean + meanIncrement;
      }

      try
      {
        kMeans->Update();
      }
      catch( itk::ExceptionObject& err )
      {
        setErrorCondition(-5);
        QString ss = QObject::tr("Failed to execute itk::KMeans filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
        notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
      }

      //copy back into volume
      ITKUtilitiesType::SetSlice(outputImage, kMeans->GetOutput(), ImageProcessingConstants::ZSlice, i);
    }
  }
  else
  {
    //find min+max of image
    typedef itk::MinimumMaximumImageCalculator< ImageProcessingConstants::DefaultImageType > CalculatorType;
    CalculatorType::Pointer minMaxFilter = CalculatorType::New ();
    minMaxFilter->SetImage(inputImage);
    minMaxFilter->Compute();
    ImageProcessingConstants::DefaultPixelType range = minMaxFilter->GetMaximum() - minMaxFilter->GetMinimum();

    //set up kmeans filter
    typedef itk::ScalarImageKmeansImageFilter< ImageProcessingConstants::DefaultImageType, ImageProcessingConstants::DefaultImageType > KMeansType;
    KMeansType::Pointer kMeans = KMeansType::New();
    kMeans->SetInput(inputImage);

    //start with evenly spaced class means
    ImageProcessingConstants::DefaultPixelType meanIncrement = range / m_Classes;
    ImageProcessingConstants::DefaultPixelType mean = range / (2 * m_Classes);
    for(int i = 0; i < m_Classes; i++)
    {
      kMeans->AddClassWithInitialMean(mean);
      mean = mean + meanIncrement;
    }

    ITKUtilitiesType::SetITKFilterOutput(kMeans->GetOutput(), m_NewCellArrayPtr.lock());
    try
    {
      kMeans->Update();
    }
    catch( itk::ExceptionObject& err )
    {
      setErrorCondition(-5);
      QString ss = QObject::tr("Failed to execute itk::KMeans filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
      notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    }
  }


  //array name changing/cleanup
  if(m_SaveAsNewArray == false)
  {
    AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(m_SelectedCellArrayPath.getAttributeMatrixName());
    attrMat->removeAttributeArray(m_SelectedCellArrayPath.getDataArrayName());
    attrMat->renameAttributeArray(m_NewCellArrayName, m_SelectedCellArrayPath.getDataArrayName());
  }



  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkKMeans::newFilterInstance(bool copyFilterParameters)
{
  /*
  * write code to optionally copy the filter parameters from the current filter into the new instance
  */
  ItkKMeans::Pointer filter = ItkKMeans::New();
  if(true == copyFilterParameters)
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
    //    DREAM3D_COPY_INSTANCEVAR(OutputFile)
    //    DREAM3D_COPY_INSTANCEVAR(ZStartIndex)
    //    DREAM3D_COPY_INSTANCEVAR(ZEndIndex)
    //    DREAM3D_COPY_INSTANCEVAR(ZResolution)
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKMeans::getCompiledLibraryName()
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKMeans::getGroupName()
{return DREAM3D::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKMeans::getSubGroupName()
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkKMeans::getHumanLabel()
{return "ITK: K Means";}

