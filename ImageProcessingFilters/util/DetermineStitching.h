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
#pragma once

#include <vector>

#include <QtCore/QString>

#include "SIMPLib/ITK/itkSupportConstants.h"
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/DataArrays/IDataArray.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/SIMPLib.h"

/**
 * @brief The DetermineStitching class
 */
class DetermineStitching
{
  public:

    virtual ~DetermineStitching();

    /**
   * @brief FindGlobalOrigins
   * @param totalPoints
   * @param udims
   * @param sampleOrigin
   * @param voxelResolution
   * @param dataArrayList
   * @param xGlobCoordsList
   * @param yGlobCoordsList
   * @param xTileList
   * @param yTileList
   * @param obs
   * @return
   */
    static FloatArrayType::Pointer FindGlobalOriginsLegacy(size_t totalPoints,
                                                     QVector<size_t> udims,
                                                     float sampleOrigin[],
                                                     float voxelResolution[],
                                                     QVector<ImageProcessingConstants::DefaultPixelType *> dataArrayList,
                                                     QVector<float> xGlobCoordsList,
                                                     QVector<float> yGlobCoordsList,
                                                     QVector<qint32> xTileList,
                                                     QVector<qint32> yTileList,
                                                     AbstractFilter *filter = nullptr);

	static FloatArrayType::Pointer FindGlobalOrigins(int xTileCount, int yTileCount,
		int ImportMode,
		float overlapPer,
		QVector<ImageProcessingConstants::DefaultPixelType*> dataArrayList,
		QVector<size_t> udims,
		float sampleOrigin[],
		float voxelResolution[]);

    /**
   * @brief ReturnIndexForCombOrder
   * @param xTileList
   * @param yTileList
   * @param numXtiles
   * @param numYtiles
   * @return
   */
    static QVector<size_t> ReturnIndexForCombOrder(QVector<qint32> xTileList, QVector<qint32> yTileList, size_t numXtiles, size_t numYtiles);


	/**
	* @brief ReturnProperIndex
	* @param InputMode
	* @param Numtiles
	*/
  static QVector<size_t> ReturnProperIndex(int InputMode, int xDims, int yDims);

    /**
   * @brief CropAndCrossCorrelate
   * @param cropSpecsIm1Im2
   * @param currentImage
   * @param fixedImage
   * @return
   */
    static std::vector<float> CropAndCrossCorrelate(std::vector<float> cropSpecsIm1Im2, ImageProcessingConstants::UInt8ImageType* currentImage, ImageProcessingConstants::UInt8ImageType* fixedImage);

  protected:
    DetermineStitching();

  public:
    DetermineStitching(const DetermineStitching&) = delete; // Copy Constructor Not Implemented
    DetermineStitching(DetermineStitching&&) = delete;      // Move Constructor Not Implemented
    DetermineStitching& operator=(const DetermineStitching&) = delete; // Copy Assignment Not Implemented
    DetermineStitching& operator=(DetermineStitching&&) = delete;      // Move Assignment Not Implemented
};


