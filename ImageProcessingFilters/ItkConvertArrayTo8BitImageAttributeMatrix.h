/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
 * Copyright (c) 2013 Dr. Joseph C. Tucker (UES, Inc.)
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
 * Neither the name of Joseph C. Tucker, Michael A. Groeber, Michael A. Jackson,
 * UES, Inc., the US Air Force, BlueQuartz Software nor the names of its contributors
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
 *  This code was written under United States Air Force Contract number
 *                   FA8650-07-D-5800 and FA8650-10-D-5226
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <memory>

#include <QtCore/QString>
#include <set>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataArrays/IDataArray.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/DataArrays/DataArray.hpp"

#include "ImageProcessing/ImageProcessingDLLExport.h"

#include "ImageProcessing/ImageProcessingConstants.h"
/**
 * @class ConvertArrayTo8BitImageAttributeMatrix ConvertArrayTo8BitImageAttributeMatrix.h /FilterCategoryFilters/ConvertArrayTo8BitImageAttributeMatrix.h
 * @brief
 * @author
 * @date
 * @version 1.0
 */
class ImageProcessing_EXPORT ItkConvertArrayTo8BitImageAttributeMatrix : public AbstractFilter
{
    Q_OBJECT

    // Start Python bindings declarations
    PYB11_BEGIN_BINDINGS(ItkConvertArrayTo8BitImageAttributeMatrix SUPERCLASS AbstractFilter)
    PYB11_FILTER()
    PYB11_SHARED_POINTERS(ItkConvertArrayTo8BitImageAttributeMatrix)
    PYB11_FILTER_NEW_MACRO(ItkConvertArrayTo8BitImageAttributeMatrix)
    PYB11_PROPERTY(DataArrayPath AttributeMatrixName READ getAttributeMatrixName WRITE setAttributeMatrixName)
    PYB11_PROPERTY(QString NewArrayArrayName READ getNewArrayArrayName WRITE setNewArrayArrayName)
    PYB11_END_BINDINGS()
    // End Python bindings declarations

  public:
    using Self = ItkConvertArrayTo8BitImageAttributeMatrix;
    using Pointer = std::shared_ptr<Self>;
    using ConstPointer = std::shared_ptr<const Self>;
    using WeakPointer = std::weak_ptr<Self>;
    using ConstWeakPointer = std::weak_ptr<const Self>;
    static Pointer NullPointer();

    static std::shared_ptr<ItkConvertArrayTo8BitImageAttributeMatrix> New();

    /**
     * @brief Returns the name of the class for ItkConvertArrayTo8BitImageAttributeMatrix
     */
    QString getNameOfClass() const override;
    /**
     * @brief Returns the name of the class for ItkConvertArrayTo8BitImageAttributeMatrix
     */
    static QString ClassName();

    ~ItkConvertArrayTo8BitImageAttributeMatrix() override;

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
     * @brief Setter property for NewArrayArrayName
     */
    void setNewArrayArrayName(const QString& value);
    /**
     * @brief Getter property for NewArrayArrayName
     * @return Value of NewArrayArrayName
     */
    QString getNewArrayArrayName() const;

    Q_PROPERTY(QString NewArrayArrayName READ getNewArrayArrayName WRITE setNewArrayArrayName)

    /**
     * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
     */
    QString getCompiledLibraryName() const override;

    /**
     * @brief getBrandingString Returns the branding string for the filter, which is a tag
     * used to denote the filter's association with specific plugins
     * @return Branding string
    */
    QString getBrandingString() const override;

    /**
     * @brief getFilterVersion Returns a version string for this filter. Default
     * value is an empty string.
     * @return
     */
    QString getFilterVersion() const override;

    /**
     * @brief newFilterInstance Reimplemented from @see AbstractFilter class
     */
    AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

    /**
     * @brief getGroupName Reimplemented from @see AbstractFilter class
     */
    QString getGroupName() const override;

    /**
     * @brief getSubGroupName Reimplemented from @see AbstractFilter class
     */
    QString getSubGroupName() const override;

    /**
     * @brief getUuid Return the unique identifier for this filter.
     * @return A QUuid object.
     */
    QUuid getUuid() const override;

    /**
     * @brief getHumanLabel Reimplemented from @see AbstractFilter class
     */
    QString getHumanLabel() const override;

    /**
     * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
     */
    void setupFilterParameters() override;

    /**
    * @brief This method will read the options from a file
    * @param reader The reader that is used to read the options from a file
    */
    void readFilterParameters(AbstractFilterParametersReader* reader, int index) override;

    /**
    * @brief Reimplemented from @see AbstractFilter class
    */
    void execute() override;


  protected:
    ItkConvertArrayTo8BitImageAttributeMatrix();

    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck() override;

    /**
     * @brief Initializes all the private instance variables.
     */
    void initialize();


  private:
    std::weak_ptr<DataArray<uint8_t>> m_NewArrayPtr;
    uint8_t* m_NewArray = nullptr;

    DataArrayPath m_AttributeMatrixName = {};
    QString m_NewArrayArrayName = {};

  public:
    ItkConvertArrayTo8BitImageAttributeMatrix(const ItkConvertArrayTo8BitImageAttributeMatrix&) = delete; // Copy Constructor Not Implemented
    ItkConvertArrayTo8BitImageAttributeMatrix(ItkConvertArrayTo8BitImageAttributeMatrix&&) = delete;      // Move Constructor Not Implemented
    ItkConvertArrayTo8BitImageAttributeMatrix& operator=(const ItkConvertArrayTo8BitImageAttributeMatrix&) = delete; // Copy Assignment Not Implemented
    ItkConvertArrayTo8BitImageAttributeMatrix& operator=(ItkConvertArrayTo8BitImageAttributeMatrix&&) = delete;      // Move Assignment Not Implemented
};



