/* ============================================================================
 * Copyright (c) 2014 Michael A. Jackson (BlueQuartz Software)
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
 * Neither the name of Michael A. Jackson, BlueQuartz Software nor the names of
 * its contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
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
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "DetermineStitching.h"

#include <QtCore/QDir>

#include "itkMaskedFFTNormalizedCorrelationImageFilter.h"
#include "itkImage.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkChangeInformationImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"


#include "ImageProcessing/ImageProcessingFilters/ItkBridge.h"
#include "ImageProcessing/ImageProcessingHelpers.hpp"






// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DetermineStitching::DetermineStitching()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DetermineStitching::~DetermineStitching()
{

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
//Finds the max value in a vector ... possibly redunant since it is one line ...
template<typename T>
T FindMaxValue(QVector<T> inputVector)
{
    typename QVector<T>::iterator it = std::max_element(inputVector.begin(), inputVector.end());
    return *it;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FloatArrayType::Pointer DetermineStitching::FindGlobalOrigins(size_t totalPoints,
                                                              QVector<size_t> udims,
                                                              float sampleOrigin[],
                                                              float voxelResolution[],
                                                              QVector<ImageProcessing::DefaultPixelType *> dataArrayList,
                                                              QVector<float> xGlobCoordsList,
                                                              QVector<float> yGlobCoordsList,
                                                              QVector<qint32> xTileList,
                                                              QVector<qint32> yTileList,
                                                              AbstractFilter* filter)
{

    QVector<size_t> cDims(1, 2);  // a dimension for the xvalues and one for the y values
    QVector<size_t> tDims(1);

    tDims[0] = xTileList.size();

    FloatArrayType::Pointer xyStitchedGlobalListPtr = FloatArrayType::CreateArray(tDims, cDims, "xyGlobalList");
    FloatArrayType::Pointer xyStitchedGlobalListPtr_orig = FloatArrayType::CreateArray(tDims, cDims, "xyGlobalList_orig");

    qint32 numXtiles = FindMaxValue(xTileList) + 1;
    qint32 numYtiles = FindMaxValue(yTileList) + 1;

    QVector<size_t> combIndexList(xTileList.size());

    //return an index list that puts all the tiles in an order as though they were collected by row combing
    //this is how the stitching algorithm will stitch the tiles together
    combIndexList = ReturnIndexForCombOrder(xTileList, yTileList, numXtiles, numYtiles);

    ImageProcessing::UInt8ImageType* currentImage;
    ImageProcessing::UInt8ImageType* leftImage;
    ImageProcessing::UInt8ImageType* aboveImage;
    ImageProcessing::ImportUInt8FilterType::Pointer importFilter;
    ImageProcessing::ImportUInt8FilterType::Pointer importFilter2;
    std::vector<float> cropSpecsIm1Im2(12, 0);
    std::vector<float> newXYOrigin(2,0);
    std::vector<float> newXYOrigin2(2,0);

    //set the stitched global coordinates of the first tile to the top left corner
    xyStitchedGlobalListPtr->setValue(0, 0);
    xyStitchedGlobalListPtr->setValue(1, 0);
    xyStitchedGlobalListPtr_orig->setValue(0, 0);
    xyStitchedGlobalListPtr_orig->setValue(1, 0);


    //helper variables to store previous stitched global values
    float previousXleft = 0;
    float previousYleft = 0;

    float previousXtop = 0;
    float previousYtop = 0;

    float newXfromleft = 0;
    float newYfromleft = 0;

    float newXfromtop = 0;
    float newYfromtop = 0;





    for (size_t i=1; i<combIndexList.size(); i++)
    {
        if (i < numXtiles) //if the image is in the top row of images, we need only the image to the left
        {

            //get filter to convert m_RawImageData to itk::image

            importFilter = ITKUtilitiesType::Dream3DtoITKImportFilterDataArray<ImageProcessing::DefaultPixelType>(totalPoints, udims, sampleOrigin, voxelResolution, dataArrayList[combIndexList[i]]);
            currentImage = importFilter->GetOutput();
            importFilter2 = ITKUtilitiesType::Dream3DtoITKImportFilterDataArray<ImageProcessing::DefaultPixelType>(totalPoints, udims, sampleOrigin, voxelResolution, dataArrayList[combIndexList[i-1]]);
            leftImage = importFilter2->GetOutput();

            // Determine the windows to be cross correlated depending on the rough overlap as found from the global coordinates
            cropSpecsIm1Im2[0] = xGlobCoordsList[combIndexList[i]] - xGlobCoordsList[combIndexList[i-1]]; //xGlobCoordsList[combIndexList[i]] - xyStitchedGlobalListPtr->getValue(2*(i-1)) - xGlobCoordsList[0]; //left image X Origin
            cropSpecsIm1Im2[1] = 0; //left image Y Origin
            cropSpecsIm1Im2[2] = 0; //left image Z Origin
            cropSpecsIm1Im2[3] = 0; //current image X Origin
            cropSpecsIm1Im2[4] = 0; //current image Y Origin
            cropSpecsIm1Im2[5] = 0; //current image Z Origin

            cropSpecsIm1Im2[6] = udims[0] - cropSpecsIm1Im2[0]; //left image X Size
            cropSpecsIm1Im2[7] = udims[1]; //left image Y Size
            cropSpecsIm1Im2[8] = 1; //left image Z Size
            cropSpecsIm1Im2[9] = udims[0] - cropSpecsIm1Im2[0]; //current image X Size
            cropSpecsIm1Im2[10] = udims[1]; //current image Y Size
            cropSpecsIm1Im2[11] = 1; //current image Z Size

            //Cross correlate the image windows and return the local shifts between the two images
            newXYOrigin = CropAndCrossCorrelate(cropSpecsIm1Im2, currentImage, leftImage);

            previousXleft = xyStitchedGlobalListPtr->getValue(2*(i-1));
            previousYleft = xyStitchedGlobalListPtr->getValue(2*(i-1)+1);

            newXfromleft = previousXleft + cropSpecsIm1Im2[0] + newXYOrigin[0];
            newYfromleft = previousYleft + newXYOrigin[1];

            xyStitchedGlobalListPtr->setValue(2*i, newXfromleft);
            xyStitchedGlobalListPtr->setValue(2*i + 1, newYfromleft);




        }

        else if (i%numXtiles == 0) //if the image is in the first (left most) column of images, we only need the top image
        {

            //get filter to convert m_RawImageData to itk::image

            importFilter = ITKUtilitiesType::Dream3DtoITKImportFilterDataArray<ImageProcessing::DefaultPixelType>(totalPoints, udims, sampleOrigin, voxelResolution, dataArrayList[combIndexList[i]]); //combIndexList[i]]
            currentImage = importFilter->GetOutput();
            importFilter2 = ITKUtilitiesType::Dream3DtoITKImportFilterDataArray<ImageProcessing::DefaultPixelType>(totalPoints, udims, sampleOrigin, voxelResolution, dataArrayList[combIndexList[i-numXtiles]]);
            aboveImage = importFilter2->GetOutput();

            // Determine the windows to be cross correlated depending on the rough overlap as found from the global coordinates

            cropSpecsIm1Im2[0] = 0; //top image X Origin
            cropSpecsIm1Im2[1] = yGlobCoordsList[combIndexList[i]] - yGlobCoordsList[combIndexList[i-numXtiles]];// yGlobCoordsList[combIndexList[i]] - xyStitchedGlobalListPtr->getValue(2*(i-numXtiles)+1) - yGlobCoordsList[0]; //top image Y Origin
            cropSpecsIm1Im2[2] = 0; //top image Z Origin
            cropSpecsIm1Im2[3] = 0; //current image X Origin
            cropSpecsIm1Im2[4] = 0; //current image Y Origin
            cropSpecsIm1Im2[5] = 0; //current image Z Origin

            cropSpecsIm1Im2[6] = udims[0]; //top image X Size
            cropSpecsIm1Im2[7] = udims[1] - cropSpecsIm1Im2[1]; //top image Y Size
            cropSpecsIm1Im2[8] = 1; //top image Z Size
            cropSpecsIm1Im2[9] = udims[0]; //current image X Size
            cropSpecsIm1Im2[10] = udims[1] - cropSpecsIm1Im2[1]; //current image Y Size
            cropSpecsIm1Im2[11] = 1; //current image Z Size


            //Cross correlate the image windows and return the local shifts between the two images
            newXYOrigin2 = CropAndCrossCorrelate(cropSpecsIm1Im2, currentImage, aboveImage);

            previousXtop = xyStitchedGlobalListPtr->getValue(2*(i-numXtiles));
            previousYtop = xyStitchedGlobalListPtr->getValue(2*(i-numXtiles)+1);

            //Add the local shifts to the preivous global value to get the current stitched global shift
            newXfromtop = previousXtop + newXYOrigin2[0];
            newYfromtop = previousYtop + newXYOrigin2[1] + cropSpecsIm1Im2[1];


            xyStitchedGlobalListPtr->setValue(2*i, newXfromtop);
            xyStitchedGlobalListPtr->setValue(2*i + 1, newYfromtop);






        }

        else  //for all other images, we need to match to the top and the left
        {

            //get filter to convert m_RawImageData to itk::image
            ///TOP IMAGE FIRST
            importFilter = ITKUtilitiesType::Dream3DtoITKImportFilterDataArray<ImageProcessing::DefaultPixelType>(totalPoints, udims, sampleOrigin, voxelResolution, dataArrayList[combIndexList[i]]);
            currentImage = importFilter->GetOutput();
            importFilter2 = ITKUtilitiesType::Dream3DtoITKImportFilterDataArray<ImageProcessing::DefaultPixelType>(totalPoints, udims, sampleOrigin, voxelResolution, dataArrayList[combIndexList[i-numXtiles]]);
            aboveImage = importFilter2->GetOutput();


            // Determine the windows to be cross correlated depending on the rough overlap as found from the global coordinates

            cropSpecsIm1Im2[0] = 0; //top image X Origin
            cropSpecsIm1Im2[1] = yGlobCoordsList[combIndexList[i]] - yGlobCoordsList[combIndexList[i-numXtiles]]; //yGlobCoordsList[combIndexList[i]] - xyStitchedGlobalListPtr->getValue(2*(i-numXtiles)+1) - yGlobCoordsList[0]; //top image Y Origin
            cropSpecsIm1Im2[2] = 0; //top image Z Origin
            cropSpecsIm1Im2[3] = 0; //current image X Origin
            cropSpecsIm1Im2[4] = 0; //current image Y Origin
            cropSpecsIm1Im2[5] = 0; //current image Z Origin

            cropSpecsIm1Im2[6] = udims[0]; //top image X Size
            cropSpecsIm1Im2[7] = udims[1] - cropSpecsIm1Im2[1]; //top image Y Size
            cropSpecsIm1Im2[8] = 1; //top image Z Size
            cropSpecsIm1Im2[9] = udims[0]; //current image X Size
            cropSpecsIm1Im2[10] = udims[1] - cropSpecsIm1Im2[1]; //current image Y Size
            cropSpecsIm1Im2[11] = 1; //current image Z Size

            //Cross correlate the image windows and return the local shifts between the two images
            newXYOrigin2 = CropAndCrossCorrelate(cropSpecsIm1Im2, currentImage, aboveImage);

            previousXtop = xyStitchedGlobalListPtr->getValue(2*(i-numXtiles));
            previousYtop = xyStitchedGlobalListPtr->getValue(2*(i-numXtiles)+1);

            //Add the local shifts to the preivous global value to get the current stitched global shift
            newXfromtop = previousXtop + newXYOrigin2[0];
            newYfromtop = previousYtop + newXYOrigin2[1] + cropSpecsIm1Im2[1];

            //BOTTOM IMAGE NEXT
            importFilter = ITKUtilitiesType::Dream3DtoITKImportFilterDataArray<ImageProcessing::DefaultPixelType>(totalPoints, udims, sampleOrigin, voxelResolution, dataArrayList[combIndexList[i]]);
            currentImage = importFilter->GetOutput();
            importFilter2 = ITKUtilitiesType::Dream3DtoITKImportFilterDataArray<ImageProcessing::DefaultPixelType>(totalPoints, udims, sampleOrigin, voxelResolution, dataArrayList[combIndexList[i-1]]);
            leftImage = importFilter2->GetOutput();

            cropSpecsIm1Im2[0] = xGlobCoordsList[combIndexList[i]] - xGlobCoordsList[combIndexList[i-1]]; //xGlobCoordsList[combIndexList[i]] - xyStitchedGlobalListPtr->getValue(2*(i-1)) - xGlobCoordsList[0]; //left image X Origin
            cropSpecsIm1Im2[1] = 0; //left image Y Origin
            cropSpecsIm1Im2[2] = 0; //left image Z Origin
            cropSpecsIm1Im2[3] = 0; //current image X Origin
            cropSpecsIm1Im2[4] = 0; //current image Y Origin
            cropSpecsIm1Im2[5] = 0; //current image Z Origin

            cropSpecsIm1Im2[6] = udims[0] - cropSpecsIm1Im2[0]; //left image X Size
            cropSpecsIm1Im2[7] = udims[1]; //left image Y Size
            cropSpecsIm1Im2[8] = 1; //left image Z Size
            cropSpecsIm1Im2[9] = udims[0] - cropSpecsIm1Im2[0]; //current image X Size
            cropSpecsIm1Im2[10] = udims[1]; //current image Y Size
            cropSpecsIm1Im2[11] = 1; //current image Z Size

            //Cross correlate the image windows and return the local shifts between the two images
            newXYOrigin = CropAndCrossCorrelate(cropSpecsIm1Im2, currentImage, leftImage);

            previousXleft = xyStitchedGlobalListPtr->getValue(2*(i-1));
            previousYleft = xyStitchedGlobalListPtr->getValue(2*(i-1)+1);

            //Add the local shifts to the preivous global value to get the current stitched global shift
            newXfromleft = previousXleft + cropSpecsIm1Im2[0] + newXYOrigin[0];
            newYfromleft = previousYleft + newXYOrigin[1];



            //AVERAGE the two new locations
            xyStitchedGlobalListPtr->setValue(2*i, (newXfromtop + newXfromleft)/2.0);
            xyStitchedGlobalListPtr->setValue(2*i + 1, (newYfromtop + newYfromleft)/2.0);

        }

        if(filter)
        {
            //Generate a QString of the message
            QString msg;
            QTextStream out(&msg);

            out << "Placing Image Number " << i;
            filter->notifyStatusMessage(filter->getMessagePrefix(), filter->getHumanLabel(), msg);


//            std::cout << "new global x " << xyStitchedGlobalListPtr->getValue(2*i) << std::endl;
//            std::cout << "new global y " << xyStitchedGlobalListPtr->getValue(2*i + 1) << std::endl;
        }

        //Put the values found in from going in the comb order into another data array which represents the original order the images came in as
        xyStitchedGlobalListPtr_orig->setValue(2*combIndexList[i], xyStitchedGlobalListPtr->getValue(2*i));
        xyStitchedGlobalListPtr_orig->setValue(2*combIndexList[i]+1, xyStitchedGlobalListPtr->getValue(2*i+1));
    }


    return xyStitchedGlobalListPtr_orig;

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<float> DetermineStitching::CropAndCrossCorrelate(std::vector<float> cropSpecsIm1Im2,  ImageProcessing::UInt8ImageType* currentImage,  ImageProcessing::UInt8ImageType* fixedImage)
{

    std::vector<float> newXYOrigin(2, 0);




    //////FIRST IMAGE CROP
    ImageProcessing::UInt8ImageType::RegionType cropRegion;

    cropRegion.SetIndex(0, cropSpecsIm1Im2[0]);
    cropRegion.SetIndex(1, cropSpecsIm1Im2[1]);
    cropRegion.SetIndex(2, cropSpecsIm1Im2[2]);

    cropRegion.SetSize(0, cropSpecsIm1Im2[6]);
    cropRegion.SetSize(1, cropSpecsIm1Im2[7]);
    cropRegion.SetSize(2, cropSpecsIm1Im2[8]);

    //Extract window from first image
    typedef itk::RegionOfInterestImageFilter< ImageProcessing::UInt8ImageType, ImageProcessing::UInt8ImageType > exImFilterType;
    exImFilterType::Pointer exImfilter = exImFilterType::New();
    exImfilter->SetRegionOfInterest(cropRegion);
    exImfilter->SetInput(fixedImage);
    exImfilter->Update();
    ImageProcessing::UInt8ImageType* fixedImageWindow = exImfilter->GetOutput();



    /////////////////////SECOND IMAGE CROP
    ImageProcessing::UInt8ImageType::RegionType cropRegion2;
    cropRegion2.SetIndex(0, cropSpecsIm1Im2[3]);
    cropRegion2.SetIndex(1, cropSpecsIm1Im2[4]);
    cropRegion2.SetIndex(2, cropSpecsIm1Im2[5]);

    cropRegion2.SetSize(0, cropSpecsIm1Im2[9]);
    cropRegion2.SetSize(1, cropSpecsIm1Im2[10]);
    cropRegion2.SetSize(2, cropSpecsIm1Im2[11]);

    //Extract window from second image
    exImFilterType::Pointer exImfilter2 = exImFilterType::New();
    exImfilter2->SetRegionOfInterest(cropRegion2);
    exImfilter2->SetInput(currentImage);
    exImfilter2->Update();
    ImageProcessing::UInt8ImageType* currentImageWindow = exImfilter2->GetOutput();


    ///CHANGE THE ORIGIN OF THE IMAGES SO THEY MATCH - the cross correlation filter will not work unless this is done
    typedef itk::ChangeInformationImageFilter< ImageProcessing::UInt8ImageType > ChangeInfoFilterType;
    ImageProcessing::UInt8ImageType::PointType origin = fixedImageWindow->GetOrigin();
    origin[0] = 0;
    origin[1] = 0;
    origin[2] = 0;

    //First the Fixed image
    ChangeInfoFilterType::Pointer changeInfo = ChangeInfoFilterType::New();
    changeInfo->SetInput(fixedImageWindow);
    changeInfo->SetOutputOrigin( origin );
    changeInfo->ChangeOriginOn();
    changeInfo->UpdateOutputInformation();
    ImageProcessing::UInt8ImageType* fixedImageWindow2 = changeInfo->GetOutput();

    //Next the Current Image
    ChangeInfoFilterType::Pointer changeInfo2 = ChangeInfoFilterType::New();
    changeInfo2->SetInput(currentImageWindow);
    changeInfo2->SetOutputOrigin( origin );
    changeInfo2->ChangeOriginOn();
    changeInfo2->UpdateOutputInformation();
    ImageProcessing::UInt8ImageType* currentImageWindow2 = changeInfo2->GetOutput();


    /////WRITING THE IMAGES FOR TESTING
    typedef itk::ImageFileWriter< ImageProcessing::UInt8ImageType > WriterType;
#if 0
    QString imagePath = QDir::homePath() + QDir::separator() + "Desktop" + QDir::separator() + "fixedImageWindow.tiff";
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(imagePath.toLatin1().constData());
    writer->SetInput( fixedImageWindow2);
    writer->Update();

    imagePath = QDir::homePath() + QDir::separator() + "Desktop" + QDir::separator() + "CurrentImageWindow.tiff";
    WriterType::Pointer writer2 = WriterType::New();
    writer2->SetFileName(imagePath.toLatin1().constData());
    writer2->SetInput( currentImageWindow2 );
    writer2->Update();
#endif




    //CROSS CORRELATE THE 2 WINDOWS.
    //Note: It is much faster to cross correlate the extracted windows than to cross correlate the full windows with a mask applied
    typedef itk::MaskedFFTNormalizedCorrelationImageFilter< ImageProcessing::DefaultImageType, ImageProcessing::FloatImageType, ImageProcessing::DefaultImageType > XCFilterType;
    XCFilterType::Pointer xCorrFilter = XCFilterType::New();
    xCorrFilter->SetFixedImage(fixedImageWindow2);
    xCorrFilter->SetMovingImage(currentImageWindow2);
    xCorrFilter->SetRequiredFractionOfOverlappingPixels(0.5); //currently require that the windows overlap at least 50percent. Might want to make this a user controlled variable
    xCorrFilter->Update();
    ImageProcessing::FloatImageType* xcoutputImage = xCorrFilter->GetOutput();

    // Create and initialize the calculator
    typedef itk::MinimumMaximumImageCalculator<ImageProcessing::FloatImageType>   MinMaxCalculatorType;
    MinMaxCalculatorType::Pointer calculator = MinMaxCalculatorType::New();
    calculator->SetImage( xcoutputImage );
    calculator->Compute();

    //    // Return minimum of intensity
    //    float minimumResult = calculator->GetMinimum();
    //    std::cout << "The Minimum intensity value is : " << minimumResult << std::endl;
    //    std::cout << "Its index position is : " << calculator->GetIndexOfMinimum() << std::endl;

    //    float maximumResult = calculator->GetMaximum();
    //    std::cout << "The Maximum intensity value is : " << maximumResult << std::endl;
    //    std::cout << "Its index position is : " << calculator->GetIndexOfMaximum() << std::endl;
    //    std::cout << "Real Max" << xcoutputImage->GetPixel(calculator->GetIndexOfMaximum()) << std::endl;

    newXYOrigin[0] = float(calculator->GetIndexOfMaximum()[0]) - float(fixedImageWindow2->GetLargestPossibleRegion().GetSize()[0]);
    newXYOrigin[1] = float(calculator->GetIndexOfMaximum()[1]) - float(fixedImageWindow2->GetLargestPossibleRegion().GetSize()[1]);

    // xcoutputImage->SetPixel(calculator->GetIndexOfMaximum(), 0); //just testing to make the brightest pixel dark so I could see which one it is

    //    typedef itk::ImageFileWriter< ImageProcessing::FloatImageType > nWriterType;
    //    nWriterType::Pointer writer3 = nWriterType::New();
    //    writer3->SetFileName( "/Users/megnashah/Desktop/imageXC.tiff");
    //    writer3->SetInput( xcoutputImage );
    //    writer3->Update();

    return newXYOrigin;
}



//This helper function takes the tile list and creates a new vector that orders the tiles as though they are in comb order. So a tile set collected
//in a comb fashion (along the rows first) will have the values in the new vector match the original index. This is a helper so that we can always stitch the
//tiles the same way regardless of how they were collected.

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<size_t> DetermineStitching::ReturnIndexForCombOrder(QVector<qint32> xTileList, QVector<qint32> yTileList, size_t numXtiles, size_t numYtiles)
{

    QVector<size_t> newIndices(xTileList.size(), 0);
    size_t count;

    for (size_t iter = 0; iter < xTileList.size(); iter++)
    {
        count = 0;
        for (size_t j=0; j<numYtiles; j++)
        {
            for (size_t i=0; i<numXtiles; i++)
            {
                if (xTileList[iter] == i && yTileList[iter] == j)
                {
                    newIndices[iter] = count;
                }
                count ++;
            }
        }
    }

    return newIndices;

}
