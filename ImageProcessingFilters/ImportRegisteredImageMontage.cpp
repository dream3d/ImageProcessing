/*
 * Your License or Copyright Information can go here
 */

#include "ImportRegisteredImageMontage.h"

#include <QtCore/QDir>
#include <QtGui/QImageReader>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/TemplateHelpers.h"
#include "SIMPLib/Common/TemplateHelpers.hpp"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersWriter.h"
#include "SIMPLib/FilterParameters/FileListInfoFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatVec3FilterParameter.h"
#include "SIMPLib/FilterParameters/InputFileFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Utilities/FilePathGenerator.h"

#include "ImageProcessing/ImageProcessingFilters/ItkReadImageImpl.hpp"
#include "ImageProcessing/ImageProcessingConstants.h"
#include "ImageProcessing/ImageProcessingVersion.h"

// Get ITKReadImage so you can properly read in 16 bit images
#include "ItkReadImage.h"



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ImportRegisteredImageMontage::ImportRegisteredImageMontage()
: m_DataContainerName(SIMPL::Defaults::ImageDataContainerName)
, m_CellAttributeMatrixName(SIMPL::Defaults::CellAttributeMatrixName)
, m_MetaDataAttributeMatrixName("MetaDataAttributeMatrix")
,
    //  m_RegistrationFile(""),
    m_RegistrationCoordinatesArrayName("RegistrationCoordinates")
