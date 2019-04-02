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

//#include <vector>
#include <QtCore/QString>

#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/DataArrays/IDataArray.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/SIMPLib.h"

#include "ImageProcessing/ImageProcessingConstants.h"

//#include "TemplateUtilities.h"

#include "ImageProcessing/ImageProcessingDLLExport.h"

/**
 * @class ItkReadImage ItkReadImage.h ImageProcessing/ImageProcessingFilters/ItkReadImage.h
 * @brief
 * @author
 * @date
 * @version 1.0
 */
class ImageProcessing_EXPORT ItkReadImage : public AbstractFilter
{
    Q_OBJECT
    //    PYB11_CREATE_BINDINGS(ItkReadImage SUPERCLASS AbstractFilter)
    //    PYB11_PROPERTY(QString InputFileName READ getInputFileName WRITE setInputFileName)
    //    PYB11_PROPERTY(DataArrayPath DataContainerName READ getDataContainerName WRITE setDataContainerName)
    //    PYB11_PROPERTY(QString CellAttributeMatrixName READ getCellAttributeMatrixName WRITE setCellAttributeMatrixName)
    //    PYB11_PROPERTY(QString ImageDataArrayName READ getImageDataArrayName WRITE setImageDataArrayName)

  public:
    SIMPL_SHARED_POINTERS(ItkReadImage)
    SIMPL_FILTER_NEW_MACRO(ItkReadImage)
    SIMPL_TYPE_MACRO_SUPER_OVERRIDE(ItkReadImage, AbstractFilter)

    ~ItkReadImage() override;

    SIMPL_FILTER_PARAMETER(QString, InputFileName)
    Q_PROPERTY(QString InputFileName READ getInputFileName WRITE setInputFileName)

    SIMPL_FILTER_PARAMETER(DataArrayPath, DataContainerName)
    Q_PROPERTY(DataArrayPath DataContainerName READ getDataContainerName WRITE setDataContainerName)

    SIMPL_FILTER_PARAMETER(QString, CellAttributeMatrixName)
    Q_PROPERTY(QString CellAttributeMatrixName READ getCellAttributeMatrixName WRITE setCellAttributeMatrixName)

    SIMPL_FILTER_PARAMETER(QString, ImageDataArrayName)
    Q_PROPERTY(QString ImageDataArrayName READ getImageDataArrayName WRITE setImageDataArrayName)

    /**
     * @brief getCompiledLibraryName Returns the name of the Library that this filter is a part of
     * @return
     */
    const QString getCompiledLibraryName() const override;

    /**
    * @brief This returns a string that is displayed in the GUI. It should be readable
    * and understandable by humans.
    */
    const QString getHumanLabel() const override;

    /**
    * @brief This returns the group that the filter belonds to. You can select
    * a different group if you want. The string returned here will be displayed
    * in the GUI for the filter
    */
    const QString getGroupName() const override;

    /**
    * @brief This returns a string that is displayed in the GUI and helps to sort the filters into
    * a subgroup. It should be readable and understandable by humans.
    */
    const QString getSubGroupName() const override;

    /**
     * @brief getUuid Return the unique identifier for this filter.
     * @return A QUuid object.
     */
    const QUuid getUuid() override;

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

    // virtual void ItkReadImage::template_execute();

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
    ItkReadImage();

    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck();

    /**
     * @brief Initializes all the private instance variables.
     */
    void initialize();


  private:
    DEFINE_IDATAARRAY_VARIABLE(ImageData)

  public:
    ItkReadImage(const ItkReadImage&) = delete;   // Copy Constructor Not Implemented
    ItkReadImage(ItkReadImage&&) = delete;        // Move Constructor Not Implemented
    ItkReadImage& operator=(const ItkReadImage&) = delete; // Copy Assignment Not Implemented
    ItkReadImage& operator=(ItkReadImage&&) = delete;      // Move Assignment Not Implemented
};

