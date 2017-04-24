Watershed Filter (ImageProcessing) {#itkwatershed}
=====

## Group (Subgroup) ##
ImageProcessing (ImageProcessing)


## Description ##
This filter segments grayscale images into grains using ITK's watershed segmentation

## Parameters ##
| Name             | Type |
|------------------|------|
| Array to Process | String |
| Watershed threshold | float |
| Watershed level | float |

## Required Arrays ##

| Type | Default Array Name | Description | Comment |
|------|--------------------|-------------|---------|
| Int  | ImageData | 8 bit image data        | |


## Created Arrays ##

| Type | Default Array Name | Description | Comment |
|------|--------------------|-------------|---------|
| Int  | Grain ID | | |




## License & Copyright ##

Please see the description file distributed with this plugin.

## DREAM3D Mailing Lists ##

If you need more help with a filter, please consider asking your question on the DREAM3D Users mailing list:
https://groups.google.com/forum/?hl=en#!forum/dream3d-users




