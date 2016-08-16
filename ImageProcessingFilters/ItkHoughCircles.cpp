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
#include "ItkHoughCircles.h"

#include "itkHoughTransform2DCirclesImageFilter.h"

#include <QtCore/QString>

#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/DoubleFilterParameter.h"
#include "SIMPLib/FilterParameters/IntFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "ItkBridge.h"

// Include the MOC generated file for this class
#include "moc_ItkHoughCircles.cpp"



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkHoughCircles::ItkHoughCircles() :
  AbstractFilter(),
  m_SelectedCellArrayPath("", "", ""),
  m_NewCellArrayName(""),
  m_SaveAsNewArray(true),
  m_MinRadius(0),
  m_MaxRadius(0),
  m_NumberCircles(0),
  m_SelectedCellArray(NULL),
  m_NewCellArray(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkHoughCircles::~ItkHoughCircles()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkHoughCircles::setupFilterParameters()
{
  FilterParameterVector parameters;

  QStringList linkedProps;
  linkedProps << "NewCellArrayName";
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Save as New Array", SaveAsNewArray, FilterParameter::Parameter, ItkHoughCircles, linkedProps));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Int8, 1, SIMPL::AttributeMatrixObjectType::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Process", SelectedCellArrayPath, FilterParameter::RequiredArray, ItkHoughCircles, req));
  }
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_STRING_FP("Output Attribute Array", NewCellArrayName, FilterParameter::CreatedArray, ItkHoughCircles));
  parameters.push_back(SIMPL_NEW_DOUBLE_FP("Minimum Radius", MinRadius, FilterParameter::Parameter, ItkHoughCircles));
  parameters.push_back(SIMPL_NEW_DOUBLE_FP("Maximum Radius", MaxRadius, FilterParameter::Parameter, ItkHoughCircles));
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Number of Circles", NumberCircles, FilterParameter::Parameter, ItkHoughCircles));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkHoughCircles::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  setSaveAsNewArray( reader->readValue( "SaveAsNewArray", getSaveAsNewArray() ) );
  setMinRadius( reader->readValue( "MinRadius", getMinRadius() ) );
  setMaxRadius( reader->readValue( "MaxRadius", getMaxRadius() ) );
  setNumberCircles( reader->readValue( "NumberCircles", getNumberCircles() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkHoughCircles::initialize()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkHoughCircles::dataCheck()
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
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkHoughCircles::preflight()
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
void ItkHoughCircles::execute()
{
  //int err = 0;
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath().getAttributeMatrixName();

  /* Place all your code to execute your filter here. */
  //get dimensions
  size_t udims[3] = {0, 0, 0};
  m->getGeometryAs<ImageGeom>()->getDimensions(udims);

  int64_t dims[3] =
  {
    static_cast<int64_t>(udims[0]),
    static_cast<int64_t>(udims[1]),
    static_cast<int64_t>(udims[2]),
  };

  size_t totalPoints = m_SelectedCellArrayPtr.lock()->getNumberOfTuples();
  for(int i = 0; i < totalPoints; ++i)
  {
    m_NewCellArray[i] = m_SelectedCellArray[i];
  }

  //wrap raw and processed image data as itk::images
  ImageProcessingConstants::DefaultImageType::Pointer inputImage = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_SelectedCellArray);
  ImageProcessingConstants::DefaultImageType::Pointer outputImage = ITKUtilitiesType::CreateItkWrapperForDataPointer(m, attrMatName, m_NewCellArray);

  ImageProcessingConstants::DefaultSliceType::IndexType localIndex;
  typedef itk::HoughTransform2DCirclesImageFilter<ImageProcessingConstants::DefaultPixelType, ImageProcessingConstants::FloatPixelType> HoughTransformFilterType;
  HoughTransformFilterType::Pointer houghFilter = HoughTransformFilterType::New();
  houghFilter->SetNumberOfCircles( m_NumberCircles );
  houghFilter->SetMinimumRadius( m_MinRadius );
  houghFilter->SetMaximumRadius( m_MaxRadius );
  /*optional parameters, these are the default values
  houghFilter->SetSweepAngle( 0 );
  houghFilter->SetSigmaGradient( 1 );
  houghFilter->SetVariance( 5 );
  houghFilter->SetDiscRadiusRatio( 10 );
  */

  //loop over slices
  for(int i = 0; i < dims[2]; ++i)
  {
    //extract slice and transform
    QString ss = QObject::tr("Hough Transforming Slice: %1").arg(i + 1);
    notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
    ImageProcessingConstants::DefaultSliceType::Pointer inputSlice = ITKUtilitiesType::ExtractSlice(inputImage, ImageProcessingConstants::ZSlice, i);
    houghFilter->SetInput( inputSlice );
    houghFilter->Update();
    ImageProcessingConstants::FloatSliceType::Pointer localAccumulator = houghFilter->GetOutput();

    //find circles
    ss = QObject::tr("Finding Circles on Slice: %1").arg(i + 1);
    notifyStatusMessage(getMessagePrefix(), getHumanLabel(), ss);
    HoughTransformFilterType::CirclesListType circles = houghFilter->GetCircles( m_NumberCircles );

    //create blank slice of same dimensions
    ImageProcessingConstants::DefaultSliceType::Pointer outputSlice = ImageProcessingConstants::DefaultSliceType::New();
    ImageProcessingConstants::DefaultSliceType::RegionType region;
    region.SetSize(inputSlice->GetLargestPossibleRegion().GetSize());
    region.SetIndex(inputSlice->GetLargestPossibleRegion().GetIndex());
    outputSlice->SetRegions( region );
    outputSlice->SetOrigin(inputSlice->GetOrigin());
    outputSlice->SetSpacing(inputSlice->GetSpacing());
    outputSlice->Allocate();
    outputSlice->FillBuffer(0);

    //loop over circles drawing on slice
    HoughTransformFilterType::CirclesListType::const_iterator itCircles = circles.begin();
    while( itCircles != circles.end() )
    {
      //std::cout << "Center: ";
      //std::cout << (*itCircles)->GetObjectToParentTransform()->GetOffset() << std::endl;
      //std::cout << "Radius: " << (*itCircles)->GetRadius()[0] << std::endl;

      for(double angle = 0; angle <= 2 * vnl_math::pi; angle += vnl_math::pi / 60.0 )
      {
        localIndex[0] = (long int)((*itCircles)->GetObjectToParentTransform()->GetOffset()[0]
                                   + (*itCircles)->GetRadius()[0] * vcl_cos(angle));
        localIndex[1] = (long int)((*itCircles)->GetObjectToParentTransform()->GetOffset()[1]
                                   + (*itCircles)->GetRadius()[0] * vcl_sin(angle));
        ImageProcessingConstants::DefaultSliceType::RegionType outputRegion = outputSlice->GetLargestPossibleRegion();
        if( outputRegion.IsInside( localIndex ) )
        {
          outputSlice->SetPixel( localIndex, 255 );
        }
      }
      itCircles++;
    }

    //copy slice into output
    ITKUtilitiesType::SetSlice(outputImage, outputSlice, ImageProcessingConstants::ZSlice, i);
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
AbstractFilter::Pointer ItkHoughCircles::newFilterInstance(bool copyFilterParameters)
{
  ItkHoughCircles::Pointer filter = ItkHoughCircles::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkHoughCircles::getCompiledLibraryName()
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkHoughCircles::getGroupName()
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkHoughCircles::getSubGroupName()
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkHoughCircles::getHumanLabel()
{return "ITK: Hough Circle Detection";}

