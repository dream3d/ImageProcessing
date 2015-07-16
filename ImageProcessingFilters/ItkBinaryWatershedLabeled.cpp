/* ============================================================================
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "ItkBinaryWatershedLabeled.h"

//thresholding filter
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMorphologicalWatershedFromMarkersImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"


#include "DREAM3DLib/FilterParameters/AbstractFilterParametersWriter.h"
#include "DREAM3DLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "DREAM3DLib/FilterParameters/DoubleFilterParameter.h"
#include "DREAM3DLib/FilterParameters/StringFilterParameter.h"
#include "DREAM3DLib/FilterParameters/AbstractFilterParametersReader.h"
#include "DREAM3DLib/FilterParameters/SeparatorFilterParameter.h"



// ImageProcessing Plugin
#include "ItkBridge.h"
#include "ImageProcessing/ImageProcessingHelpers.hpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkBinaryWatershedLabeled::ItkBinaryWatershedLabeled() :
  AbstractFilter(),
  m_SelectedCellArrayPath("", "", ""),
  m_PeakTolerance(1.0),
  m_NewCellArrayName("BinaryWatershedLabeled"),
  m_SelectedCellArray(NULL),
  m_NewCellArray(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkBinaryWatershedLabeled::~ItkBinaryWatershedLabeled()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::DataStructureRequirements req;
    req.daTypes = QVector<QString>(1, DREAM3D::TypeNames::Bool);
    req.componentDimensions = QVector< QVector<size_t> >(1, QVector<size_t>(1, 1));
    parameters.push_back(DataArraySelectionFilterParameter::New("Attribute Array to Watershed", "SelectedCellArrayPath", getSelectedCellArrayPath(), FilterParameter::RequiredArray, req));
  }
  parameters.push_back(DoubleFilterParameter::New("Peak Noise Tolerance", "PeakTolerance", getPeakTolerance(), FilterParameter::Parameter));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(StringFilterParameter::New("Watershed Array", "NewCellArrayName", getNewCellArrayName(), FilterParameter::CreatedArray));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setPeakTolerance( reader->readValue( "PeakTolerance", getPeakTolerance() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int ItkBinaryWatershedLabeled::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(SelectedCellArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(PeakTolerance)
  DREAM3D_FILTER_WRITE_PARAMETER(NewCellArrayName)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::dataCheck()
{
  setErrorCondition(0);
  DataArrayPath tempPath;

  QVector<size_t> dims(1, 1);
  m_SelectedCellArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>, AbstractFilter>(this, getSelectedCellArrayPath(), dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_SelectedCellArrayPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_SelectedCellArray = m_SelectedCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCondition() < 0) { return; }

  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCondition() < 0 || NULL == image.get()) { return; }

  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint32_t>, AbstractFilter, ImageProcessingConstants::DefaultPixelType>(this, tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_NewCellArrayPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::preflight()
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
void ItkBinaryWatershedLabeled::execute()
{
  QString ss;
  dataCheck();
  if(getErrorCondition() < 0)
  {
    setErrorCondition(-10000);
    ss = QObject::tr("DataCheck did not pass during execute");
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath().getAttributeMatrixName();

  //get utilities
  typedef ItkBridge<bool> BoolBridgeType;
  typedef ItkBridge<float> FloatBridgeType;
  typedef ItkBridge<uint32_t> LabelBridgeType;

  //wrap input
  BoolBridgeType::ScalarImageType::Pointer inputImage = BoolBridgeType::CreateItkWrapperForDataPointer(m, attrMatName, m_SelectedCellArray);

  //compute distance map
  typedef itk::SignedMaurerDistanceMapImageFilter<BoolBridgeType::ScalarImageType, FloatBridgeType::ScalarImageType> DistanceMapType;
  DistanceMapType::Pointer distanceMap = DistanceMapType::New();
  distanceMap->SetInsideIsPositive(true);
  distanceMap->SetInput(inputImage);
  try
  {
    distanceMap->Update();
  }
  catch( itk::ExceptionObject& err )
  {
    setErrorCondition(-5);
    QString ss = QObject::tr("Failed to execute itk::KMeans filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  //find maxima in distance map (ultimate points)
  std::vector<FloatBridgeType::ScalarImageType::IndexType> peakLocations = ImageProcessing::LocalMaxima<FloatBridgeType::ScalarImageType>::Find(distanceMap->GetOutput(), m_PeakTolerance, true);

  //create labeled image from peaks
  typedef itk::Image<uint32_t, FloatBridgeType::ScalarImageType::ImageDimension> LabelImageType;
  LabelImageType::Pointer seedLabels = LabelImageType::New();
  LabelImageType::RegionType region = inputImage->GetLargestPossibleRegion();
  seedLabels->SetRegions(region);
  seedLabels->Allocate();
  seedLabels->FillBuffer(0);
  for(size_t i = 0; i < peakLocations.size(); i++)
  {
    seedLabels->SetPixel(peakLocations[i], i + 1);
  }
  seedLabels->CopyInformation(inputImage);

  //invert distance map as gradient for watershed
  typedef itk::InvertIntensityImageFilter< FloatBridgeType::ScalarImageType, FloatBridgeType::ScalarImageType > InvertType;
  InvertType::Pointer invert = InvertType::New();
  invert->SetInput(distanceMap->GetOutput());
  invert->SetMaximum(0);

  //set up seeded watershed
  typedef itk::MorphologicalWatershedFromMarkersImageFilter< FloatBridgeType::ScalarImageType, LabelImageType > WatershedType;
  WatershedType::Pointer watershed = WatershedType::New();
  watershed->SetInput1(invert->GetOutput());
  watershed->SetInput2(seedLabels);
  watershed->SetMarkWatershedLine(false);

  //mask watershed output (we only want things that were originally in the)
  typedef itk::MaskImageFilter<LabelImageType, BoolBridgeType::ScalarImageType, LabelBridgeType::ScalarImageType> MaskType;
  MaskType::Pointer mask = MaskType::New();
  mask->SetInput(watershed->GetOutput());
  mask->SetMaskImage(inputImage);

//  //threshold all labels into boolean array
//  typedef itk::BinaryThresholdImageFilter< LabelImageType, BoolBridgeType::ScalarImageType > ThresholdType;
//  ThresholdType::Pointer threshold = ThresholdType::New();
//  threshold->SetInput(mask->GetOutput());
//  threshold->SetLowerThreshold(1);
//  threshold->SetInsideValue(true);
//  threshold->SetOutsideValue(false);

  //wrap output
  LabelBridgeType::SetITKFilterOutput(mask->GetOutput(), m_NewCellArrayPtr.lock());
  try
  {
    mask->Update();
  }
  catch( itk::ExceptionObject& err )
  {
    setErrorCondition(-5);
    QString ss = QObject::tr("Failed to execute itk::KMeans filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkBinaryWatershedLabeled::newFilterInstance(bool copyFilterParameters)
{
  ItkBinaryWatershedLabeled::Pointer filter = ItkBinaryWatershedLabeled::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkBinaryWatershedLabeled::getCompiledLibraryName()
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkBinaryWatershedLabeled::getGroupName()
{return DREAM3D::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkBinaryWatershedLabeled::getSubGroupName()
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkBinaryWatershedLabeled::getHumanLabel()
{return "ITK: Binary Watershed Labeled";}

