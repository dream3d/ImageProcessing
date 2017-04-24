Convert RGB to Grayscale (ImageProcessing) {#itkrgbtogray}
=====

## Group (Subgroup) ##

ImageProcessing (ImageProcessing)


## Description ##

Converts arrays that represent color images (RGB or RGBA) to grayscale with the specified weightings. The filter uses a Colorimetric (luminance-preserving) algorithm [https://en.wikipedia.org/wiki/Grayscale](https://en.wikipedia.org/wiki/Grayscale) which requires the user to enter the luminance values for each channel in the image (alpha channel is ignored). The defaults that appear are the generally accepted values. If the user wishes to change those values they can be changed. The filter will allow the user to select from 1 to N number of arrays to convert.

## Parameters ##

| Name             | Type |
|------------------|------|
| Arrays to Process | QVector of DataArrayPaths |
| Color Weights | 3*float |
| Output Array Prefix | String |
| Output AttributeMatrix | String |

## Required Arrays ##

| Type | Default Array Name | Description | Comment |
|------|--------------------|-------------|---------|
| any | ImageData | any 3 or 4 component image data       | |


## Created Arrays ##

| Type | Default Array Name | Description | Comment |
|------|--------------------|-------------|---------|
| UInt8 | GrayScale | 1 component image data |  |


## License & Copyright ##

Please see the description file distributed with this plugin.

## DREAM3D Mailing Lists ##

If you need more help with a filter, please consider asking your question on the DREAM3D Users mailing list:
https://groups.google.com/forum/?hl=en#!forum/dream3d-users




