/* ============================================================================
 * Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
 * Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
 * contributors may be used to endorse or promote products derived from this software
 * without specific prior written permission.
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
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#pragma once

#include <memory>

#include <QtCore/QString>
#include <vector>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/Filtering/AbstractFilter.h"

class IDataArray;
using IDataArrayWkPtrType = std::weak_ptr<IDataArray>;

#include "ImageProcessing/ImageProcessingConstants.h"

#include "ImageProcessing/ImageProcessingDLLExport.h"

/**
 * @class KMeans KMeans.h ImageProcessing/ImageProcessingFilters/KMeans.h
 * @brief
 * @author
 * @date
 * @version 1.0
 */
class ImageProcessing_EXPORT ItkKdTreeKMeans : public AbstractFilter
{
  Q_OBJECT

  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(ItkKdTreeKMeans SUPERCLASS AbstractFilter)
  PYB11_FILTER()
  PYB11_SHARED_POINTERS(ItkKdTreeKMeans)
  PYB11_FILTER_NEW_MACRO(ItkKdTreeKMeans)
  PYB11_PROPERTY(DataArrayPath SelectedCellArrayPath READ getSelectedCellArrayPath WRITE setSelectedCellArrayPath)
  PYB11_PROPERTY(QString NewCellArrayName READ getNewCellArrayName WRITE setNewCellArrayName)
  PYB11_PROPERTY(int Classes READ getClasses WRITE setClasses)
  PYB11_END_BINDINGS()
  // End Python bindings declarations

public:
  using Self = ItkKdTreeKMeans;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  static std::shared_ptr<ItkKdTreeKMeans> New();

  /**
   * @brief Returns the name of the class for ItkKdTreeKMeans
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for ItkKdTreeKMeans
   */
  static QString ClassName();

  ~ItkKdTreeKMeans() override;

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
   * @brief Setter property for Classes
   */
  void setClasses(int value);
  /**
   * @brief Getter property for Classes
   * @return Value of Classes
   */
  int getClasses() const;

  Q_PROPERTY(int Classes READ getClasses WRITE setClasses)

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
  ItkKdTreeKMeans();

  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck() override;

  /**
   * @brief Initializes all the private instance variables.
   */
  void initialize();

private:
  std::weak_ptr<DataArray<int32_t>> m_NewCellArrayPtr;
  int32_t* m_NewCellArray = nullptr;

  IDataArrayWkPtrType m_SelectedCellArrayPtr;

  DataArrayPath m_SelectedCellArrayPath = {"", "", ""};
  QString m_NewCellArrayName = {"ClassLabels"};
  int m_Classes = {2};

public:
  ItkKdTreeKMeans(const ItkKdTreeKMeans&) = delete;            // Copy Constructor Not Implemented
  ItkKdTreeKMeans(ItkKdTreeKMeans&&) = delete;                 // Move Constructor Not Implemented
  ItkKdTreeKMeans& operator=(const ItkKdTreeKMeans&) = delete; // Copy Assignment Not Implemented
  ItkKdTreeKMeans& operator=(ItkKdTreeKMeans&&) = delete;      // Move Assignment Not Implemented
};
