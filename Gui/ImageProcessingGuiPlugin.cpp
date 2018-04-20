

#include "ImageProcessingGuiPlugin.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ImageProcessingGuiPlugin::ImageProcessingGuiPlugin()
: ImageProcessingPlugin()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
ImageProcessingGuiPlugin::~ImageProcessingGuiPlugin() = default;

#include "ImageProcessing/Gui/FilterParameterWidgets/RegisterKnownFilterParameterWidgets.cpp"
