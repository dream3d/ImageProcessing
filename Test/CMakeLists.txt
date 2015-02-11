project(ImageProcessingTest)



ADD_EXECUTABLE(ITKUtilitiesTest ITKUtilitiesTest.cpp)
TARGET_LINK_LIBRARIES(ITKUtilitiesTest Qt5::Core
										DREAM3DLib
										Qt5::Gui Qt5::Widgets
										QtSupport
										DREAM3DWidgetsLib
										${ITK_LIBRARIES}
)
SET_TARGET_PROPERTIES(ITKUtilitiesTest PROPERTIES FOLDER ${PLUGIN_NAME}Plugin/Test)

include_directories(${DREAM3DProj_SOURCE_DIR}/Source)

find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(BinaryWatershedTest  BinaryWatershedTest.cpp)

target_link_libraries(BinaryWatershedTest ${QT_LIBRARIES} H5Support EbsdLib DREAM3DLib QtSupport DREAM3DWidgetsLib ${ITK_LIBRARIES} )

