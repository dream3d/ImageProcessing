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
#pragma once

#include <memory>

#include <QtCore/QString>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataArrays/StringDataArray.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/DataArrays/DataArray.hpp"

#include "ImageProcessing/ImageProcessingConstants.h"

#include "ImageProcessing/ImageProcessingDLLExport.h"

/**
 * @class StitchImages StitchImages.h ImageProcessing/ImageProcessingFilters/StitchImages.h
 * @brief
 * @author
 * @date
 * @version 1.0
 */
class ImageProcessing_EXPORT ItkStitchImages : public AbstractFilter
{
    Q_OBJECT

    // Start Python bindings declarations
    PYB11_BEGIN_BINDINGS(ItkStitchImages SUPERCLASS AbstractFilter)
    PYB11_FILTER()
    PYB11_SHARED_POINTERS(ItkStitchImages)
    PYB11_FILTER_NEW_MACRO(ItkStitchImages)
    PYB11_PROPERTY(DataArrayPath AttributeMatrixName READ getAttributeMatrixName WRITE setAttributeMatrixName)
    PYB11_PROPERTY(DataArrayPath StitchedCoordinatesArrayPath READ getStitchedCoordinatesArrayPath WRITE setStitchedCoordinatesArrayPath)
    PYB11_PROPERTY(DataArrayPath AttributeArrayNamesPath READ getAttributeArrayNamesPath WRITE setAttributeArrayNamesPath)
    PYB11_PROPERTY(DataArrayPath StitchedVolumeDataContainerName READ getStitchedVolumeDataContainerName WRITE setStitchedVolumeDataContainerName)
    PYB11_PROPERTY(QString StitchedImagesArrayName READ getStitchedImagesArrayName WRITE setStitchedImagesArrayName)
    PYB11_PROPERTY(QString StitchedAttributeMatrixName READ getStitchedAttributeMatrixName WRITE setStitchedAttributeMatrixName)
    PYB11_END_BINDINGS()
    // End Python bindings declarations

  public:
    using Self = ItkStitchImages;
    using Pointer = std::shared_ptr<Self>;
    using ConstPointer = std::shared_ptr<const Self>;
    using WeakPointer = std::weak_ptr<Self>;
    using ConstWeakPointer = std::weak_ptr<const Self>;
    static Pointer NullPointer();

    static std::shared_ptr<ItkStitchImages> New();

    /**
     * @brief Returns the name of the class for ItkStitchImages
     */
    QString getNameOfClass() const override;
    /**
     * @brief Returns the name of the class for ItkStitchImages
     */
    static QString ClassName();

    ~ItkStitchImages() override;

    /**
     * @brief Setter property for AttributeMatrixName
     */
    void setAttributeMatrixName(const DataArrayPath& value);
    /**
     * @brief Getter property for AttributeMatrixName
     * @return Value of AttributeMatrixName
     */
    DataArrayPath getAttributeMatrixName() const;

    Q_PROPERTY(DataArrayPath AttributeMatrixName READ getAttributeMatrixName WRITE setAttributeMatrixName)

    /**
     * @brief Setter property for StitchedCoordinatesArrayPath
     */
    void setStitchedCoordinatesArrayPath(const DataArrayPath& value);
    /**
     * @brief Getter property for StitchedCoordinatesArrayPath
     * @return Value of StitchedCoordinatesArrayPath
     */
    DataArrayPath getStitchedCoordinatesArrayPath() const;

    Q_PROPERTY(DataArrayPath StitchedCoordinatesArrayPath READ getStitchedCoordinatesArrayPath WRITE setStitchedCoordinatesArrayPath)

    /**
     * @brief Setter property for AttributeArrayNamesPath
     */
    void setAttributeArrayNamesPath(const DataArrayPath& value);
    /**
     * @brief Getter property for AttributeArrayNamesPath
     * @return Value of AttributeArrayNamesPath
     */
    DataArrayPath getAttributeArrayNamesPath() const;

    Q_PROPERTY(DataArrayPath AttributeArrayNamesPath READ getAttributeArrayNamesPath WRITE setAttributeArrayNamesPath)

