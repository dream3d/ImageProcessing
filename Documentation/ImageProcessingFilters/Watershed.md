Watershed {#watershed}
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


## Authors ##

**Copyright:** 2015 BlueQuartz Software, LLC

**Contact Info:** dream3d@bluequartz.net

**Version:** 1.0.0

**License:**  See the License.txt file that came with DREAM3D.

See a bug? Does this documentation need updated with a citation? Send comments, corrections and additions to [The DREAM3D development team](mailto:dream3d@bluequartz.net?subject=Documentation%20Correction)



