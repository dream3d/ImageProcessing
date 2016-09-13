/*
 * Your License or Copyright Information can go here
 */

#include "ItkDetermineStitchingCoordinatesGeneric.h"

#include <QtCore/QString>

#include "ImageProcessing/ImageProcessingFilters/util/DetermineStitching.h"


#include "SIMPLib/Common/TemplateHelpers.hpp"
#include "SIMPLib/DataArrays/StringDataArray.hpp"
#include "SIMPLib/FilterParameters/AttributeMatrixSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/FilterParameters/IntFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/LinkedChoicesFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"



// Include the MOC generated file for this class
#include "moc_ItkDetermineStitchingCoordinatesGeneric.cpp"



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkDetermineStitchingCoordinatesGeneric::ItkDetermineStitchingCoordinatesGeneric() :
AbstractFilter(),
  m_AttributeMatrixName(SIMPL::Defaults::ImageDataContainerName, SIMPL::Defaults::CellAttributeMatrixName, ""),
  m_ImportMode(0),
  m_xTileDim(3),
  m_yTileDim(3),
  m_OverlapPer(50.0f),
  m_UseZeissMetaData(false),
  m_MetaDataAttributeMatrixName("TileAttributeMatrix"),
  m_TileCalculatedInfoAttributeMatrixName("TileInfoAttrMat"),
  m_StitchedCoordinatesArrayName("StitchedCoordinates"),
  m_StitchedArrayNames("StitchedArrayNames")
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
// Class Destructor
// -----------------------------------------------------------------------------
ItkDetermineStitchingCoordinatesGeneric::~ItkDetermineStitchingCoordinatesGeneric()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkDetermineStitchingCoordinatesGeneric::setupFilterParameters()
{
  FilterParameterVector parameters;
//  {
//    QStringList linkedProps;
//    linkedProps << "MetaDataAttributeMatrixName";
//    parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Use Zeiss Meta Data", UseZeissMetaData, FilterParameter::Parameter, ItkDetermineStitchingCoordinatesGeneric, linkedProps));
//  }

  {
    LinkedChoicesFilterParameter::Pointer combobox = LinkedChoicesFilterParameter::New();
    combobox->setHumanLabel("Import Mode");
    combobox->setPropertyName("ImportMode");
    QVector<QString> choices;
    choices.push_back("Row-By-Row (Comb Order)");
    choices.push_back("Column-By-Column");
    choices.push_back("Snake-By-Row");
    choices.push_back("Snake-By-Column");
    choices.push_back("Zeiss Data (Legacy)");
    combobox->setChoices(choices);
    QStringList linkedProps;
    linkedProps << "MetaDataAttributeMatrixName";
    combobox->setLinkedProperties(linkedProps);
    combobox->setEditable(false);
    combobox->setCategory(FilterParameter::Parameter);
    parameters.push_back(combobox);
  }

  parameters.push_back(SeparatorFilterParameter::New("Dimensions", FilterParameter::RequiredArray));
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Tile Dimensions X", xTileDim, FilterParameter::RequiredArray, ItkDetermineStitchingCoordinatesGeneric));
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Tile Dimensions Y", yTileDim, FilterParameter::RequiredArray, ItkDetermineStitchingCoordinatesGeneric));
  parameters.push_back(SIMPL_NEW_INTEGER_FP("Overlap Percentage (Estimate):", OverlapPer, FilterParameter::RequiredArray, ItkDetermineStitchingCoordinatesGeneric));

  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));

  {
    AttributeMatrixSelectionFilterParameter::RequirementType req;
    parameters.push_back(SIMPL_NEW_AM_SELECTION_FP("Cell Attribute Matrix", AttributeMatrixName, FilterParameter::RequiredArray, ItkDetermineStitchingCoordinatesGeneric, req));
  }


  {
    AttributeMatrixSelectionFilterParameter::RequirementType req;
    parameters.push_back(SIMPL_NEW_AM_SELECTION_FP("Zeiss Meta Data Attribute Matrix", MetaDataAttributeMatrixName, FilterParameter::RequiredArray, ItkDetermineStitchingCoordinatesGeneric, req, 5));
  }

  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_STRING_FP("Stitched Attribute Matrix", TileCalculatedInfoAttributeMatrixName, FilterParameter::CreatedArray, ItkDetermineStitchingCoordinatesGeneric));
  parameters.push_back(SIMPL_NEW_STRING_FP("Stitched Coordinates", StitchedCoordinatesArrayName, FilterParameter::CreatedArray, ItkDetermineStitchingCoordinatesGeneric));
  parameters.push_back(SIMPL_NEW_STRING_FP("Stitched Coordinates Names", StitchedArrayNames, FilterParameter::CreatedArray, ItkDetermineStitchingCoordinatesGeneric));

  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkDetermineStitchingCoordinatesGeneric::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setImportMode(reader->readValue("ImportMode", getImportMode()));
  setxTileDim(reader->readValue("xTileDim", getxTileDim()));
  setyTileDim(reader->readValue("yTileDim", getyTileDim()));
  setOverlapPer(reader->readValue("OverlapPer", getOverlapPer()));
  setAttributeMatrixName(reader->readDataArrayPath("AttributeMatrixName", getAttributeMatrixName()));
  setUseZeissMetaData(reader->readValue("UseZeissMetaData", getUseZeissMetaData()));
  setMetaDataAttributeMatrixName(reader->readDataArrayPath("MetaDataAttributeMatrixName", getMetaDataAttributeMatrixName()));

  setTileCalculatedInfoAttributeMatrixName(reader->readString("TileCalculatedInfoAttributeMatrixName", getTileCalculatedInfoAttributeMatrixName()));
  setStitchedCoordinatesArrayName(reader->readString("StitchedCoordinatesArrayName", getStitchedCoordinatesArrayName()));
  setStitchedArrayNames(reader->readString("DataArrayNamesForStitchedCoordinates", getStitchedArrayNames()));

  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
