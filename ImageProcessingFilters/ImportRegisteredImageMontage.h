/*
 * Your License or Copyright Information can go here
 */

#ifndef _importregisteredimagemontage_h_
#define _importregisteredimagemontage_h_

#include <QtCore/QFile>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Common/AbstractFilter.h"
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/DataArrays/StringDataArray.hpp"
#include "SIMPLib/FilterParameters/FloatVec3FilterParameter.h"
#include "SIMPLib/FilterParameters/FileListInfoFilterParameter.h"

/**
 * @brief The ImportRegisteredImageMontage class. See [Filter documentation](@ref importregisteredimagemontage) for details.
 */
class ImportRegisteredImageMontage : public AbstractFilter
{
    Q_OBJECT /* Need this for Qt's signals and slots mechanism to work */
  public:
    SIMPL_SHARED_POINTERS(ImportRegisteredImageMontage)
    SIMPL_STATIC_NEW_MACRO(ImportRegisteredImageMontage)
    SIMPL_TYPE_MACRO_SUPER(ImportRegisteredImageMontage, AbstractFilter)

    virtual ~ImportRegisteredImageMontage();

    SIMPL_FILTER_PARAMETER(QString, DataContainerName)
    Q_PROPERTY(QString DataContainerName READ getDataContainerName WRITE setDataContainerName)

    SIMPL_FILTER_PARAMETER(QString, CellAttributeMatrixName)
    Q_PROPERTY(QString CellAttributeMatrixName READ getCellAttributeMatrixName WRITE setCellAttributeMatrixName)

    SIMPL_FILTER_PARAMETER(QString, MetaDataAttributeMatrixName)
    Q_PROPERTY(QString MetaDataAttributeMatrixName READ getMetaDataAttributeMatrixName WRITE setMetaDataAttributeMatrixName)

    SIMPL_FILTER_PARAMETER(FloatVec3_t, Origin)
    Q_PROPERTY(FloatVec3_t Origin READ getOrigin WRITE setOrigin)

    SIMPL_FILTER_PARAMETER(FloatVec3_t, Resolution)
    Q_PROPERTY(FloatVec3_t Resolution READ getResolution WRITE setResolution)

    //SIMPL_FILTER_PARAMETER(QString, RegistrationFile)
    //Q_PROPERTY(QString RegistrationFile READ getRegistrationFile WRITE setRegistrationFile)

    SIMPL_FILTER_PARAMETER(FileListInfo_t, InputFileListInfo)
    Q_PROPERTY(FileListInfo_t InputFileListInfo READ getInputFileListInfo WRITE setInputFileListInfo)

    SIMPL_FILTER_PARAMETER(QString, RegistrationCoordinatesArrayName)
    Q_PROPERTY(QString RegistrationCoordinatesArrayName READ getRegistrationCoordinatesArrayName WRITE setRegistrationCoordinatesArrayName)

    SIMPL_FILTER_PARAMETER(QString, AttributeArrayNamesArrayName)
    Q_PROPERTY(QString AttributeArrayNamesArrayName READ getAttributeArrayNamesArrayName WRITE setAttributeArrayNamesArrayName)

    /**
     * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getCompiledLibraryName();

    /**
     * @brief getBrandingString Returns the branding string for the filter, which is a tag
     * used to denote the filter's association with specific plugins
     * @return Branding string
    */
    virtual const QString getBrandingString();

    /**
     * @brief getFilterVersion Returns a version string for this filter. Default
     * value is an empty string.
     * @return
     */
    virtual const QString getFilterVersion();

    /**
     * @brief newFilterInstance Reimplemented from @see AbstractFilter class
     */
    virtual AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters);

    /**
     * @brief getGroupName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getGroupName();

    /**
     * @brief getSubGroupName Reimplemented from @see AbstractFilter class
     */
    virtual const QString getSubGroupName();

    /**
     * @brief getHumanLabel Reimplemented from @see AbstractFilter class
     */
    virtual const QString getHumanLabel();

    /**
     * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual void setupFilterParameters();


    /**
     * @brief readFilterParameters Reimplemented from @see AbstractFilter class
     */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

    /**
     * @brief execute Reimplemented from @see AbstractFilter class
     */
    virtual void execute();

    /**
    * @brief preflight Reimplemented from @see AbstractFilter class
    */
    virtual void preflight();

  signals:
    /**
      * @brief updateFilterParameters Emitted when the Filter requests all the latest Filter parameters
     * be pushed from a user-facing control (such as a widget)
     * @param filter Filter instance pointer
     */
    void updateFilterParameters(AbstractFilter* filter);

    /**
     * @brief parametersChanged Emitted when any Filter parameter is changed internally
     */
    void parametersChanged();

    /**
     * @brief preflightAboutToExecute Emitted just before calling dataCheck()
     */
    void preflightAboutToExecute();

    /**
     * @brief preflightExecuted Emitted just after calling dataCheck()
     */
    void preflightExecuted();

  protected:
    ImportRegisteredImageMontage();

    /**
     * @brief parseRegistrationFile Parses the ASCII file with the registration coordinates
     * @param reader QFile to read
     * @return Integer error value
     */
    int32_t parseRegistrationFile(QFile& reader);

    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck();

  private:
    DEFINE_DATAARRAY_VARIABLE(float, RegistrationCoordinates)

    StringDataArray::WeakPointer m_AttributeArrayNamesPtr;
    QFile m_InStream;
    int32_t m_NumImages;
    QVector<QString> m_ArrayNames;
    QVector<float> m_Coords;

    ImportRegisteredImageMontage(const ImportRegisteredImageMontage&); // Copy Constructor Not Implemented
    void operator=(const ImportRegisteredImageMontage&); // Operator '=' Not Implemented
};

#endif /* ImportRegisteredImageMontage_H_ */
