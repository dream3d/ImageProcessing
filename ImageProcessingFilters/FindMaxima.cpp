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
#include "FindMaxima.h"

#include "DREAM3DLib/Common/TemplateHelpers.hpp"

// ImageProcessing Plugin
#include "ItkBridge.h"
#include "ImageProcessing/ImageProcessingHelpers.hpp"
#include "itkRegionalMaximaImageFilter.h"
#include "itkBinaryImageToLabelMapFilter.h"
#include "itkBinaryThresholdImageFunction.h"
#include "itkFloodFilledImageFunctionConditionalIterator.h"

  //this class emulates imagej's "find maxima" algorithm
  template< class TInputImage >
  class LocalMaxima
  {
    public:
      typedef itk::Image<uint8_t, TInputImage::ImageDimension> BinaryImageType;
      typedef itk::RegionalMaximaImageFilter<TInputImage, BinaryImageType> MaximaType;
      typedef itk::BinaryImageToLabelMapFilter<BinaryImageType> BinaryToLabelType;
      typedef itk::BinaryThresholdImageFunction< TInputImage, double > ThresholdFunctionType;
      typedef itk::FloodFilledImageFunctionConditionalIterator< TInputImage, ThresholdFunctionType > FloodingIterator;

      typename std::vector<typename TInputImage::IndexType> static Find(typename TInputImage::Pointer inputImage, typename TInputImage::PixelType noiseTolerance, bool fullyConnected)
      {
        //find local maxaima (any region of constant value surrounded by pixels of lower value)
        typename MaximaType::Pointer maxima = MaximaType::New();
        maxima->SetInput(inputImage);
        maxima->SetBackgroundValue(0);
        maxima->SetForegroundValue(255);
        maxima->SetFullyConnected(fullyConnected);//4 vs 8 connected

        //segment local maxima flag image
        typename BinaryToLabelType::Pointer binaryLabel = BinaryToLabelType::New();
        binaryLabel->SetInput(maxima->GetOutput());
        binaryLabel->SetFullyConnected(fullyConnected);
        binaryLabel->Update();

        //loop over all local maxima eliminating bad peaks
        int numObjects = binaryLabel->GetOutput()->GetNumberOfLabelObjects();
        std::vector<bool> goodPeak (numObjects, true);
        for(int i=0; i<numObjects; i++)
        {
          //make sure we haven't already eliminated this peak
          if(goodPeak[i])
          {
            //get peak label object and height of peak
            typename BinaryToLabelType::OutputImageType::LabelObjectType* labelObject = binaryLabel->GetOutput()->GetNthLabelObject(i);

            //create list of seed points (label member pixels)
            typename std::vector<typename TInputImage::IndexType> seedList;
            for(size_t j=0; j<labelObject->Size(); j++)
            {
              seedList.push_back(labelObject->GetIndex(j));
            }

            //get peak value (all pixels in label have same value)
            typename TInputImage::PixelType peakValue = inputImage->GetPixel(seedList[0]);

            //create threshold function to flood fill
            typename ThresholdFunctionType::Pointer thresholdFunction = ThresholdFunctionType::New();
            thresholdFunction->SetInputImage(inputImage);
            thresholdFunction->ThresholdAbove(peakValue-noiseTolerance);//flood fill through anything within tolerance

            //iterate over image, flood filling (only changes pixels in iterator list, not image values)
            FloodingIterator it(inputImage, thresholdFunction, seedList);
            it.GoToBegin();
            while ( !it.IsAtEnd() )
            {
              //another peak of higher intensity is within the watershed tolerance, this peak is bad
              if(it.Get()>peakValue)
              {
                goodPeak[i]=false;
                break;
              }
              else if(it.Get()==peakValue)
              {
                //check if index belongs to another object (not in this peak)
                typename TInputImage::IndexType otherIndex = it.GetIndex();
                if(!labelObject->HasIndex(otherIndex))
                {
                  //there is another peak within tolerance that is the same intensity, find the peak it belongs to
                  //loop over other good objects to find peak id
                  for(int j = i+1; j < numObjects; j++)
                  {
                    if(goodPeak[j])
                    {
                      if(binaryLabel->GetOutput()->GetNthLabelObject(j)->HasIndex(otherIndex))
                      {
                        //label j is the peak with the same value as i, merge labels
                        goodPeak[j] = false;
                        typename BinaryToLabelType::OutputImageType::LabelObjectType* otherLabelObject = binaryLabel->GetOutput()->GetNthLabelObject(j);
                        for(size_t k=0; k<otherLabelObject->Size(); k++)
                        {
                          labelObject->AddIndex(otherLabelObject->GetIndex(k));
                        }
                        break;
                      }
                    }
                  }
                }
              }
              //increment
              ++it;
            }
          }
        }

        //loop over all good peaks consolidating from a region->1 voxel
        std::vector<typename TInputImage::IndexType> peakLocations;
        for(int i=0; i<numObjects; i++)
        {
          if(goodPeak[i])
          {
            //get label object and find size
            typename BinaryToLabelType::OutputImageType::LabelObjectType* labelObject = binaryLabel->GetOutput()->GetNthLabelObject(i);
            int numVoxels = labelObject->Size();

            //find average location
            typename TInputImage::IndexType peakIndex;
            if(1==numVoxels)
            {
              for(int k=0; k<TInputImage::ImageDimension; k++)
              {
                peakIndex[k] = labelObject->GetIndex(0)[k];
              }
            }
            else
            {
              typename std::vector<float> avgIndex(TInputImage::ImageDimension, 0);
              for(int j=0; j<numVoxels; j++)
              {
                for(int k=0; k<TInputImage::ImageDimension; k++)
                {
                  avgIndex[k] = avgIndex[k] + labelObject->GetIndex(j)[k];
                }
              }
              for(int k=0; k<TInputImage::ImageDimension; k++)
              {
                avgIndex[k] = avgIndex[k] / numVoxels;
                peakIndex[k] = floor(avgIndex[k]);
                if(avgIndex[k]-peakIndex[k]>=0.5) peakIndex[k]++;
              }
            }
            peakLocations.push_back(peakIndex);
          }
        }

        return peakLocations;
      }
  };


