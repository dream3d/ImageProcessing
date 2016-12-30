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
#include "ItkWriteImage.h"

#include <QtCore/QString>
#include <QtCore/QFileInfo>

#include "itkImageFileWriter.h"
#include "itkRGBPixel.h"
#include "itkRGBAPixel.h"
#include "itkVectorImage.h"


#include "SIMPLib/Common/TemplateHelpers.hpp"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/OutputFileFilterParameter.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/Geometry/ImageGeom.h"

#include "ImageProcessing/ImageProcessingFilters/ItkBridge.h"


/**
 * @brief This is a private implementation for the filter that handles the actual algorithm implementation details
 * for us like figuring out if we can use this private implementation with the data array that is assigned.
 */
template<typename TInputType>
class WriteImagePrivate
{
  public:
    typedef DataArray<TInputType> DataArrayType;

    WriteImagePrivate() {}
    virtual ~WriteImagePrivate() {}

    // -----------------------------------------------------------------------------
    // Determine if this is the proper type of an array to downcast from the IDataArray
    // -----------------------------------------------------------------------------
    bool operator()(IDataArray::Pointer p)
    {
      return (std::dynamic_pointer_cast<DataArrayType>(p).get() != nullptr);
    }

    // -----------------------------------------------------------------------------
    // This is the actual templated algorithm
    // -----------------------------------------------------------------------------
    void static Execute(ItkWriteImage* filter, DataContainer::Pointer m, QString attrMatName, IDataArray::Pointer inputDataArray, QString outputFile)
    {
      typename DataArrayType::Pointer inputDataPtr = std::dynamic_pointer_cast<DataArrayType>(inputDataArray);

      // Get a Raw Pointer to the data
      TInputType* inputData = inputDataPtr->getPointer(0);
      //size_t numVoxels = inputDataPtr->getNumberOfTuples();

      //get utilities


      //get the total number of components of the data and switch accordingly
      int numComp = inputDataPtr->getNumberOfComponents();

      itk::ProcessObject::Pointer writerObject;

      if(1 == numComp) //scalar image
      {
        typedef ItkBridge<TInputType> ItkBridgeType;
        //define types and wrap input image
        typedef typename ItkBridgeType::ScalarImageType ImageType;
        typename ImageType::Pointer inputImage = ItkBridgeType::CreateItkWrapperForDataPointer(m, attrMatName, inputData);

        //create writer and execute
        typedef itk::ImageFileWriter<ImageType> WriterType;
        typename WriterType::Pointer writer = WriterType::New();
        writer->SetFileName( outputFile.toLatin1().constData() );
        writer->SetInput( inputImage );
        writerObject = writer;
      }
      else if(3 == numComp)//rgb image
      {
        //define types and wrap input image
        typedef ItkBridge<TInputType>                            ItkBridgeType;

        //define types and wrap input image
        typedef typename ItkBridgeType::RGBImageType ImageType;
        typedef typename itk::ImageFileWriter<ImageType> WriterType;
        //     typename ImageType::Pointer inputImage = ItkBridgeType::template Dream3DtoITKTemplate<ImageType>(m, attrMatName, inputData);
        typename ImageType::Pointer inputImage = ItkBridgeType::template Dream3DtoITKImportFilter<typename ImageType::PixelType>(m, attrMatName, inputData)->GetOutput();

        //create writer and execute
        typename WriterType::Pointer writer = WriterType::New();
        writer->SetFileName( outputFile.toLatin1().constData() );
        writer->SetInput( inputImage );
        writerObject = writer;
      }
      else if(4 == numComp)//rgba image
      {
        typedef ItkBridge<TInputType>                            ItkBridgeType;

        //define types and wrap input image
        typedef typename ItkBridgeType::RGBAImageType ImageType;
        typedef typename itk::ImageFileWriter<ImageType> WriterType;
        //     typename ImageType::Pointer inputImage = ItkBridgeType::template Dream3DtoITKTemplate<ImageType>(m, attrMatName, inputData);
        typename ImageType::Pointer inputImage = ItkBridgeType::template Dream3DtoITKImportFilter<typename ImageType::PixelType>(m, attrMatName, inputData)->GetOutput();

        //create writer and execute
        typename WriterType::Pointer writer = WriterType::New();
        writer->SetFileName( outputFile.toLatin1().constData() );
        writer->SetInput( inputImage );
        writerObject = writer;
      } /** else//vector image
      {
        //define types and wrap input image
        typedef itk::Image<itk::FixedArray<TInputType, numComp> >, ImageProcessingConstants::ImageDimension> ImageType;
        typedef itk::ImageFileWriter<ImageType> WriterType;
        ImageType::Pointer inputImage = ItkBridgeType::Dream3DtoITKTemplate<ImageType>(m, attrMatName, inputData);

        //create writer and execute
        typename WriterType::Pointer writer = WriterType::New();
        writer->SetFileName( outputFile.toLatin1().constData() );
        writer->SetInput( inputImage );
        writerObject = writer;
      }*/

      try
      {
        writerObject->Update();
      }
      catch( itk::ExceptionObject& err )
      {
        filter->setErrorCondition(-5);
        QString ss = QObject::tr("Failed to write image. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
        filter->notifyErrorMessage(filter->getHumanLabel(), ss, filter->getErrorCondition());
      }
    }
  private:
    WriteImagePrivate(const WriteImagePrivate&); // Copy Constructor Not Implemented
    void operator=(const WriteImagePrivate&); // Operator '=' Not Implemented
};


// Include the MOC generated file for this class
#include "moc_ItkWriteImage.cpp"



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkWriteImage::ItkWriteImage() :
  AbstractFilter(),
  m_SelectedCellArrayPath("", "", ""),
  m_OutputFileName(""),
  m_SelectedCellArray(nullptr)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ItkWriteImage::~ItkWriteImage()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWriteImage::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(SeparatorFilterParameter::New("Cell Data", FilterParameter::RequiredArray));
  {
    DataArraySelectionFilterParameter::RequirementType req;
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Color Data", SelectedCellArrayPath, FilterParameter::RequiredArray, ItkWriteImage, req));
  }
  parameters.push_back(SIMPL_NEW_OUTPUT_FILE_FP("Output File Name", OutputFileName, FilterParameter::Parameter, ItkWriteImage, "*.tif", "TIFF"));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWriteImage::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setOutputFileName( reader->readString( "OutputFileName", getOutputFileName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWriteImage::initialize()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWriteImage::dataCheck()
{
  setErrorCondition(0);


  if(m_OutputFileName.isEmpty())
  {
    QString ss = QObject::tr("Output file name/path was not given");
    setErrorCondition(-100);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  QFileInfo fi(m_OutputFileName);
  if(fi.suffix().compare("tif") != 0)
  {
    QString ss = QObject::tr("Image Stacks are only supported for TIFF formatted output");
    setErrorCondition(-101);
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  //pass empty dimensions to allow any size
  QVector<size_t> compDims;
  m_SelectedCellArrayPtr = TemplateHelpers::GetPrereqArrayFromPath<AbstractFilter>()(this, getSelectedCellArrayPath(), compDims);
  if(nullptr != m_SelectedCellArrayPtr.lock().get())
  {
    m_SelectedCellArray = m_SelectedCellArrayPtr.lock().get();
  }
  if(getErrorCondition() < 0) { return; }

  getDataContainerArray()->getPrereqGeometryFromDataContainer<ImageGeom, AbstractFilter>(this, getSelectedCellArrayPath().getDataContainerName());
  // Ignore returning from the dataCheck if this errors out. We are just trying to
  // ensure an ImageGeometry is selected. If code is added below that starts depending
  // on the image geometry, then the next line should be uncommented.
  //if(getErrorCondition() < 0 || nullptr == image.get()) { return; }


  //make sure dims of selected array are appropriate
  if(1 == compDims.size())
  {
    if(1 == compDims[0]) //scalar
    {

    }
    else if (3 == compDims[0])//rgb
    {
      notifyWarningMessage(getHumanLabel(), "Warning: writing of rgb images is currenlty experimental (unstable behavoir may occur)", 0);
    }
    else if (4 == compDims[0])//rgba
    {
      notifyWarningMessage(getHumanLabel(), "Warning: writing of rgba images is currenlty experimental (unstable behavoir may occur)", 0);
    }
    else  //vector
    {
      //notifyWarningMessage(getHumanLabel(), "Warning: writing of vector images is currenlty experimental (unstable behavoir may occur)", 0);
      setErrorCondition(-102);
      notifyErrorMessage(getHumanLabel(), "Error: writing of vector images is currently not supported", getErrorCondition());
    }
  }
  else
  {
    QString message = QObject::tr("The selected array '%1' has unsupported dimensionality (%2)").arg(m_SelectedCellArrayPath.getDataArrayName()).arg(compDims.size());
    setErrorCondition(-101);
    notifyErrorMessage(getHumanLabel(), message, getErrorCondition());
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void ItkWriteImage::preflight()
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
void ItkWriteImage::execute()
{
  //int err = 0;
  dataCheck();
  if(getErrorCondition() < 0) { return; }

  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath().getAttributeMatrixName();

  //get input data
  IDataArray::Pointer inputData = m_SelectedCellArrayPtr.lock();

  if(WriteImagePrivate<int8_t>()(inputData))
  {
    WriteImagePrivate<int8_t>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else if(WriteImagePrivate<uint8_t>()(inputData) )
  {
    WriteImagePrivate<uint8_t>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else if(WriteImagePrivate<int16_t>()(inputData) )
  {
    WriteImagePrivate<int16_t>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else if(WriteImagePrivate<uint16_t>()(inputData) )
  {
    WriteImagePrivate<uint16_t>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else if(WriteImagePrivate<int32_t>()(inputData) )
  {
    WriteImagePrivate<int32_t>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else if(WriteImagePrivate<uint32_t>()(inputData) )
  {
    WriteImagePrivate<uint32_t>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else if(WriteImagePrivate<int64_t>()(inputData) )
  {
    WriteImagePrivate<int64_t>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else if(WriteImagePrivate<uint64_t>()(inputData) )
  {
    WriteImagePrivate<uint64_t>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else if(WriteImagePrivate<float>()(inputData) )
  {
    WriteImagePrivate<float>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else if(WriteImagePrivate<double>()(inputData) )
  {
    WriteImagePrivate<double>::Execute(this, m, attrMatName, inputData, m_OutputFileName);
  }
  else
  {
    setErrorCondition(-10001);
    QString ss = QObject::tr("A Supported DataArray type was not used for an input array.");
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }



  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer ItkWriteImage::newFilterInstance(bool copyFilterParameters)
{
  ItkWriteImage::Pointer filter = ItkWriteImage::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkWriteImage::getCompiledLibraryName()
{return ImageProcessingConstants::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkWriteImage::getGroupName()
{return SIMPL::FilterGroups::Unsupported;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkWriteImage::getSubGroupName()
{return "IO";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString ItkWriteImage::getHumanLabel()
{ return "Write Tiff Image Stack (ImageProcessing)"; }

