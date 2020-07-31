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

#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/ITK/itkBridge.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"

// ImageProcessing Plugin
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
ItkBinaryWatershedLabeled::ItkBinaryWatershedLabeled()
: m_SelectedCellArrayPath("", "", "")
, m_PeakTolerance(1.0)
, m_NewCellArrayName("BinaryWatershedLabeled")
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkBinaryWatershedLabeled::~ItkBinaryWatershedLabeled() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Bool, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Watershed", SelectedCellArrayPath, FilterParameter::RequiredArray, ItkBinaryWatershedLabeled, req));
  }
  parameters.push_back(SIMPL_NEW_FLOAT_FP("Peak Noise Tolerance", PeakTolerance, FilterParameter::Parameter, ItkBinaryWatershedLabeled));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Watershed Array", NewCellArrayName, SelectedCellArrayPath, SelectedCellArrayPath, FilterParameter::CreatedArray, ItkBinaryWatershedLabeled));
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
void ItkBinaryWatershedLabeled::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  std::vector<size_t> dims(1, 1);
  m_SelectedCellArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<bool>>(this, getSelectedCellArrayPath(), dims);
  if(nullptr != m_SelectedCellArrayPtr.lock())
  { m_SelectedCellArray = m_SelectedCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
  if(getErrorCode() < 0)
  {
    return;
  }

  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName())->getPrereqGeometry<ImageGeom>(this);
  if(getErrorCode() < 0 || nullptr == image.get())
  {
    return;
  }

  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr =
      getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<uint32_t>>(this, tempPath, 0, dims, "", DataArrayID31);
  if(nullptr != m_NewCellArrayPtr.lock())
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::execute()
{
  QString ss;
  dataCheck();
  if(getErrorCode() < 0)
  {
    ss = QObject::tr("DataCheck did not pass during execute");
    setErrorCondition(-11000, ss);
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
    QString ss = QObject::tr("Failed to execute itk::KMeans filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
    setErrorCondition(-5, ss);
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
    QString ss = QObject::tr("Failed to execute itk::KMeans filter. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
    setErrorCondition(-5, ss);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkBinaryWatershedLabeled::newFilterInstance(bool copyFilterParameters) const
{
  ItkBinaryWatershedLabeled::Pointer filter = ItkBinaryWatershedLabeled::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkBinaryWatershedLabeled::getCompiledLibraryName() const
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkBinaryWatershedLabeled::getGroupName() const
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid ItkBinaryWatershedLabeled::getUuid() const
{
  return QUuid("{76fd1b13-5feb-5338-8d7f-b3b935ff3f22}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkBinaryWatershedLabeled::getSubGroupName() const
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString ItkBinaryWatershedLabeled::getHumanLabel() const
{ return "Binary Watershed Labeled (ImageProcessing)"; }

// -----------------------------------------------------------------------------
ItkBinaryWatershedLabeled::Pointer ItkBinaryWatershedLabeled::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<ItkBinaryWatershedLabeled> ItkBinaryWatershedLabeled::New()
{
  struct make_shared_enabler : public ItkBinaryWatershedLabeled
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString ItkBinaryWatershedLabeled::getNameOfClass() const
{
  return QString("ItkBinaryWatershedLabeled");
}

// -----------------------------------------------------------------------------
QString ItkBinaryWatershedLabeled::ClassName()
{
  return QString("ItkBinaryWatershedLabeled");
}

// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::setSelectedCellArrayPath(const DataArrayPath& value)
{
  m_SelectedCellArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath ItkBinaryWatershedLabeled::getSelectedCellArrayPath() const
{
  return m_SelectedCellArrayPath;
}

// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::setPeakTolerance(float value)
{
  m_PeakTolerance = value;
}

// -----------------------------------------------------------------------------
float ItkBinaryWatershedLabeled::getPeakTolerance() const
{
  return m_PeakTolerance;
}

// -----------------------------------------------------------------------------
void ItkBinaryWatershedLabeled::setNewCellArrayName(const QString& value)
{
  m_NewCellArrayName = value;
}

// -----------------------------------------------------------------------------
QString ItkBinaryWatershedLabeled::getNewCellArrayName() const
{
  return m_NewCellArrayName;
}
