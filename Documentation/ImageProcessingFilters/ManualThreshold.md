Manual Threshold Image {#manualthreshold}
=====

## Group (Subgroup) ##
ImageProcessing (ImageProcessing)


## Description ##
Thresholds an 8 bit array to 0 and 255 using the selected method. Values below the selected
value will be set to 0 (black) and above will be set to 255 (white). Manual Parameter is
 threshold value for manual selection and power for robust automatic selection.

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


## Authors ##

**Copyright:** 2015 BlueQuartz Software, LLC

**Contact Info:** dream3d@bluequartz.net

**Version:** 1.0.0

**License:**  See the License.txt file that came with DREAM3D.

See a bug? Does this documentation need updated with a citation? Send comments, corrections and additions to [The DREAM3D development team](mailto:dream3d@bluequartz.net?subject=Documentation%20Correction)



