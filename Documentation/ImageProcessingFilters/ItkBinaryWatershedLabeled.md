Binary Watershed Labeled (ImageProcessing) {#itkbinarywatershedlabeled}
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




## License & Copyright ##

Please see the description file distributed with this plugin.

## DREAM3D Mailing Lists ##

If you need more help with a filter, please consider asking your question on the DREAM3D Users mailing list:
https://groups.google.com/forum/?hl=en#!forum/dream3d-users

