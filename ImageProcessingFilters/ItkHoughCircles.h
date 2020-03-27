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

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/DataArrays/DataArray.hpp"

#include "ImageProcessing/ImageProcessingConstants.h"

#include "ImageProcessing/ImageProcessingDLLExport.h"

/**
 * @class HoughCircles HoughCircles.h ImageProcessing/ImageProcessingFilters/HoughCircles.h
 * @brief
 * @author
 * @date
 * @version 1.0
 */
class ImageProcessing_EXPORT ItkHoughCircles : public AbstractFilter
{
    Q_OBJECT

    // Start Python bindings declarations
    PYB11_BEGIN_BINDINGS(ItkHoughCircles SUPERCLASS AbstractFilter)
    PYB11_SHARED_POINTERS(ItkHoughCircles)
    PYB11_FILTER_NEW_MACRO(ItkHoughCircles)
    PYB11_PROPERTY(DataArrayPath SelectedCellArrayPath READ getSelectedCellArrayPath WRITE setSelectedCellArrayPath)
    PYB11_PROPERTY(QString NewCellArrayName READ getNewCellArrayName WRITE setNewCellArrayName)
    PYB11_PROPERTY(bool SaveAsNewArray READ getSaveAsNewArray WRITE setSaveAsNewArray)
    PYB11_PROPERTY(float MinRadius READ getMinRadius WRITE setMinRadius)
    PYB11_PROPERTY(float MaxRadius READ getMaxRadius WRITE setMaxRadius)
    PYB11_PROPERTY(int NumberCircles READ getNumberCircles WRITE setNumberCircles)
    PYB11_END_BINDINGS()
    // End Python bindings declarations

  public:
    using Self = ItkHoughCircles;
    using Pointer = std::shared_ptr<Self>;
    using ConstPointer = std::shared_ptr<const Self>;
    using WeakPointer = std::weak_ptr<Self>;
    using ConstWeakPointer = std::weak_ptr<const Self>;
    static Pointer NullPointer();

    static std::shared_ptr<ItkHoughCircles> New();

    /**
     * @brief Returns the name of the class for ItkHoughCircles
     */
    QString getNameOfClass() const override;
    /**
     * @brief Returns the name of the class for ItkHoughCircles
     */
    static QString ClassName();

    ~ItkHoughCircles() override;

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
     * @brief Setter property for NewCellArrayName
     */
    void setNewCellArrayName(const QString& value);
    /**
     * @brief Getter property for NewCellArrayName
     * @return Value of NewCellArrayName
     */
    QString getNewCellArrayName() const;

    Q_PROPERTY(QString NewCellArrayName READ getNewCellArrayName WRITE setNewCellArrayName)

    /**
     * @brief Setter property for SaveAsNewArray
     */
    void setSaveAsNewArray(bool value);
    /**
     * @brief Getter property for SaveAsNewArray
     * @return Value of SaveAsNewArray
     */
    bool getSaveAsNewArray() const;

    Q_PROPERTY(bool SaveAsNewArray READ getSaveAsNewArray WRITE setSaveAsNewArray)
    /**
     * @brief Setter property for MinRadius
     */
    void setMinRadius(float value);
    /**
     * @brief Getter property for MinRadius
     * @return Value of MinRadius
     */
    float getMinRadius() const;

    Q_PROPERTY(float MinRadius READ getMinRadius WRITE setMinRadius)
    /**
     * @brief Setter property for MaxRadius
     */
    void setMaxRadius(float value);
    /**
     * @brief Getter property for MaxRadius
     * @return Value of MaxRadius
     */
    float getMaxRadius() const;

    Q_PROPERTY(float MaxRadius READ getMaxRadius WRITE setMaxRadius)
    /**
     * @brief Setter property for NumberCircles
     */
    void setNumberCircles(int value);
    /**
     * @brief Getter property for NumberCircles
     * @return Value of NumberCircles
     */
    int getNumberCircles() const;

    Q_PROPERTY(int NumberCircles READ getNumberCircles WRITE setNumberCircles)

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
    ItkHoughCircles();

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
    ImageProcessingConstants::DefaultPixelType* m_SelectedCellArray = nullptr;
    std::weak_ptr<DataArray<ImageProcessingConstants::DefaultPixelType>> m_NewCellArrayPtr;
    ImageProcessingConstants::DefaultPixelType* m_NewCellArray = nullptr;

    DataArrayPath m_SelectedCellArrayPath = {};
    QString m_NewCellArrayName = {};
    bool m_SaveAsNewArray = {};
    float m_MinRadius = {};
    float m_MaxRadius = {};
    int m_NumberCircles = {};

  public:
    ItkHoughCircles(const ItkHoughCircles&) = delete; // Copy Constructor Not Implemented
    ItkHoughCircles(ItkHoughCircles&&) = delete;      // Move Constructor Not Implemented
    ItkHoughCircles& operator=(const ItkHoughCircles&) = delete; // Copy Assignment Not Implemented
    ItkHoughCircles& operator=(ItkHoughCircles&&) = delete;      // Move Assignment Not Implemented
};

