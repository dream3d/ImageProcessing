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

#pragma once

#include <memory>

#include <QtCore/QString>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/DataArrays/DataArray.hpp"

#include "ImageProcessing/ImageProcessingConstants.h"

#include "ImageProcessing/ImageProcessingDLLExport.h"

/**
 * @class Watershed Watershed.h ImageProcessing/ImageProcessingFilters/Watershed.h
 * @brief
 * @author
 * @date
 * @version 1.0
 */
class ImageProcessing_EXPORT ItkWatershed : public AbstractFilter
{
    Q_OBJECT

#ifdef SIMPL_ENABLE_PYTHON
    PYB11_CREATE_BINDINGS(ItkWatershed SUPERCLASS AbstractFilter)
    PYB11_SHARED_POINTERS(ItkWatershed)
    PYB11_FILTER_NEW_MACRO(ItkWatershed)
    PYB11_FILTER_PARAMETER(DataArrayPath, SelectedCellArrayPath)
    PYB11_FILTER_PARAMETER(QString, FeatureIdsArrayName)
    PYB11_FILTER_PARAMETER(float, Threshold)
    PYB11_FILTER_PARAMETER(float, Level)
    PYB11_PROPERTY(DataArrayPath SelectedCellArrayPath READ getSelectedCellArrayPath WRITE setSelectedCellArrayPath)
    PYB11_PROPERTY(QString FeatureIdsArrayName READ getFeatureIdsArrayName WRITE setFeatureIdsArrayName)
    PYB11_PROPERTY(float Threshold READ getThreshold WRITE setThreshold)
    PYB11_PROPERTY(float Level READ getLevel WRITE setLevel)
#endif

  public:
    using Self = ItkWatershed;
    using Pointer = std::shared_ptr<Self>;
    using ConstPointer = std::shared_ptr<const Self>;
    using WeakPointer = std::weak_ptr<Self>;
    using ConstWeakPointer = std::weak_ptr<const Self>;
    static Pointer NullPointer();

    static std::shared_ptr<ItkWatershed> New();

    /**
     * @brief Returns the name of the class for ItkWatershed
     */
    QString getNameOfClass() const override;
    /**
     * @brief Returns the name of the class for ItkWatershed
     */
    static QString ClassName();

    ~ItkWatershed() override;

    /**
     * @brief Setter property for SelectedCellArrayPath
     */
    void setSelectedCellArrayPath(const DataArrayPath& value);
    /**
     * @brief Getter property for SelectedCellArrayPath
     * @return Value of SelectedCellArrayPath
     */
    DataArrayPath getSelectedCellArrayPath() const;

    Q_PROPERTY(DataArrayPath SelectedCellArrayPath READ getSelectedCellArrayPath WRITE setSelectedCellArrayPath)

    /**
     * @brief Setter property for FeatureIdsArrayName
     */
    void setFeatureIdsArrayName(const QString& value);
    /**
     * @brief Getter property for FeatureIdsArrayName
     * @return Value of FeatureIdsArrayName
     */
    QString getFeatureIdsArrayName() const;

    Q_PROPERTY(QString FeatureIdsArrayName READ getFeatureIdsArrayName WRITE setFeatureIdsArrayName)

    /**
     * @brief Setter property for Threshold
     */
    void setThreshold(float value);
    /**
     * @brief Getter property for Threshold
     * @return Value of Threshold
     */
    float getThreshold() const;

    Q_PROPERTY(float Threshold READ getThreshold WRITE setThreshold)
    /**
     * @brief Setter property for Level
     */
    void setLevel(float value);
    /**
     * @brief Getter property for Level
     * @return Value of Level
     */
    float getLevel() const;

    Q_PROPERTY(float Level READ getLevel WRITE setLevel)

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
    * @brief This function runs some sanity checks on the DataContainer and inputs
    * in an attempt to ensure the filter can process the inputs.
    */
    void preflight() override;

    /**
     * @brief newFilterInstance Returns a new instance of the filter optionally copying the filter parameters from the
     * current filter to the new instance.
     * @param copyFilterParameters
     * @return
     */
    AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

  protected:
    ItkWatershed();

    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck();

    /**
     * @brief Initializes all the private instance variables.
     */
    void initialize();


  private:
    std::weak_ptr<DataArray<ImageProcessingConstants::DefaultPixelType>> m_SelectedCellArrayPtr;
    ImageProcessingConstants::DefaultPixelType* m_SelectedCellArray = nullptr;
    std::weak_ptr<DataArray<int32_t>> m_FeatureIdsPtr;
    int32_t* m_FeatureIds = nullptr;

    DataArrayPath m_SelectedCellArrayPath = {};
    QString m_FeatureIdsArrayName = {};
    float m_Threshold = {};
    float m_Level = {};

  public:
    ItkWatershed(const ItkWatershed&) = delete;   // Copy Constructor Not Implemented
    ItkWatershed(ItkWatershed&&) = delete;        // Move Constructor Not Implemented
    ItkWatershed& operator=(const ItkWatershed&) = delete; // Copy Assignment Not Implemented
    ItkWatershed& operator=(ItkWatershed&&) = delete;      // Move Assignment Not Implemented
};

