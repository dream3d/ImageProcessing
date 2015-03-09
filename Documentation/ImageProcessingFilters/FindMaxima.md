Find Maxima {#findmaxima}
=====

## Group (Subgroup) ##
ImageProcessing (ImageProcessing)


## Description ##
Finds local peaks:
-all voxels/groups of same valued voxels surrounded by voxels of lower value are peak candidates
-for each peak candidate the surrounding region is flood filled through any voxels with a value > (peak value - noise tolerance)
-if multiple peaks are in a flooded region, only the brightest peak is kept (in the case of 2 or more equal valued peaks, merging occurs)
-the average x, y, and z position of each peak region is the peak voxel

## Parameters ##
| Name             | Type |
|------------------|------|
| Array to Process | String |
| Created Array Name | String |
| Minimum Intensity| Double |

## Required Arrays ##

| Type | Default Array Name |
|------|--------------------|
| Any Scalar | ImageData |


## Created Arrays ##

| Type | Default Array Name | Description | Comment |
|------|--------------------|-------------|---------|
| bool | Maxima | local maxima       | |



## Authors ##

**Copyright:** 2015 BlueQuartz Software, LLC

**Contact Info:** dream3d@bluequartz.net

**Version:** 1.0.0

**License:**  See the License.txt file that came with DREAM3D.



See a bug? Does this documentation need updated with a citation? Send comments, corrections and additions to [The DREAM3D development team](mailto:dream3d@bluequartz.net?subject=Documentation%20Correction)

