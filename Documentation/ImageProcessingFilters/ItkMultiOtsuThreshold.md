Multi Level Otsu Threshold (ImageProcessing) {#itkmultiotsuthreshold}
=====

## Group (Subgroup) ##
ImageProcessing (ImageProcessing)


## Description ##
Splits an image into (levels+1) classes with (levels) threshold levels using the mutiple otsu threshold method

## Parameters ##
| Name             | Type |
|------------------|------|
| Array to Process | String |
| Overwrite Array| Bool |
| Created Array Name | String |
| Slice at a Time | Bool|
| Number of Levels | Int |

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