// This runs to make sure all of the data you have inputed is at least valid. This information is shown in the current structure tab
// -----------------------------------------------------------------------------
void ItkDetermineStitchingCoordinatesGeneric::dataCheck()
{
  setErrorCondition(0);
  DataArrayPath tempPath;

  QString ss;

  // Get a pointer to the start of the attribute matrix
  AttributeMatrix::Pointer am = getDataContainerArray()->getAttributeMatrix(m_AttributeMatrixName);


  // If it's null, then it's not good throw an error
  if (am.get() == nullptr)
  {
    setErrorCondition(-76000);
    notifyErrorMessage(getHumanLabel(), "The attribute matrix has not been selected properly", -76000);
    return;
  }

  // Getting the names of whatever is stored in the metadata attribute array names
  QList<QString> names = am->getAttributeArrayNames();


  QVector<size_t> dims(1, 1);

  // Get a pointer to the first image to make sure that there are images on the stack currently
  // If there isn't then we have a problem; throw an error
  ImageGeom::Pointer image = getDataContainerArray()->getDataContainer(getAttributeMatrixName().getDataContainerName())->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCondition() < 0 || nullptr == image.get()) { return; }

  // Populate the m_PointerList with (Something)(Maybe the images?)
  m_PointerList.resize(names.size());

  for(int i = 0; i < names.size(); i++)
  {
    tempPath.update(getAttributeMatrixName().getDataContainerName(), getAttributeMatrixName().getAttributeMatrixName(), names[i]);

  m_SelectedCellArrayPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<ImageProcessingConstants::DefaultPixelType>, AbstractFilter>(this, tempPath, dims);

    if( nullptr != m_SelectedCellArrayPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
    { m_SelectedCellArray = m_SelectedCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

    m_PointerList[i] = m_SelectedCellArray;

  }

  // Zeiss Data things. I won't be using this because I won't be givin the Zeiss Data
  // Which is unfortunete
  if(m_UseZeissMetaData == true)
  {
    AttributeMatrix::Pointer MetaDataAm = getDataContainerArray()->getAttributeMatrix(m_MetaDataAttributeMatrixName);
    if(nullptr == MetaDataAm.get())
    {
      notifyErrorMessage(getHumanLabel(), "The Attribute Matrix was not found", -76001);
      return;
    }
    //        QString temp = "_META_DATA";
    bool a = getMetaDataAttributeMatrixName().getAttributeMatrixName().contains("_META_DATA");
    if (a == false)
    {
      notifyErrorMessage(getHumanLabel(), "The Attribute Matrix does not contain the Zeiss Meta Data", -76002);
      return;
    }

  }

  // Get current data container because we're not making a new one
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getAttributeMatrixName().getDataContainerName());
  if(getErrorCondition() < 0 || nullptr == m) { return; }

  // Create a new attribute matrix
  QVector<size_t> tDims(1, m_PointerList.size());
  AttributeMatrix::Pointer AttrMat = m->createNonPrereqAttributeMatrix<AbstractFilter>(this, getTileCalculatedInfoAttributeMatrixName(), tDims, SIMPL::AttributeMatrixType::CellFeature);
  if(getErrorCondition() < 0) { return; }

  dims[0] = 2;

  tempPath.update(getAttributeMatrixName().getDataContainerName(), getTileCalculatedInfoAttributeMatrixName(), getStitchedCoordinatesArrayName());
  m_StitchedCoordinatesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<float>, AbstractFilter, float>(this,  tempPath, 0, dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( nullptr != m_StitchedCoordinatesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  { m_StitchedCoordinates = m_StitchedCoordinatesPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

  dims[0] = 1;



  //tempPath.update(getAttributeMatrixName().getDataContainerName(), getTileCalculatedInfoAttributeMatrixName(), getStitchedArrayNames() );
  //m_DataArrayNamesForStitchedCoordinatesPtr = getDataContainerArray()->createNonPrereqArrayFromPath<StringDataArray, AbstractFilter, std::string>(this, tempPath, "0", dims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */

  StringDataArray::Pointer StrongDataArrayNames = StringDataArray::CreateArray(AttrMat->getNumberOfTuples(), getStitchedArrayNames());
  AttrMat->addAttributeArray(getStitchedArrayNames(), StrongDataArrayNames);
  m_DataArrayNamesForStitchedCoordinatesPtr = StrongDataArrayNames;

  if( nullptr != m_DataArrayNamesForStitchedCoordinatesPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-nullptr pointer to a DataArray<T> object */
  {
    // m_DataArrayNamesForStitchedCoordinates = m_DataArrayNamesForStitchedCoordinatesPtr.lock()->getPointer(0);  /* Now assign the raw pointer to data from the DataArray<T> object */
  }

  return;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkDetermineStitchingCoordinatesGeneric::preflight()
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
void ItkDetermineStitchingCoordinatesGeneric::execute()
{
  dataCheck();
  if(getErrorCondition() < 0) { return; }
  setErrorCondition(0);

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getAttributeMatrixName().getDataContainerName()); // M is CellData basically? or maybe it's the meta data? I don't know what 'm' means; // Wait it's a 'DataContainer' object. So it's just the data container?
  QString attrMatName = getAttributeMatrixName().getAttributeMatrixName();
  // Set up some preliminary variables
  AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(attrMatName);;
  FloatArrayType::Pointer temp;

  // Set up the origins and resolutions (some of this data is used in legacy, some of it's used in non-legacy, some of it's used in both; for now we're including it up here; consider moving it where it's cleanest)
  float sampleOrigin[3];
  float voxelResolution[3];

  m->getGeometryAs<ImageGeom>()->getOrigin(sampleOrigin);
  m->getGeometryAs<ImageGeom>()->getResolution(voxelResolution);
  QVector<size_t> udims = attrMat->getTupleDimensions(); // The udims variable is filled with information about the size of each image (provided they were imported correctly) [0] = x; [1] = y; [2] = z;
  size_t totalPoints = attrMat->getNumberOfTuples();

  // If mode is equal to the max value then we're using the legacy zeiss data (which we can't really use too well)
  // This code doesn't really work and I don't know how to fix it because I'm not using zeiss data. For now we'll just do this
  if (m_ImportMode == 5)
  {
    // Get all the information that is specific to the zeiss data
    // Zeiss
    QString XTileIndexName = "ImageIndexU";
    QString YTileIndexName = "ImageIndexV";
    QString XGlobalIndexName = "StagePositionX";
    QString YGlobalIndexName = "StagePositionY";
    QString XScale = "ScaleFactorForX";
    QString YScale = "ScaleFactorForY";

    // Information you can get from the Zeiss metadata
    QVector<qint32> xTileList(m_PointerList.size());
    QVector<qint32> yTileList(m_PointerList.size());
    QVector<float> xGlobCoordsList(m_PointerList.size());
    QVector<float> yGlobCoordsList(m_PointerList.size());

    xTileList = extractIntegerValues(XTileIndexName);
    if (getErrorCondition() < 0) { return; }
    yTileList = extractIntegerValues(YTileIndexName);
    if (getErrorCondition() < 0) { return; }

    QVector<float> scaleFactors = extractFloatValues(XScale);
    xGlobCoordsList = extractFloatValues(XGlobalIndexName);
    for (qint32 i = 0; i < xGlobCoordsList.size(); i++)
    {
      xGlobCoordsList[i] /= scaleFactors[i];
    }

    scaleFactors = extractFloatValues(YScale);
    yGlobCoordsList = extractFloatValues(YGlobalIndexName);
    for (qint32 i = 0; i < yGlobCoordsList.size(); i++)
    {
      yGlobCoordsList[i] /= scaleFactors[i];
    }


    // Use the helper class to do the actual stitching of the images. There are a lot
    // of parameters so make sure we understand all of them
    temp = DetermineStitching::FindGlobalOriginsLegacy(totalPoints, udims,
      sampleOrigin, voxelResolution,
      m_PointerList,
      xGlobCoordsList, yGlobCoordsList,
      xTileList, yTileList,
      this);

  }
  else
  {
    // Otherwise, we're not using the zeiss data method so call this and let everything work itself out
    temp = DetermineStitching::FindGlobalOrigins(m_xTileDim, m_yTileDim, m_ImportMode, m_OverlapPer, m_PointerList, udims, sampleOrigin, voxelResolution);
  }

#if 1
  temp->copyIntoArray(m_StitchedCoordinatesPtr.lock());
#else
  float* src = temp->getPointer(0);
  float* dest = m_StitchedCoordinatesPtr.lock()->getPointer(0);
  size_t totalBytes = (m_StitchedCoordinatesPtr.lock()->getNumberOfTuples() * m_StitchedCoordinatesPtr.lock()->getNumberOfComponents() * sizeof(float));
  ::memcpy(dest, src, totalBytes);
#endif
  StringDataArray::Pointer arrayNames = m_DataArrayNamesForStitchedCoordinatesPtr.lock();
  arrayNames->resize(m_PointerList.size());

  //Create another data array with the list of names of the images in the same order as the returned stitched coordinates
  QList<QString> names = attrMat->getAttributeArrayNames();
  for(size_t i = 0; i < names.size(); i++ )
  {
    arrayNames->setValue(i, names[i]);
  }

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
// For Zeiss Data
// -----------------------------------------------------------------------------
QVector<float> ItkDetermineStitchingCoordinatesGeneric::extractFloatValues(QString arrayName)
{
  QVector<float> tileList(m_PointerList.size());
  DataArrayPath tempPath;
  bool ok = false; // Used to make sure we can convert the string into an integer
  tempPath.update(getMetaDataAttributeMatrixName().getDataContainerName(), getMetaDataAttributeMatrixName().getAttributeMatrixName(), arrayName);

  QString datatype = getDataContainerArray()->getPrereqIDataArrayFromPath<IDataArray, AbstractFilter>(this, tempPath)->getTypeAsString();

  QVector<size_t> dims(1, 1);

  if(datatype == "int8_t")
  {
    std::stringstream str;
    IDataArray::Pointer iDataArray = getDataContainerArray()->getPrereqIDataArrayFromPath<DataArray<int8_t>, AbstractFilter>(this, tempPath);
    Int8ArrayType::Pointer MetaDataPtr = std::dynamic_pointer_cast<DataArray<int8_t> >(iDataArray);
    int8_t* MetaData = MetaDataPtr->getPointer(0);
    dims = MetaDataPtr->getComponentDimensions();
    for (size_t i = 0; i < m_PointerList.size(); i++)
    {

      for (size_t j = 0; j < dims[0]; j++)
      {
        char test = char(MetaData[(dims[0] * i + j)]);
        str << test;
      }

      str >> tileList[i];
      str.str("");
      str.clear();
    }


  }

  else if (datatype == "StringDataArray")
  {
    StringDataArray::Pointer metaDataArray = getDataContainerArray()->getPrereqArrayFromPath<StringDataArray, AbstractFilter>(this, tempPath, dims);

    for (size_t i = 0; i < m_PointerList.size(); i++)
    {
      QString value = metaDataArray->getValue(i);
      tileList[i] = value.toFloat(&ok);
      if(!ok)
      {
        QString ss = QObject::tr("Error trying to convert the string '%1' to a float. This string was part of the Data Array '%2' at index '%3'.").arg(value).arg(arrayName).arg(i);
        setErrorCondition(-34005);
        notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
        tileList.clear();
        return tileList;
      }
    }

  }
  else
  {
    QString ss = QObject::tr("Error trying to read the metadata");
    setErrorCondition(-34006);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  return tileList;
}

// -----------------------------------------------------------------------------
// For Zeiss Data
// -----------------------------------------------------------------------------
QVector<qint32> ItkDetermineStitchingCoordinatesGeneric::extractIntegerValues(QString arrayName)
{
  QVector<qint32> tileList(m_PointerList.size());
  DataArrayPath tempPath;
  bool ok = false; // Used to make sure we can convert the string into an integer
  tempPath.update(getMetaDataAttributeMatrixName().getDataContainerName(), getMetaDataAttributeMatrixName().getAttributeMatrixName(), arrayName);

  QVector<size_t> dims(1, 1);
  QString datatype = getDataContainerArray()->getPrereqIDataArrayFromPath<IDataArray, AbstractFilter>(this, tempPath)->getTypeAsString();

  if (datatype == "int8_t")
  {
    std::stringstream str;
    IDataArray::Pointer iDataArray = getDataContainerArray()->getPrereqIDataArrayFromPath<DataArray<int8_t>, AbstractFilter>(this, tempPath);
    Int8ArrayType::Pointer MetaDataPtr = std::dynamic_pointer_cast<DataArray<int8_t> >(iDataArray);
    int8_t* MetaData = MetaDataPtr->getPointer(0);
    dims = MetaDataPtr->getComponentDimensions();
    for (size_t i = 0; i < m_PointerList.size(); i++)
    {

      for (size_t j = 0; j < dims[0]; j++)
      {
        char test = char(MetaData[(dims[0] * i + j)]);
        str << test;
      }

      str >> tileList[i];
      str.str("");
      str.clear();
    }
  }
  else if (datatype == "StringDataArray")
  {

    StringDataArray::Pointer metaDataArray = getDataContainerArray()->getPrereqArrayFromPath<StringDataArray, AbstractFilter>(this, tempPath, dims);

    for (size_t i = 0; i < m_PointerList.size(); i++)
    {
      QString value = metaDataArray->getValue(i);
      tileList[i] = value.toInt(&ok);
      if(!ok)
      {
        QString ss = QObject::tr("Error trying to convert the string '%1' to an integer. This string was part of the Data Array '%2' at index '%3'.").arg(value).arg(arrayName).arg(i);
        setErrorCondition(-34005);
        notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
        tileList.clear();
        return tileList;
      }
    }
  }
  else
  {
    QString ss = QObject::tr("Error trying to read the metadata");
    setErrorCondition(-34006);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  return tileList;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkDetermineStitchingCoordinatesGeneric::getCompiledLibraryName()
{
  return ImageProcessingConstants::ImageProcessingBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkDetermineStitchingCoordinatesGeneric::getGroupName()
{
  return SIMPL::FilterGroups::Unsupported;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkDetermineStitchingCoordinatesGeneric::getHumanLabel()
{
  return "Determine Stitching Coordinates (Generic)";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkDetermineStitchingCoordinatesGeneric::getSubGroupName()
{
  return "Misc";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkDetermineStitchingCoordinatesGeneric::newFilterInstance(bool copyFilterParameters)
{
  /*
  * write code to optionally copy the filter parameters from the current filter into the new instance
  */
  ItkDetermineStitchingCoordinatesGeneric::Pointer filter = ItkDetermineStitchingCoordinatesGeneric::New();
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
    //    SIMPL_COPY_INSTANCEVAR(OutputFile)
    //    SIMPL_COPY_INSTANCEVAR(ZStartIndex)
    //    SIMPL_COPY_INSTANCEVAR(ZEndIndex)
    //    SIMPL_COPY_INSTANCEVAR(ZResolution)
  }
  return filter;
}

