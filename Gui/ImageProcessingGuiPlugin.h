#pragma once

#include "ImageProcessing/ImageProcessingPlugin.h"

class ImageProcessingGuiPlugin : public ImageProcessingPlugin
{
  Q_OBJECT
  Q_INTERFACES(ISIMPLibPlugin)
  Q_PLUGIN_METADATA(IID "net.bluequartz.dream3d.ImageProcessingGuiPlugin")

public:
  ImageProcessingGuiPlugin();
  ~ImageProcessingGuiPlugin() override;

public:
  ImageProcessingGuiPlugin(const ImageProcessingGuiPlugin&) = delete;            // Copy Constructor Not Implemented
  ImageProcessingGuiPlugin(ImageProcessingGuiPlugin&&) = delete;                 // Move Constructor
  ImageProcessingGuiPlugin& operator=(const ImageProcessingGuiPlugin&) = delete; // Copy Assignment Not Implemented
  ImageProcessingGuiPlugin& operator=(ImageProcessingGuiPlugin&&) = delete;      // Move Assignment Not Implemented
};
