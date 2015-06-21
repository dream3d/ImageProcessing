/*
 * Your License or Copyright Information can go here
 */

#ifndef _DetermineStitchingCoordinatesGeneric_H_
#define _DetermineStitchingCoordinatesGeneric_H_


#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/Common/AbstractFilter.h"
#include "DREAM3DLib/DataArrays/StringDataArray.hpp"
#include "ImageProcessing/ImageProcessingConstants.h"


/**
 * @class DetermineStitchingCoordinatesGeneric DetermineStitchingCoordinatesGeneric.h ZeissImport/ZeissImportFilters/DetermineStitchingCoordinatesGeneric.h
 * @brief
 * @author
 * @date
 * @version 1.0
 */
class DetermineStitchingCoordinatesGeneric : public AbstractFilter
{
  Q_OBJECT /* Need this for Qt's signals and slots mechanism to work */

  public:
    DREAM3D_SHARED_POINTERS(DetermineStitchingCoordinatesGeneric)
    DREAM3D_STATIC_NEW_MACRO(DetermineStitchingCoordinatesGeneric)
    DREAM3D_TYPE_MACRO_SUPER(DetermineStitchingCoordinatesGeneric, AbstractFilter)

    virtual ~DetermineStitchingCoordinatesGeneric();

    /* Place your input parameters here using the DREAM3D macros to declare the Filter Parameters
     * or other instance variables
     */
    //DREAM3D_FILTER_PARAMETER(QString, ImagePrefix)
    /* If you declare a filter parameter above then you MUST create a Q_PROPERTY for that FilterParameter */
    //Q_PROPERTY(QString ImagePrefix READ getImagePrefix WRITE setImagePrefix)

    /* Here is another example of declaring an integer FilterParameter */
    // DREAM3D_FILTER_PARAMETER(int, ImageSize)
    // Q_PROPERTY(int ImageSize READ getImageSize WRITE setImageSize)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, AttributeMatrixName)
    Q_PROPERTY(DataArrayPath AttributeMatrixName READ getAttributeMatrixName WRITE setAttributeMatrixName)

    DREAM3D_FILTER_PARAMETER(bool, UseZeissMetaData)
    Q_PROPERTY(bool UseZeissMetaData READ getUseZeissMetaData WRITE setUseZeissMetaData)

    DREAM3D_FILTER_PARAMETER(DataArrayPath, MetaDataAttributeMatrixName)
    Q_PROPERTY(DataArrayPath MetaDataAttributeMatrixName READ getMetaDataAttributeMatrixName WRITE setMetaDataAttributeMatrixName)

    DREAM3D_FILTER_PARAMETER(QString, TileCalculatedInfoAttributeMatrixName)
    Q_PROPERTY(QString TileCalculatedInfoAttributeMatrixName READ getTileCalculatedInfoAttributeMatrixName WRITE setTileCalculatedInfoAttributeMatrixName)

    DREAM3D_FILTER_PARAMETER(QString, StitchedCoordinatesArrayName)
    Q_PROPERTY(QString StitchedCoordinatesArrayName READ getStitchedCoordinatesArrayName WRITE setStitchedCoordinatesArrayName)

    DREAM3D_FILTER_PARAMETER(QString, StitchedArrayNames)
    Q_PROPERTY(QString StitchedArrayNames READ getStitchedArrayNames WRITE setStitchedArrayNames)


    /**
     * @brief getCompiledLibraryName Returns the name of the Library that this filter is a part of
     * @return
     */
    virtual const QString getCompiledLibraryName();

    /**
    * @brief This returns a string that is displayed in the GUI. It should be readable
    * and understandable by humans.
    */
    virtual const QString getHumanLabel();

    /**
    * @brief This returns the group that the filter belonds to. You can select
    * a different group if you want. The string returned here will be displayed
    * in the GUI for the filter
    */
    virtual const QString getGroupName();

    /**
    * @brief This returns a string that is displayed in the GUI and helps to sort the filters into
    * a subgroup. It should be readable and understandable by humans.
    */
    virtual const QString getSubGroupName();

    /**
    * @brief This method will instantiate all the end user settable options/parameters
    * for this filter
    */
    virtual void setupFilterParameters();

    /**
    * @brief This method will write the options to a file
    * @param writer The writer that is used to write the options to a file
    * @param index The index that the data should be written to
    * @return Returns the next index for writing
    */
    virtual int writeFilterParameters(AbstractFilterParametersWriter* writer, int index);

    /**
    * @brief This method will read the options from a file
    * @param reader The reader that is used to read the options from a file
    * @param index The index to read the information from
    */
    virtual void readFilterParameters(AbstractFilterParametersReader* reader, int index);

   /**
    * @brief Reimplemented from @see AbstractFilter class
    */
    virtual void execute();

    /**
    * @brief This function runs some sanity checks on the DataContainer and inputs
    * in an attempt to ensure the filter can process the inputs.
    */
    virtual void preflight();

    /**
     * @brief newFilterInstance Returns a new instance of the filter optionally copying the filter parameters from the
     * current filter to the new instance.
     * @param copyFilterParameters
     * @return
     */
    virtual AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters);

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
    DetermineStitchingCoordinatesGeneric();

    QVector<qint32> extractIntegerValues(QString arrayName);
    QVector<float> extractFloatValues(QString arrayName);
    QVector<float> extractGlobalIndices(QString DataArrayName, QString resolution);

    /**
    * @brief Checks for the appropriate parameter values and availability of arrays in the data container
    */
    void dataCheck();

  private:
//    DEFINE_DATAARRAY_VARIABLE(QVector<IDataArray::Pointer>, PointerList)

    QVector<ImageProcessingConstants::DefaultPixelType* > m_PointerList;
    DEFINE_DATAARRAY_VARIABLE(ImageProcessingConstants::DefaultPixelType, SelectedCellArray)
    DEFINE_DATAARRAY_VARIABLE(float, StitchedCoordinates)
    StringDataArray::WeakPointer m_DataArrayNamesForStitchedCoordinatesPtr;


    DetermineStitchingCoordinatesGeneric(const DetermineStitchingCoordinatesGeneric&); // Copy Constructor Not Implemented
    void operator=(const DetermineStitchingCoordinatesGeneric&); // Operator '=' Not Implemented
};

#endif /* _DetermineStitchingCoordinatesGeneric_H_ */
