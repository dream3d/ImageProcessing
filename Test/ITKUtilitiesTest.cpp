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

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QDateTime>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/DataArrays/DataArray.hpp"
#include "DREAM3DLib/Common/FilterPipeline.h"
#include "DREAM3DLib/Common/FilterManager.h"
#include "DREAM3DLib/Common/FilterFactory.hpp"
#include "DREAM3DLib/Plugin/IDREAM3DPlugin.h"
#include "DREAM3DLib/Plugin/DREAM3DPluginLoader.h"
#include "DREAM3DLib/Utilities/UnitTestSupport.hpp"
#include "DREAM3DLib/Utilities/QMetaObjectUtilities.h"

#include "ImageProcessing/ImageProcessingConstants.h"
#include "ImageProcessingFilters/ItkBridge.h"



/**
* These functions are just here to make sure the templated static functions in the ItkBridge class
*  will properly compile. If these are actually executed the program would surely crash
* due to the use of the NULL pointers every where.
*/

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveTestFiles()
{
#if REMOVE_TEST_FILES
  //QFile::remove();
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TestDream3DtoITKImportFilter()
{

  ImageProcessing::DefaultImageType::Pointer ptr;


  DataContainer::Pointer m = DataContainer::New();
  ImageGeom::Pointer image = ImageGeom::CreateGeometry(DREAM3D::Geometry::ImageGeometry);
  m->setGeometry(image);
  QString attrMatName("CellData");
  ImageProcessing::DefaultPixelType* data = NULL;

  ImageProcessing::ImportUInt8FilterType::Pointer importFilter = ItkBridge<ImageProcessing::DefaultPixelType>::Dream3DtoITKImportFilter<ImageProcessing::DefaultPixelType>(m, attrMatName, data);

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TestCreateItkWrapperForDataPointer()
{
  ImageProcessing::DefaultImageType::Pointer imagePtr;
  DataContainer::Pointer m = DataContainer::New();
  ImageGeom::Pointer image = ImageGeom::CreateGeometry(DREAM3D::Geometry::ImageGeometry);
  m->setGeometry(image);
  QString attrMatName("CellData");

  ImageProcessing::DefaultPixelType* data = NULL;

  imagePtr = (ImageProcessing::DefaultImageType::Pointer)ItkBridge<ImageProcessing::DefaultPixelType>::CreateItkWrapperForDataPointer(m, attrMatName, data);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TestSetITKOutput()
{

  ImageProcessing::DefaultImageType::Pointer imagePtr;
// ImageProcessing::DefaultPixelType* output = NULL;
// unsigned int totalPoints = 0;
  ImageProcessing::DefaultArrayType::Pointer array;
  ItkBridge<ImageProcessing::DefaultPixelType>::SetITKFilterOutput(imagePtr, array);

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TestCopyITKtoDream3D()
{

  ImageProcessing::DefaultImageType::Pointer imagePtr;
  ImageProcessing::DefaultPixelType* data = NULL;
  ItkBridge<ImageProcessing::DefaultPixelType>::CopyITKtoDream3D(imagePtr, data);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TestExtractSlice()
{
  //itk::Image<ImageProcessing::DefaultPixelType,ImageProcessing::SliceDimension>::Pointer slicePtr;
  ImageProcessing::DefaultSliceType::Pointer slicePtr;
  ImageProcessing::DefaultImageType::Pointer image;
  slicePtr = ItkBridge<ImageProcessing::DefaultPixelType>::ExtractSlice(image, 0, 0);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void TestSetSlice()
{
  // Lets use a typedef to shorten up the "type" that is used in the templates
  //typedef ImageProcessing::DefaultPixelType PixelType;
  // Now declare our Image and Slice variables
  ImageProcessing::DefaultImageType::Pointer imagePtr;
  ImageProcessing::DefaultSliceType::Pointer slicePtr;
  ItkBridge<ImageProcessing::DefaultPixelType>::SetSlice(imagePtr, slicePtr, 0, 0);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loadFilterPlugins()
{
  // Register all the filters including trying to load those from Plugins
  FilterManager* fm = FilterManager::Instance();
  DREAM3DPluginLoader::LoadPluginFilters(fm);

  // Send progress messages from PipelineBuilder to this object for display
  QMetaObjectUtilities::RegisterMetaTypes();
}

// -----------------------------------------------------------------------------
//  Use test framework
// -----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  // Instantiate the QCoreApplication that we need to get the current path and load plugins.
  QCoreApplication app(argc, argv);
  QCoreApplication::setOrganizationName("BlueQuartz Software");
  QCoreApplication::setOrganizationDomain("bluequartz.net");
  QCoreApplication::setApplicationName("ITKUtilitiesTest");

  int err = EXIT_SUCCESS;
  DREAM3D_REGISTER_TEST( loadFilterPlugins() );

  DREAM3D_REGISTER_TEST( TestSetSlice() );
  DREAM3D_REGISTER_TEST( TestExtractSlice() )
  DREAM3D_REGISTER_TEST( TestCopyITKtoDream3D() )
  DREAM3D_REGISTER_TEST( TestSetITKOutput() )
  DREAM3D_REGISTER_TEST( TestCreateItkWrapperForDataPointer() )
  DREAM3D_REGISTER_TEST( TestDream3DtoITKImportFilter() )


  DREAM3D_REGISTER_TEST( RemoveTestFiles() )
  PRINT_TEST_SUMMARY();
  return err;
}



