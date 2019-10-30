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

#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/SIMPLib.h"

class IDataArray;
using IDataArrayWkPtrType = std::weak_ptr<IDataArray>;

/**
 * @class AlignSectionsPhaseCorrelation AlignSectionsPhaseCorrelation.h ImageProcessing/ImageProcessingFilters/AlignSectionsPhaseCorrelation.h
 * @brief aligns sections using phase correlation
 * @author will lenthe
 * @date 9/1/2014
 * @version 1.0
 */
class AlignSectionsPhaseCorrelation : public AlignSections
{
    Q_OBJECT
    //    PYB11_CREATE_BINDINGS(ItkAlignSectionsPhaseCorrelation SUPERCLASS AlignSections)
    //    PYB11_PROPERTY(DataArrayPath SelectedCellArrayPath READ getSelectedCellArrayPath WRITE setSelectedCellArrayPath)
    //    PYB11_PROPERTY(QString InputFile READ getInputFile WRITE setInputFile)
  public:
    using Self = AlignSectionsPhaseCorrelation;
    using Pointer = std::shared_ptr<Self>;
    using ConstPointer = std::shared_ptr<const Self>;
    using WeakPointer = std::weak_ptr<Self>;
    using ConstWeakPointer = std::weak_ptr<Self>;
    static Pointer NullPointer();

    static std::shared_ptr<AlignSectionsPhaseCorrelation> New();

    /**
     * @brief Returns the name of the class for AlignSectionsPhaseCorrelation
     */
    QString getNameOfClass() const override;
    /**
     * @brief Returns the name of the class for AlignSectionsPhaseCorrelation
     */
    static QString ClassName();

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

    virtual ~AlignSectionsPhaseCorrelation();

    /**
     * @brief Setter property for InputFile
     */
    void setInputFile(const QString& value);
    /**
     * @brief Getter property for InputFile
     * @return Value of InputFile
     */
    QString getInputFile() const;

    Q_PROPERTY(QString InputFile READ getInputFile WRITE setInputFile)

    QString getCompiledLibraryName() const override;
    AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;
    QString getGroupName() const override;
    QString getSubGroupName() const override;

    /**
     * @brief getUuid Return the unique identifier for this filter.
     * @return A QUuid object.
     */
    QUuid getUuid() const override;
    QString getHumanLabel() const override;
    virtual const QString getBrandingString() { return "DREAM3D Reconstruction Plugin"; }

    /**
     * @brief Reimplemented from @see AbstractFilter class
     */
    void execute() override;
    void preflight() override;

  protected:
    AlignSectionsPhaseCorrelation();

    virtual void find_shifts(QVector<int>& xshifts, QVector<int>& yshifts);

    void setupFilterParameters() override;
    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);
    void readFilterParameters(AbstractFilterParametersReader* reader, int index) override;

  private:
    IDataArrayWkPtrType m_SelectedCellArrayPtr;
    void* m_SelectedCellArray = nullptr;

    DataArrayPath m_SelectedCellArrayPath = {};
    QString m_InputFile = {};

    void dataCheck();

    AlignSectionsPhaseCorrelation(const AlignSectionsPhaseCorrelation&) = delete; // Copy Constructor Not Implemented
    void operator=(const AlignSectionsPhaseCorrelation&) = delete;                // Move assignment Not Implemented
};

