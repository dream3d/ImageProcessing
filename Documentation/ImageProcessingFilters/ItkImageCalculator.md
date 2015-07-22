Image Calculator {#itkimagecalculator}
=====

## Group (Subgroup) ##
ImageProcessing (ImageProcessing)


## Description ##
Performs the selected operation with two arrays to make a third. If an operation goes out of bounds it will be truncated to the appropriate min or max value (eg. for an 8 bit image 200+128=255).

## Parameters ##
| Name             | Type |
|------------------|------|
| Selected Array 1 | String |
| Selected Array 2 | String |
| Operator | String |

## Required Arrays ##

| Type | Default Array Name | 
|------|--------------------|
| UInt8  | ImageData     |
| UInt8  | ImageData     |


## Created Arrays ##
| Type | Default Array Name | 
|------|--------------------|
| UInt8  | ImageData     |




## License & Copyright ##

Please see the description file distributed with this plugin.

## DREAM3D Mailing Lists ##

If you need more help with a filter, please consider asking your question on the DREAM3D Users mailing list:
https://groups.google.com/forum/?hl=en#!forum/dream3d-users




