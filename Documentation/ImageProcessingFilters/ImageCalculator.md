Image Calculator {#imagecalculator}
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


## Authors ##

**Copyright:** 2015 BlueQuartz Software, LLC

**Contact Info:** dream3d@bluequartz.net

**Version:** 1.0.0

**License:**  See the License.txt file that came with DREAM3D.

See a bug? Does this documentation need updated with a citation? Send comments, corrections and additions to [The DREAM3D development team](mailto:dream3d@bluequartz.net?subject=Documentation%20Correction)



