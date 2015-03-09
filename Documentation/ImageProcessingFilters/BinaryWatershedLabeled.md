Binary Watershed Labeled {#binarywatershedlabeled}
=====

## Group (Subgroup) ##
ImageProcessing (ImageProcessing)


## Description ##
Performs a binary watershed operation to split concave objects in a binary image. The watershed using a distance map instead of a grayscale gradient. Watershed regions are seeded using ultimate points to avoid over splitting the image. Ultimate points are found as maxima on the distance map using the algorithm of "Find Maxima". As a result a higher noise tolerance will reject more maxima on the distance map and therefore split concave objects more conservatively (while a lower value will split more aggressively). This filter is nearly identical to the *Binary Watershed* filter except that the output images is a labeled output image and watershed lines are not given the background color, but rather assigned to one of the features. 

## Parameters ##
| Name             | Type |
|------------------|------|
| Array to Process | String |
| Peak Noise Tolerance | float |
| Created Array Name | String |


## Required Arrays ##

| Type | Default Array Name | Description | Comment |
|------|--------------------|-------------|---------|
| boolean | ThresholdArray | boolean array to be watershed       | |


## Created Arrays ##

| Type | Default Array Name | Description | Comment |
|------|--------------------|-------------|---------|
| int32 | WatershedArray | watershedded array | |


## Authors ##

**Copyright:** 2015 BlueQuartz Software, LLC

**Contact Info:** dream3d@bluequartz.net

**Version:** 1.0.0

**License:**  See the License.txt file that came with DREAM3D.




See a bug? Does this documentation need updated with a citation? Send comments, corrections and additions to [The DREAM3D development team](mailto:dream3d@bluequartz.net?subject=Documentation%20Correction)
