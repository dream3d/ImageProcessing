Determine Stitching Coordinates Generic {#itkdeterminestitchingcoordinatesgeneric}
=====

## Group (Subgroup) ##
ImageProcessing (ImageProcessing)


## Description ##
This filter takes a series of tiled gray-scale images (8bit) and calculates the origin of each tile such that a fully stitched montage would result from placing each tile. The images, and only the images, must all be sitting in one attribute matrix.Currently the only way to import the required data is to use the Zeiss AxioVision Import filter. The meta-data must include the global stage positions of each tile, and index, which is used as a rough starting point. 

For a series of images, the images are re-ordered as though they were captured in a row-by-row comb of the data ![](RowWiseComb.tif)

This re-ordering is only done locally for the filter, any output data will correspond to how the images were originally ordered. 

The first image is given coordinates of (0,0). The second is by placed by taking the overlap window between the first image and the second image and using cross correlation to find the location of maximum overlap of the image data. ![](LeftXC.tif) The overlap window is found from the global stage positions of the tiles in the meta data. All images in the first row are placed by comparing a shared window on the left of the image with a shared window on the right of the previously placed image. 

For images in the first column, the overlap window is found by taking the top of the image to be placed with the bottom of the image that is already placed. ![](TopXC.tif)

For all other images, both a top and left window are taken, and the best position is averaged. ![](TopAndLeftXC.tif)

When running the cross-correlation, a requirement of at least 50% overlap of the two windows is placed on the operation. 

This filter uses the *FFTNormalizedCorrelationImageFilter* from the ITK library. 

The result of this filter is an array containing the global xy origins of each tile (with (0, 0) being the origin of the first tile). In order to actually stitch the images and put into a new data array, the *Stitch Images* filter must be called after this one. 


## Parameters ##
| Name             | Type | Comment |
|------------------|------|--------|
| Use Zeiss Meta Data | Bool | Currently must be clicked on |



## Required Attribute Matrix ##

An attribute matrix that holds all the images (and only the images) of uint8 type is required. Currently, the only way to generate this attribute matrix is to use the Zeiss AxioVision Import filter. 

Additionally, the meta-data attribute matrix is also required. Currently, the only way to generate this is to use the Zeiss AxioVision Import filter. In order to supply this attribute matrix, the "Use Zeiss Meta Data" option must be clicked. 


## Created Arrays ##
| Type | Default Array Name | 
|------|--------------------|
| Float  | Stitched Coordinates     |
| String | Stitched Array Names | 

## Created Attribute Matrix ##
An attribute matrix to hold the above arrays is also created. The default name is "Tile Info AttrMat". 



## License & Copyright ##

Please see the description file distributed with this plugin.

## DREAM3D Mailing Lists ##

If you need more help with a filter, please consider asking your question on the DREAM3D Users mailing list:
https://groups.google.com/forum/?hl=en#!forum/dream3d-users