, m_AttributeArrayNamesArrayName("AttributeArrayNames")
, m_RegistrationCoordinates(nullptr)
{
  m_Origin.x = 0.0;
  m_Origin.y = 0.0;
  m_Origin.z = 0.0;

  m_Resolution.x = 1.0;
  m_Resolution.y = 1.0;
  m_Resolution.z = 1.0;

  m_InputFileListInfo.FileExtension = QString("tif");
  m_InputFileListInfo.StartIndex = 0;
  m_InputFileListInfo.EndIndex = 0;
  m_InputFileListInfo.PaddingDigits = 0;

  m_NumImages = 0;

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ImportRegisteredImageMontage::~ImportRegisteredImageMontage() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportRegisteredImageMontage::setupFilterParameters()
{
  QVector<FilterParameter::Pointer> parameters;
  parameters.push_back(FileListInfoFilterParameter::New("Input File List", "InputFileListInfo", getInputFileListInfo(), FilterParameter::Parameter, SIMPL_BIND_SETTER(ImportRegisteredImageMontage, this, InputFileListInfo), SIMPL_BIND_GETTER(ImportRegisteredImageMontage, this, InputFileListInfo)));
  parameters.push_back(FloatVec3FilterParameter::New("Origin", "Origin", getOrigin(), FilterParameter::Parameter, SIMPL_BIND_SETTER(ImportRegisteredImageMontage, this, Origin), SIMPL_BIND_GETTER(ImportRegisteredImageMontage, this, Origin)));
  parameters.push_back(FloatVec3FilterParameter::New("Resolution", "Resolution", getResolution(), FilterParameter::Parameter, SIMPL_BIND_SETTER(ImportRegisteredImageMontage, this, Resolution), SIMPL_BIND_GETTER(ImportRegisteredImageMontage, this, Resolution)));
  //parameters.push_back(InputFileFilterParameter::New("Registration File", "RegistrationFile", getRegistrationFile(), FilterParameter::Parameter, "", "*.txt"));
  parameters.push_back(SIMPL_NEW_STRING_FP("Data Container", DataContainerName, FilterParameter::CreatedArray, ImportRegisteredImageMontage));
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_STRING_FP("Cell Attribute Matrix", CellAttributeMatrixName, FilterParameter::CreatedArray, ImportRegisteredImageMontage));
  parameters.push_back(SeparatorFilterParameter::New("Meta Data", FilterParameter::CreatedArray));
  parameters.push_back(SIMPL_NEW_STRING_FP("Meta Data Attribute Matrix", MetaDataAttributeMatrixName, FilterParameter::CreatedArray, ImportRegisteredImageMontage));
  parameters.push_back(SIMPL_NEW_STRING_FP("Registration Coordinates", RegistrationCoordinatesArrayName, FilterParameter::CreatedArray, ImportRegisteredImageMontage));
  parameters.push_back(SIMPL_NEW_STRING_FP("Image Array Names", AttributeArrayNamesArrayName, FilterParameter::CreatedArray, ImportRegisteredImageMontage));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportRegisteredImageMontage::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setDataContainerName(reader->readString("DataContainerName", getDataContainerName() ) );
  setCellAttributeMatrixName(reader->readString("CellAttributeMatrixName", getCellAttributeMatrixName() ) );
  setMetaDataAttributeMatrixName(reader->readString("MetaDataAttributeMatrixName", getMetaDataAttributeMatrixName() ) );
  setRegistrationCoordinatesArrayName(reader->readString("RegistrationCoordinatesArrayName", getRegistrationCoordinatesArrayName() ) );
  setAttributeArrayNamesArrayName(reader->readString("AttributeArrayNamesArrayName", getAttributeArrayNamesArrayName() ) );
  setInputFileListInfo( reader->readFileListInfo("InputFileListInfo", getInputFileListInfo() ) );
  setOrigin( reader->readFloatVec3("Origin", getOrigin()) );
  setResolution( reader->readFloatVec3("Resolution", getResolution()) );
  //setRegistrationFile( reader->readString( "RegistrationFile", getRegistrationFile() ) );
  reader->closeFilterGroup();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportRegisteredImageMontage::dataCheck()
{
  clearErrorCondition();
  clearWarningCondition();

  QString ss;
  int32_t err = 0;

  if (m_InputFileListInfo.InputPath.isEmpty() == true)
  {
    ss = QObject::tr("The input directory must be set");
    setErrorCondition(-13);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());

    return;
  }


  DataContainer::Pointer m = getDataContainerArray()->createNonPrereqDataContainer<AbstractFilter>(this, getDataContainerName());
  if(getErrorCondition() < 0) { return; }

  ImageGeom::Pointer image = ImageGeom::CreateGeometry(SIMPL::Geometry::ImageGeometry);
  m->setGeometry(image);


  if (getInPreflight())
  {
    m_InStream.close();
  }


  bool hasMissingFiles = false;
  bool orderAscending = false;

  if (m_InputFileListInfo.Ordering == 0) { orderAscending = true; }
  else if (m_InputFileListInfo.Ordering == 1) { orderAscending = false; }

  // Now generate all the file names the user is asking for and populate the table
  QVector<QString> fileList = FilePathGenerator::GenerateFileList(m_InputFileListInfo.StartIndex,
                              m_InputFileListInfo.EndIndex, hasMissingFiles, orderAscending,
                              m_InputFileListInfo.InputPath, m_InputFileListInfo.FilePrefix,
                              m_InputFileListInfo.FileSuffix, m_InputFileListInfo.FileExtension,
                              m_InputFileListInfo.PaddingDigits);
  if (fileList.size() == 0)
  {
    QString ss = QObject::tr("No files have been selected for import. Have you set the input directory?");
    setErrorCondition(-11);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }
  else
  {
    // If we have RGB or RGBA Images then we are going to have to change things a bit.
    // We should read the file and see what we have? Of course Qt is going to read it up into
    // an RGB array by default
    int err = 0;
    QImageReader reader((fileList[0]));
    QSize imageDims = reader.size();
    int64_t dims[3] = {imageDims.width(), imageDims.height(), 1};
    /* Sanity check what we are trying to load to make sure it can fit in our address space.
     * Note that this does not guarantee the user has enough left, just that the
     * size of the volume can fit in the address space of the program
     */
#if   (CMP_SIZEOF_SSIZE_T==4)
    int64_t max = std::numeric_limits<size_t>::max();
#else
    int64_t max = std::numeric_limits<int64_t>::max();
#endif
    if(dims[0] * dims[1] > max)
    {
      err = -1;
      QString ss = QObject::tr("The total number of elements '%1' is greater than this program can hold. Try the 64 bit version.").arg((dims[0] * dims[1]));
      setErrorCondition(err);
      notifyErrorMessage(getHumanLabel(), ss, err);
    }

    if(dims[0] > max || dims[1] > max)
    {
      err = -1;
      QString ss = QObject::tr("One of the dimensions is greater than the max index for this sysem. Try the 64 bit version."
                               " dim[0]=%1  dim[1]=%2").arg(dims[0]).arg(dims[1]);
      setErrorCondition(err);
      notifyErrorMessage(getHumanLabel(), ss, err);
    }
    /* ************ End Sanity Check *************************** */
    m->getGeometryAs<ImageGeom>()->setDimensions(static_cast<size_t>(dims[0]), static_cast<size_t>(dims[1]), static_cast<size_t>(dims[2]));
    m->getGeometryAs<ImageGeom>()->setResolution(m_Resolution.x, m_Resolution.y, m_Resolution.z);
    m->getGeometryAs<ImageGeom>()->setOrigin(m_Origin.x, m_Origin.y, m_Origin.z);

	// Populate the structure with meaningful data
	//image/attribute matrix dimensions
	QVector<size_t> tDims(3, 0); // Touple dimensions (These need to be image width * height * pixel depth)
	QVector<size_t> cDims(1, 1); // Component dimensions 

    for (QVector<QString>::iterator filepath = fileList.begin(); filepath != fileList.end(); ++filepath)
    {
		// Set up the file path before anything else (needs to be exactly the same as it is in execute)
		QString imageFName = *filepath;
		QFileInfo fi(imageFName);
		if (!fi.exists())
		{
			continue;
		}
		QStringList splitFilePaths = imageFName.split('\\');
		QString fileName = splitFilePaths[splitFilePaths.size() - 1];
		splitFilePaths = fileName.split('.');
		ss = splitFilePaths[0]; // ss can be anything you want really; Have not tested to see if spaces mess anything up anywhere else, hopefully not; Could make into a user variable
		DataArrayPath path(getDataContainerName(), getCellAttributeMatrixName(), ss);


		// read image metadata
		itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(imageFName.toLocal8Bit().constData(), itk::ImageIOFactory::ReadMode);
		if (nullptr == imageIO)
		{
			setErrorCondition(-2);
			QString message = QObject::tr("Unable to read image '%1'").arg(imageFName);
			notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
			return;
		}
		imageIO->SetFileName(imageFName.toLocal8Bit().data());
		imageIO->ReadImageInformation();

		//get size of image
		const size_t numDimensions = imageIO->GetNumberOfDimensions();
		int xdim = imageIO->GetDimensions(0);
		int ydim = imageIO->GetDimensions(1);
		int zdim = 1;
		if (3 != numDimensions)
		{
			if (2 == numDimensions)
			{
				//allow 2 dimensional images (as 3d image with size 1 in the z direction)
			}
			else
			{
				QString message = QObject::tr("3 dimensional image required (slected image dimensions: %1)").arg(numDimensions);
				setErrorCondition(-3);
				notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
				return;
			}
		}
		else
		{
			zdim = imageIO->GetDimensions(2);
		}

		// Set up the touple dimensions
		tDims[0] = xdim;
		tDims[1] = ydim;
		tDims[2] = zdim;

		if (!m->doesAttributeMatrixExist(getCellAttributeMatrixName()))
		{ m->createNonPrereqAttributeMatrix(this, getCellAttributeMatrixName(), tDims, SIMPL::AttributeMatrixType::Cell); }
		if (getErrorCondition() < 0) { return; }

		// Set up the component dimmensions
		itk::ImageIOBase::IOPixelType pixelType = imageIO->GetPixelType();
		QVector<size_t> componentDims(1, 0);
		switch (pixelType)
		{

		case itk::ImageIOBase::SCALAR:
			componentDims[0] = 1;
			break;
		case itk::ImageIOBase::RGB:
			componentDims[0] = 3;
			break;
		case itk::ImageIOBase::RGBA:
			componentDims[0] = 4;
			break;
		default:
			setErrorCondition(-90001);
			notifyErrorMessage(getHumanLabel(), "The Pixel Type of the image is not supported with DREAM3D.", getErrorCondition());
		}


		//Now get how the actual image data is stored.
		IDataArray::Pointer data;
		itk::ImageIOBase::IOComponentType componentType = imageIO->GetComponentType();
		if (itk::ImageIOBase::CHAR == componentType)
		{
			data = Int8ArrayType::CreateArray(0, "Temp", false);
		}
		else if (itk::ImageIOBase::UCHAR == componentType)
		{
			data = UInt8ArrayType::CreateArray(0, "Temp", false);
		}
		else if (itk::ImageIOBase::SHORT == componentType)
		{
			data = Int16ArrayType::CreateArray(0, "Temp", false);
		}
		else if (itk::ImageIOBase::USHORT == componentType)
		{
			data = UInt16ArrayType::CreateArray(0, "Temp", false);
		}
		else if (itk::ImageIOBase::INT == componentType)
		{
			data = Int32ArrayType::CreateArray(0, "Temp", false);
		}
		else if (itk::ImageIOBase::UINT == componentType)
		{
			data = UInt32ArrayType::CreateArray(0, "Temp", false);
		}
		else if (itk::ImageIOBase::LONG == componentType)
		{
			data = Int64ArrayType::CreateArray(0, "Temp", false);
		}
		else if (itk::ImageIOBase::ULONG == componentType)
		{
			data = UInt64ArrayType::CreateArray(0, "Temp", false);
		}
		else if (itk::ImageIOBase::FLOAT == componentType)
		{
			data = FloatArrayType::CreateArray(0, "Temp", false);
		}
		else if (itk::ImageIOBase::DOUBLE == componentType)
		{
			data = DoubleArrayType::CreateArray(0, "Temp", false);
		}
		else
		{
			std::string componentTypeName = itk::ImageIOBase::GetComponentTypeAsString(componentType);
			QString message = QObject::tr("The component type type of '%1' (%2) is unsupported").arg(imageFName).arg(QString::fromStdString(componentTypeName));
			setErrorCondition(-9);
			notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
			return;
		}

		// Doing it this way because we have dynamic value's we're writing to
		TemplateHelpers::CreateNonPrereqArrayFromArrayType()(this, path, componentDims, data);
		if(getErrorCondition() < 0) { return; }
    }
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportRegisteredImageMontage::preflight()
{
  setInPreflight(true);
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ImportRegisteredImageMontage::execute()
{
  clearErrorCondition();
  clearWarningCondition();
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getDataContainerName());
  AttributeMatrix::Pointer attrMat = m->getAttributeMatrix(getCellAttributeMatrixName());

  bool hasMissingFiles = false;
  bool orderAscending = false;

  if (m_InputFileListInfo.Ordering == 0) { orderAscending = true; }
  else if (m_InputFileListInfo.Ordering == 1) { orderAscending = false; }

  // Now generate all the file names the user is asking for and populate the table
  QVector<QString> fileList = FilePathGenerator::GenerateFileList(m_InputFileListInfo.StartIndex,
                              m_InputFileListInfo.EndIndex, hasMissingFiles, orderAscending,
                              m_InputFileListInfo.InputPath, m_InputFileListInfo.FilePrefix,
                              m_InputFileListInfo.FileSuffix, m_InputFileListInfo.FileExtension,
                              m_InputFileListInfo.PaddingDigits);
  if (fileList.size() == 0)
  {
    QString ss = QObject::tr("No files have been selected for import. Have you set the input directory?");
    setErrorCondition(-11);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
  }

  // We'll be using ITKReadImage because it can handle most pixelTypes (8bit, 16bit, 16bit greyscale etc)
  ItkReadImage::Pointer ReadImageFilter = ItkReadImage::New();
  //ReadImageFilter->setDataContainerArray(getDataContainerArray());

  for (QVector<QString>::iterator filepath = fileList.begin(); filepath != fileList.end(); ++filepath)
  {
	// This is the same read-in-file format as in the DataCheck()
	// If 'ss' isn't the same value as it is in DataCheck then you won't see the values properly added in the 'Current structure' tab
	// If there's a problem, use the Write SIMPLview data file and look at the .dream3d file in HDFView
	// There should only be one instance of each image in the data
    QString imageFName = *filepath;
    QFileInfo fi(imageFName);
    if (!fi.exists())
    {
      continue;
    }
    QStringList splitFilePaths = imageFName.split('\\'); // It's '\\' because \ is a command
    QString fileName = splitFilePaths[splitFilePaths.size() - 1]; // 0th Index
    splitFilePaths = fileName.split('.'); // Drop the .tiff
	QString ss = splitFilePaths[0];
	// Set up the parameters for the ReadImage filter
	// DO NOT run preflight() from here. It will make the filter think there's something wrong and it won't properly load in the image
	ReadImageFilter->setInputFileName(imageFName);
	ReadImageFilter->setCellAttributeMatrixName(this->getCellAttributeMatrixName()); // These are the dafault values of the ITK ReadImage filter (which we're running here)
	ReadImageFilter->setDataContainerName(this->getDataContainerName()); 
	ReadImageFilter->setImageDataArrayName(ss);


	ReadImageFilter->execute();

	if (ReadImageFilter->getErrorCondition() == -10000)
	{
		QString message = QObject::tr("Image failed to pass data check. The image %1 might be a different size then the attribute matrix. Consider making all images the same dimensions.").arg(ss);
		setErrorCondition(-10000); // See itk read image filter
		notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
		return;
	}

	
	// Make sure the data is put down
	// This is simulating what is done in ITK ReadImage
	DataContainer::Pointer ImgFiltm = ReadImageFilter->getDataContainerArray()->getDataContainer(getDataContainerName());
	AttributeMatrix::Pointer ImgFiltattrMat = ImgFiltm->getAttributeMatrix(getCellAttributeMatrixName());
	IDataArray::Pointer imageData = ImgFiltattrMat->getAttributeArray(ReadImageFilter->getImageDataArrayName());

	if (getErrorCondition() < 0 || imageData == nullptr)
	{
		QString message = QObject::tr("The image %1 was unable to be imported").arg(ss);
		setErrorCondition(-5);
		notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
		return;
	}

	// Add the information to the Attribute Array
	// addAttributeArray will replace empty dummy arrays created in DataCheck i
	attrMat->addAttributeArray(ss, imageData);

    if (getCancel() == true) { return; }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ImportRegisteredImageMontage::newFilterInstance(bool copyFilterParameters) const
{
  ImportRegisteredImageMontage::Pointer filter = ImportRegisteredImageMontage::New();
  if(true == copyFilterParameters)
  {
    filter->setFilterParameters(getFilterParameters() );
    // We are going to hand copy all of the parameters because the other way of copying the parameters are going to
    // miss some of them because we are not enumerating all of them.
    SIMPL_COPY_INSTANCEVAR(DataContainerName)
    SIMPL_COPY_INSTANCEVAR(CellAttributeMatrixName)
    SIMPL_COPY_INSTANCEVAR(MetaDataAttributeMatrixName)
    SIMPL_COPY_INSTANCEVAR(RegistrationCoordinatesArrayName)
    SIMPL_COPY_INSTANCEVAR(AttributeArrayNamesArrayName)
    SIMPL_COPY_INSTANCEVAR(Resolution)
    SIMPL_COPY_INSTANCEVAR(Origin)
    SIMPL_COPY_INSTANCEVAR(InputFileListInfo)
   // SIMPL_COPY_INSTANCEVAR(RegistrationFile)
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ImportRegisteredImageMontage::getCompiledLibraryName() const
{
  return ImageProcessingConstants::ImageProcessingBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ImportRegisteredImageMontage::getBrandingString() const
{
  return "ImageProcessing";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ImportRegisteredImageMontage::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << ImageProcessing::Version::Major() << "." << ImageProcessing::Version::Minor() << "." << ImageProcessing::Version::Patch();
  return version;
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ImportRegisteredImageMontage::getGroupName() const
{ return SIMPL::FilterGroups::IOFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ImportRegisteredImageMontage::getSubGroupName() const
{ return SIMPL::FilterSubGroups::InputFilters; }

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ImportRegisteredImageMontage::getHumanLabel() const
{ return "Import Registered Image Montage"; }
