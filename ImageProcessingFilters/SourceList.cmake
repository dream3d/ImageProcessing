#--////////////////////////////////////////////////////////////////////////////
#-- Your License or copyright can go here
#--////////////////////////////////////////////////////////////////////////////

set(_filterGroupName ImageProcessingFilters)
set(${_filterGroupName}_FILTERS_HDRS "")

#--------
# This macro must come first before we start adding any filters
SIMPL_START_FILTER_GROUP(
  ALL_FILTERS_HEADERFILE ${AllFiltersHeaderFile}
  REGISTER_KNOWN_FILTERS_FILE ${RegisterKnownFiltersFile}
  FILTER_GROUP "${_filterGroupName}"
  BINARY_DIR ${${PLUGIN_NAME}_BINARY_DIR}
  )


#---------
# List your public filters here
set(_PublicFilters
  #AlignSectionsPhaseCorrelation
  ItkAutoThreshold
  ItkBinaryWatershedLabeled
  ItkConvertArrayTo8BitImage
  ItkConvertArrayTo8BitImageAttributeMatrix
  ItkDetermineStitchingCoordinatesGeneric
  ItkDiscreteGaussianBlur
  ItkFindMaxima
  ItkGaussianBlur
  ItkGrayToRGB
  ItkHoughCircles
  ItkImageCalculator
  ItkImageMath
  #IPItkImportImageStack
  #ItkImageRegistration
  ItkKdTreeKMeans
  ItkKMeans
  ItkManualThreshold
  ItkMeanKernel
  ItkMedianKernel
  ItkMultiOtsuThreshold
  #ItkRegionGrowing
  ItkRGBToGray
  #ItkReadImage
  ItkSobelEdge
  ItkStitchImages
  ItkWatershed
  ItkWriteImage
)



list(LENGTH _PublicFilters PluginNumFilters)
set_property(GLOBAL PROPERTY PluginNumFilters ${PluginNumFilters})

#--------------
# Loop on all the filters adding each one. In this loop we default to making each filter exposed in the user
# interface in DREAM3D. If you want to have the filter compiled but NOT exposed to the user then use the next loop
foreach(f ${_PublicFilters} )
  ADD_SIMPL_FILTER(  "ImageProcessing" "ImageProcessing"
                        ${_filterGroupName} ${f}
                        ${ImageProcessing_SOURCE_DIR}/Documentation/${_filterGroupName}/${f}.md TRUE ${ImageProcessing_BINARY_DIR})
endforeach()


#---------------
# This is the list of Private Filters. These filters are available from other filters but the user will not
# be able to use them from the DREAM3D user interface.
set(_PrivateFilters
  ItkManualThresholdTemplate
)

#-----------------
# Loop on the Private Filters adding each one to the DREAM3DLib project so that it gets compiled.
foreach(f ${_PrivateFilters} )
  ADD_SIMPL_FILTER(  "ImageProcessing" "ImageProcessing"
                        ${_filterGroupName} ${f}
                        ${${PLUGIN_NAME}_SOURCE_DIR}/Documentation/${_filterGroupName}/${f}.md FALSE ${${PLUGIN_NAME}_BINARY_DIR})
endforeach()

ADD_SIMPL_SUPPORT_HEADER(${ImageProcessing_SOURCE_DIR} ${_filterGroupName} ItkBridge.h)
ADD_SIMPL_SUPPORT_HEADER(${ImageProcessing_SOURCE_DIR} ${_filterGroupName} ItkReadImageImpl.hpp)
ADD_SIMPL_SUPPORT_HEADER(${ImageProcessing_SOURCE_DIR} ${_filterGroupName} ItkTemplateUtilities.h)


#-------------
# These are files that need to be compiled into the plugin but are NOT filters
ADD_SIMPL_SUPPORT_CLASS(${ImageProcessing_SOURCE_DIR} ${_filterGroupName} util/DetermineStitching)

#---------------------
# This macro must come last after we are done adding all the filters and support files.
SIMPL_END_FILTER_GROUP(${ImageProcessing_BINARY_DIR} "${_filterGroupName}" "ImageProcessing")