/**
 * @brief This is a private implementation for the filter that handles the actual algorithm implementation details
 * for us like figuring out if we can use this private implementation with the data array that is assigned.
 */
template<typename PixelType>
class FindMaximaPrivate
{
  public:
    typedef DataArray<PixelType> DataArrayType;

    FindMaximaPrivate() {}
    virtual ~FindMaximaPrivate() {}

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
    void static Execute(FindMaxima* filter, IDataArray::Pointer inputArray, double tolerance, bool* outputData, DataContainer::Pointer m, QString attrMatName)
    {
      typename DataArrayType::Pointer inputArrayPtr = boost::dynamic_pointer_cast<DataArrayType>(inputArray);

      //convert array to correct type
      PixelType* inputData = static_cast<PixelType*>(inputArrayPtr->getPointer(0));

      //size_t numVoxels = inputArrayPtr->getNumberOfTuples();

      typedef ItkBridge<PixelType> ItkBridgeType;

      //wrap input and output as itk image
      typedef itk::Image<PixelType, ImageProcessing::ImageDimension> ImageType;
      typedef itk::Image<bool, ImageProcessing::ImageDimension> BoolImageType;
      typename ImageType::Pointer inputImage = ItkBridge<PixelType>::CreateItkWrapperForDataPointer(m, attrMatName, inputData);
      BoolImageType::Pointer outputImage = ItkBridge<bool>::CreateItkWrapperForDataPointer(m, attrMatName, outputData);

      //find maxima
      std::vector<typename ImageType::IndexType> peakLocations;
      try
      {
        peakLocations = ImageProcessing::LocalMaxima<ImageType>::Find(inputImage, tolerance, true);
      }
      catch( itk::ExceptionObject& err )
      {
        filter->setErrorCondition(-5);
        QString ss = QObject::tr("Failed to convert image. Error Message returned from ITK:\n   %1").arg(err.GetDescription());
        filter->notifyErrorMessage(filter->getHumanLabel(), ss, filter->getErrorCondition());
      }

      //fill output data with false then set peaks to true
      outputImage->FillBuffer(false);
      for(size_t i = 0; i < peakLocations.size(); i++)
      {
        outputImage->SetPixel(peakLocations[i], true);
      }
    }
  private:
    FindMaximaPrivate(const FindMaximaPrivate&); // Copy Constructor Not Implemented
    void operator=(const FindMaximaPrivate&); // Operator '=' Not Implemented
};


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindMaxima::FindMaxima() :
  AbstractFilter(),
  m_SelectedCellArrayPath("", "", ""),
  m_Tolerance(1.0),
  m_NewCellArrayName("Maxima"),
  m_SelectedCellArray(NULL),
  m_NewCellArray(NULL)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FindMaxima::~FindMaxima()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMaxima::setupFilterParameters()
{
  FilterParameterVector parameters;
  parameters.push_back(FilterParameter::New("Input Array", "SelectedCellArrayPath", FilterParameterWidgetType::DataArraySelectionWidget, getSelectedCellArrayPath(), false, ""));
  parameters.push_back(FilterParameter::New("Noise Tolerance", "Tolerance", FilterParameterWidgetType::DoubleWidget, getTolerance(), false, ""));
  parameters.push_back(FilterParameter::New("Created Array Name", "NewCellArrayName", FilterParameterWidgetType::StringWidget, getNewCellArrayName(), false, ""));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMaxima::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedCellArrayPath( reader->readDataArrayPath( "SelectedCellArrayPath", getSelectedCellArrayPath() ) );
  setTolerance( reader->readValue( "Tolerance", getTolerance() ) );
  setNewCellArrayName( reader->readString( "NewCellArrayName", getNewCellArrayName() ) );
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int FindMaxima::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  DREAM3D_FILTER_WRITE_PARAMETER(SelectedCellArrayPath)
  DREAM3D_FILTER_WRITE_PARAMETER(Tolerance)
  DREAM3D_FILTER_WRITE_PARAMETER(NewCellArrayName)
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMaxima::dataCheck()
{
  setErrorCondition(0);
  DataArrayPath tempPath;

  //check for required arrays
  QVector<size_t> compDims(1, 1);
  m_SelectedCellArrayPtr = TemplateHelpers::GetPrereqArrayFromPath<AbstractFilter>()(this, getSelectedCellArrayPath(), compDims);
  if(NULL != m_SelectedCellArrayPtr.lock().get())
  {
    m_SelectedCellArray = m_SelectedCellArrayPtr.lock().get();
  }

  //configured created name / location
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );

  DataContainer::Pointer dataContiner = getDataContainerArray()->getPrereqDataContainer<AbstractFilter>(this, getSelectedCellArrayPath().getDataContainerName() );
  if(getErrorCondition() < 0) { return; }
  AttributeMatrix::Pointer attrMatrix = dataContiner->getPrereqAttributeMatrix<AbstractFilter>(this, getSelectedCellArrayPath().getAttributeMatrixName(), 80000);
  if(getErrorCondition() < 0) { return; }
  IDataArray::Pointer redArrayptr = attrMatrix->getPrereqIDataArray<IDataArray, AbstractFilter>(this, getSelectedCellArrayPath().getDataArrayName(), 80000);
  if(getErrorCondition() < 0 || NULL == redArrayptr.get()) { return; }
  ImageGeom::Pointer image = dataContiner->getPrereqGeometry<ImageGeom, AbstractFilter>(this);
  if(getErrorCondition() < 0 || NULL == image.get()) { return; }
  //create new boolean array
  tempPath.update(getSelectedCellArrayPath().getDataContainerName(), getSelectedCellArrayPath().getAttributeMatrixName(), getNewCellArrayName() );
  m_NewCellArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<bool>, AbstractFilter, bool>(this, tempPath, 0, compDims); /* Assigns the shared_ptr<> to an instance variable that is a weak_ptr<> */
  if( NULL != m_NewCellArrayPtr.lock().get() ) /* Validate the Weak Pointer wraps a non-NULL pointer to a DataArray<T> object */
  { m_NewCellArray = m_NewCellArrayPtr.lock()->getPointer(0); } /* Now assign the raw pointer to data from the DataArray<T> object */

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void FindMaxima::preflight()
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
void FindMaxima::execute()
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

  //get volume container
  DataContainer::Pointer m = getDataContainerArray()->getDataContainer(getSelectedCellArrayPath().getDataContainerName());
  QString attrMatName = getSelectedCellArrayPath().getAttributeMatrixName();

  //get input data
  IDataArray::Pointer inputData = m_SelectedCellArrayPtr.lock();

  //execute type dependant portion using a Private Implementation that takes care of figuring out if
  // we can work on the correct type and actually handling the algorithm execution. We pass in "this" so
  // that the private implementation can get access to the current object to pass up status notifications,
  // progress or handle "cancel" if needed.
  if(FindMaximaPrivate<int8_t>()(inputData))
  {
    FindMaximaPrivate<int8_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint8_t>()(inputData) )
  {
    FindMaximaPrivate<uint8_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<int16_t>()(inputData) )
  {
    FindMaximaPrivate<int16_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint16_t>()(inputData) )
  {
    FindMaximaPrivate<uint16_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<int32_t>()(inputData) )
  {
    FindMaximaPrivate<int32_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint32_t>()(inputData) )
  {
    FindMaximaPrivate<uint32_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<int64_t>()(inputData) )
  {
    FindMaximaPrivate<int64_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<uint64_t>()(inputData) )
  {
    FindMaximaPrivate<uint64_t>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<float>()(inputData) )
  {
    FindMaximaPrivate<float>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else if(FindMaximaPrivate<double>()(inputData) )
  {
    FindMaximaPrivate<double>::Execute(this, inputData, m_Tolerance, m_NewCellArray, m, attrMatName);
  }
  else
  {
    setErrorCondition(-10001);
    ss = QObject::tr("A Supported DataArray type was not used for an input array.");
    notifyErrorMessage(getHumanLabel(), ss, getErrorCondition());
    return;
  }

  /* Let the GUI know we are done with this filter */
  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer FindMaxima::newFilterInstance(bool copyFilterParameters)
{
  FindMaxima::Pointer filter = FindMaxima::New();
  if(true == copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString FindMaxima::getCompiledLibraryName()
{return ImageProcessing::ImageProcessingBaseName;}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString FindMaxima::getGroupName()
{return "ImageProcessing";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString FindMaxima::getSubGroupName()
{return "Misc";}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString FindMaxima::getHumanLabel()
{return "Find Maxima";}

