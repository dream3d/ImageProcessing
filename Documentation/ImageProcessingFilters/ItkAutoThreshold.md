Threshold Image (Auto) (Image Processing) {#itkautothreshold}
=====

## Group (Subgroup) ##
ImageProcessing (ImageProcessing)


## Description ##
Thresholds an 8 bit array to 0 and 255 using the selected method. Values below the selected value will be set
to 0 (black) and above will be set to 255 (white). Manual Parameter is threshold value for manual selection
and power for robust automatic selection.

## Parameters ##
| Name             | Type |
|------------------|------|
| Array to Process | String |
| Overwrite Array| Bool |
| Created Array Name | String |
| Threshold Method | String |
| Slice at a Time | Bool|
| Manual Parameter | Int |

## Required Arrays ##

| Type | Default Array Name | Description | Comment |
|------|--------------------|-------------|---------|
| uint8_t | ImageData | 8 bit image data       | |


## Created Arrays ##

| Type | Default Array Name | Description | Comment |
|------|--------------------|-------------|---------|
| uint8_t | ProcessedArray | 8 bit image data       | |



## License & Copyright ##

Please see the description file distributed with this plugin.

## DREAM3D Mailing Lists ##

If you need more help with a filter, please consider asking your question on the DREAM3D Users mailing list:
https://groups.google.com/forum/?hl=en#!forum/dream3d-users