    /**
     * @brief Setter property for StitchedVolumeDataContainerName
     */
    void setStitchedVolumeDataContainerName(const DataArrayPath& value);
    /**
     * @brief Getter property for StitchedVolumeDataContainerName
     * @return Value of StitchedVolumeDataContainerName
     */
    DataArrayPath getStitchedVolumeDataContainerName() const;

    Q_PROPERTY(DataArrayPath StitchedVolumeDataContainerName READ getStitchedVolumeDataContainerName WRITE setStitchedVolumeDataContainerName)

    /**
     * @brief Setter property for StitchedImagesArrayName
     */
    void setStitchedImagesArrayName(const QString& value);
    /**
     * @brief Getter property for StitchedImagesArrayName
     * @return Value of StitchedImagesArrayName
     */
    QString getStitchedImagesArrayName() const;

    Q_PROPERTY(QString StitchedImagesArrayName READ getStitchedImagesArrayName WRITE setStitchedImagesArrayName)

    /**
     * @brief Setter property for StitchedAttributeMatrixName
     */
    void setStitchedAttributeMatrixName(const QString& value);
    /**
     * @brief Getter property for StitchedAttributeMatrixName
     * @return Value of StitchedAttributeMatrixName
     */
    QString getStitchedAttributeMatrixName() const;

    Q_PROPERTY(QString StitchedAttributeMatrixName READ getStitchedAttributeMatrixName WRITE setStitchedAttributeMatrixName)

    /**
     * @brief getCompiledLibraryName Returns the name of the Library that this filter is a part of
     * @return
     */
    QString getCompiledLibraryName() const override;

    /**
    * @brief This returns a string that is displayed in the GUI. It should be readable
    * and understandable by humans.
    */
    QString getHumanLabel() const override;

    /**
    * @brief This returns the group that the filter belonds to. You can select
    * a different group if you want. The string returned here will be displayed
    * in the GUI for the filter
    */
    QString getGroupName() const override;

    /**
    * @brief This returns a string that is displayed in the GUI and helps to sort the filters into
    * a subgroup. It should be readable and understandable by humans.
    */
    QString getSubGroupName() const override;

    /**
     * @brief getUuid Return the unique identifier for this filter.
     * @return A QUuid object.
     */
    QUuid getUuid() const override;

    /**
    * @brief This method will instantiate all the end user settable options/parameters
    * for this filter
    */
    void setupFilterParameters() override;

    /**
    * @brief This method will read the options from a file
    * @param reader The reader that is used to read the options from a file
    * @param index The index to read the information from
    */
    void readFilterParameters(AbstractFilterParametersReader* reader, int index) override;

    /**
     * @brief Reimplemented from @see AbstractFilter class
     */
    void execute() override;


    /**
     * @brief newFilterInstance Returns a new instance of the filter optionally copying the filter parameters from the
     * current filter to the new instance.
     * @param copyFilterParameters
     * @return
     */
    AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

  protected:
    ItkStitchImages();

    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck() override;

    /**
     * @brief Initializes all the private instance variables.
     */
    void initialize();


  private:
    std::weak_ptr<DataArray<ImageProcessingConstants::DefaultPixelType>> m_SelectedCellArrayPtr;

    std::weak_ptr<DataArray<float>> m_StitchedCoordinatesPtr;
    float* m_StitchedCoordinates = nullptr;
    std::weak_ptr<DataArray<ImageProcessingConstants::DefaultPixelType>> m_StitchedImageArrayPtr;
    ImageProcessingConstants::DefaultPixelType* m_StitchedImageArray = nullptr;

    DataArrayPath m_AttributeMatrixName = {};
    DataArrayPath m_StitchedCoordinatesArrayPath = {};
    DataArrayPath m_AttributeArrayNamesPath = {};
    DataArrayPath m_StitchedVolumeDataContainerName = {};
    QString m_StitchedImagesArrayName = {};
    QString m_StitchedAttributeMatrixName = {};

    StringDataArray::WeakPointer    m_AttributeArrayNamesPtr;

  public:
    ItkStitchImages(const ItkStitchImages&) = delete; // Copy Constructor Not Implemented
    ItkStitchImages(ItkStitchImages&&) = delete;      // Move Constructor Not Implemented
    ItkStitchImages& operator=(const ItkStitchImages&) = delete; // Copy Assignment Not Implemented
    ItkStitchImages& operator=(ItkStitchImages&&) = delete;      // Move Assignment Not Implemented
};

