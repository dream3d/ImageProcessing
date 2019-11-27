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
 * @class ImageCalculator ImageCalculator.h ImageProcessing/ImageProcessingFilters/ImageCalculator.h
 * @brief
 * @author
 * @date
 * @version 1.0
 */
class ImageProcessing_EXPORT ItkImageCalculator : public AbstractFilter
{
    Q_OBJECT

#ifdef SIMPL_ENABLE_PYTHON
    PYB11_CREATE_BINDINGS(ItkImageCalculator SUPERCLASS AbstractFilter)
    PYB11_SHARED_POINTERS(ItkImageCalculator)
    PYB11_FILTER_NEW_MACRO(ItkImageCalculator)
    PYB11_FILTER_PARAMETER(DataArrayPath, SelectedCellArrayPath1)
    PYB11_FILTER_PARAMETER(DataArrayPath, SelectedCellArrayPath2)
    PYB11_FILTER_PARAMETER(QString, NewCellArrayName)
    PYB11_FILTER_PARAMETER(unsigned int, Operator)
    PYB11_PROPERTY(DataArrayPath SelectedCellArrayPath1 READ getSelectedCellArrayPath1 WRITE setSelectedCellArrayPath1)
    PYB11_PROPERTY(DataArrayPath SelectedCellArrayPath2 READ getSelectedCellArrayPath2 WRITE setSelectedCellArrayPath2)
    PYB11_PROPERTY(QString NewCellArrayName READ getNewCellArrayName WRITE setNewCellArrayName)
    PYB11_PROPERTY(unsigned int Operator READ getOperator WRITE setOperator)
#endif

  public:
    using Self = ItkImageCalculator;
    using Pointer = std::shared_ptr<Self>;
    using ConstPointer = std::shared_ptr<const Self>;
    using WeakPointer = std::weak_ptr<Self>;
    using ConstWeakPointer = std::weak_ptr<const Self>;
    static Pointer NullPointer();

    static std::shared_ptr<ItkImageCalculator> New();

    /**
     * @brief Returns the name of the class for ItkImageCalculator
     */
    QString getNameOfClass() const override;
    /**
     * @brief Returns the name of the class for ItkImageCalculator
     */
    static QString ClassName();

    ~ItkImageCalculator() override;

    /**
     * @brief Setter property for SelectedCellArrayPath1
     */
    void setSelectedCellArrayPath1(const DataArrayPath& value);
    /**
     * @brief Getter property for SelectedCellArrayPath1
     * @return Value of SelectedCellArrayPath1
     */
    DataArrayPath getSelectedCellArrayPath1() const;

    Q_PROPERTY(DataArrayPath SelectedCellArrayPath1 READ getSelectedCellArrayPath1 WRITE setSelectedCellArrayPath1)

    /**
     * @brief Setter property for SelectedCellArrayPath2
     */
    void setSelectedCellArrayPath2(const DataArrayPath& value);
    /**
     * @brief Getter property for SelectedCellArrayPath2
     * @return Value of SelectedCellArrayPath2
     */
    DataArrayPath getSelectedCellArrayPath2() const;

    Q_PROPERTY(DataArrayPath SelectedCellArrayPath2 READ getSelectedCellArrayPath2 WRITE setSelectedCellArrayPath2)

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
     * @brief Setter property for Operator
     */
    void setOperator(unsigned int value);
    /**
     * @brief Getter property for Operator
     * @return Value of Operator
     */
    unsigned int getOperator() const;

    Q_PROPERTY(unsigned int Operator READ getOperator WRITE setOperator)

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

  signals:
    /**
     * @brief updateFilterParameters This is emitted when the filter requests all the latest Filter Parameters need to be
     * pushed from a user facing control such as the FilterParameter Widget
     * @param filter The filter to push the values into
     */
    void updateFilterParameters(AbstractFilter* filter);

    /**
     * @brief parametersChanged This signal can be emitted when any of the filter parameters are changed internally.
     */
    void parametersChanged();

    /**
     * @brief preflightAboutToExecute Emitted just before the dataCheck() is called. This can change if needed.
     */
    void preflightAboutToExecute();

    /**
     * @brief preflightExecuted Emitted just after the dataCheck() is called. Typically. This can change if needed.
     */
    void preflightExecuted();

  protected:
    ItkImageCalculator();

    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck();

    /**
     * @brief Initializes all the private instance variables.
     */
    void initialize();


  private:
    std::weak_ptr<DataArray<ImageProcessingConstants::DefaultPixelType>> m_SelectedCellArray1Ptr;
    ImageProcessingConstants::DefaultPixelType* m_SelectedCellArray1 = nullptr;
    std::weak_ptr<DataArray<ImageProcessingConstants::DefaultPixelType>> m_SelectedCellArray2Ptr;
    ImageProcessingConstants::DefaultPixelType* m_SelectedCellArray2 = nullptr;
    std::weak_ptr<DataArray<ImageProcessingConstants::DefaultPixelType>> m_NewCellArrayPtr;
    ImageProcessingConstants::DefaultPixelType* m_NewCellArray = nullptr;

    DataArrayPath m_SelectedCellArrayPath1 = {};
    DataArrayPath m_SelectedCellArrayPath2 = {};
    QString m_NewCellArrayName = {};
    unsigned int m_Operator = {};

  public:
    ItkImageCalculator(const ItkImageCalculator&) = delete; // Copy Constructor Not Implemented
    ItkImageCalculator(ItkImageCalculator&&) = delete;      // Move Constructor Not Implemented
    ItkImageCalculator& operator=(const ItkImageCalculator&) = delete; // Copy Assignment Not Implemented
    ItkImageCalculator& operator=(ItkImageCalculator&&) = delete;      // Move Assignment Not Implemented
};

