#ifndef _ItkReadImageImpl_H_
#define _ItkReadImageImpl_H_


//image reading
#include "itkImageIOBase.h"
#include "itkImageIOFactory.h"
#include "itkImageFileReader.h"
#include "itkRGBPixel.h"
#include "itkRGBAPixel.h"
#include "itkVectorImage.h"

// ImageProcessing Plugin
#include "ItkBridge.h"

/**
 * @brief This is a private implementation for the filter that handles the actual algorithm implementation details
 * for us like figuring out if we can use this private implementation with the data array that is assigned.
 */
template<typename PixelType, typename AbstractFilter>
class ItkReadImagePrivate
{
  public:
    typedef DataArray<PixelType> DataArrayType;

    ItkReadImagePrivate() {}
    virtual ~ItkReadImagePrivate() {}

    // -----------------------------------------------------------------------------
    // Determine if this is the proper type of an array to downcast from the IDataArray
    // -----------------------------------------------------------------------------
    bool operator()(IDataArray::Pointer p)
    {
      return (boost::dynamic_pointer_cast<DataArrayType>(p).get() != NULL);
    }

    // -----------------------------------------------------------------------------
    // This is the actual templated algorithm
    // -----------------------------------------------------------------------------
    void static Execute(AbstractFilter* filter, QString inputFile, IDataArray::Pointer outputIDataArray)
    {
      typename DataArrayType::Pointer outputDataPtr = boost::dynamic_pointer_cast<DataArrayType>(outputIDataArray);

      //convert arrays to correct type
      PixelType* outputData = static_cast<PixelType*>(outputDataPtr->getPointer(0));
      size_t numVoxels = outputDataPtr->getNumberOfTuples();

      //read image meta data and get pixel type
      itk::ImageIOBase::Pointer imageIO = itk::ImageIOFactory::CreateImageIO(inputFile.toLocal8Bit().constData(), itk::ImageIOFactory::ReadMode);
      imageIO->SetFileName(inputFile.toLocal8Bit().data());
      imageIO->ReadImageInformation();
      itk::ImageIOBase::IOPixelType pixelType = imageIO->GetPixelType();

      itk::ProcessObject::Pointer readerObject;

      //read based on pixel type
      switch(pixelType)
      {
        case itk::ImageIOBase::SCALAR:
        {
          typedef itk::Image<PixelType, ImageProcessingConstants::ImageDimension> ImageType;
          typedef itk::ImageFileReader<ImageType> ReaderType;
          typename ReaderType::Pointer reader = ReaderType::New();
          reader->SetFileName(inputFile.toLocal8Bit().constData());
          reader->GetOutput()->GetPixelContainer()->SetImportPointer(outputData, numVoxels, false);
          readerObject = reader;
        }
        break;

        case itk::ImageIOBase::RGB:
        {
          typedef itk::Image<itk::RGBPixel<PixelType>, ImageProcessingConstants::ImageDimension> ImageType;
          typedef itk::ImageFileReader<ImageType> ReaderType;
          typename ReaderType::Pointer reader = ReaderType::New();
          reader->SetFileName(inputFile.toLocal8Bit().constData());
          reader->GetOutput()->GetPixelContainer()->SetImportPointer(reinterpret_cast<itk::RGBPixel<PixelType>*>(outputData), numVoxels, false);
          readerObject = reader;
        }
        break;

        case itk::ImageIOBase::RGBA:
        {
          typedef itk::Image<itk::RGBAPixel<PixelType>, ImageProcessingConstants::ImageDimension> ImageType;
          typedef itk::ImageFileReader<ImageType> ReaderType;
          typename ReaderType::Pointer reader = ReaderType::New();
          reader->SetFileName(inputFile.toLocal8Bit().constData());
          reader->GetOutput()->GetPixelContainer()->SetImportPointer(reinterpret_cast<itk::RGBAPixel<PixelType>*>(outputData), numVoxels, false);
          readerObject = reader;
        }
        break;
        /**
        case itk::ImageIOBase::FIXEDARRAY:
          {
            typedef itk::VectorImage<PixelType>, ImageProcessingConstants::ImageDimension> ImageType;
            typedef itk::ImageFileReader<ImageType> ReaderType;
            typename ReaderType::Pointer reader = ReaderType::New();
            reader->SetFileName(inputFile.toLocal8Bit().constData());
            reader->GetOutput()->GetPixelContainer()->SetImportPointer(outputData, numVoxels, false);
            readerObject=reader;
          }break;
          */
        case itk::ImageIOBase::UNKNOWNPIXELTYPE:
        case itk::ImageIOBase::OFFSET:
        case itk::ImageIOBase::VECTOR:
        case itk::ImageIOBase::POINT:
        case itk::ImageIOBase::COVARIANTVECTOR:
        case itk::ImageIOBase::SYMMETRICSECONDRANKTENSOR:
        case itk::ImageIOBase::DIFFUSIONTENSOR3D:
        case itk::ImageIOBase::COMPLEX:
        case itk::ImageIOBase::FIXEDARRAY:
        case itk::ImageIOBase::MATRIX:
          break;
        default:
          filter->setErrorCondition(-2);
          QString message = QObject::tr("Unable to read image '%1'").arg(inputFile);
          filter->notifyErrorMessage(filter->getHumanLabel(), message, filter->getErrorCondition());
          outputIDataArray->resize(0);
      }

      try
      {
        readerObject->Update();
      }
      catch( itk::ExceptionObject& err )
      {
        filter->setErrorCondition(-5);
        QString ss = QObject::tr("Failed to read image '%1': %2").arg(inputFile).arg(err.GetDescription());
        filter->notifyErrorMessage(filter->getHumanLabel(), ss, filter->getErrorCondition());
      }
    }
  private:
    ItkReadImagePrivate(const ItkReadImagePrivate&); // Copy Constructor Not Implemented
    void operator=(const ItkReadImagePrivate&); // Operator '=' Not Implemented
};


#endif /* _ItkReadImageImpl_H_ */
