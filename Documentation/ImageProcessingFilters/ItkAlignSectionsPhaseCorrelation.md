Align Sections Phase Correlation {#itkalignsectionsphasecorrelation}
=====

## Group (Subgroup) ##
Unsupported (Misc)

## Description ##
Aligns sections using phase correlation:
   1. Adjacent slices are fourier transformed
   2. Fourier transform of the reference slice is multiplied by the complex conjugate of the moving slice
   3. The result is normalized and inverse fourier transformed
   4. The peak intensity in the resulting image corresponds to the best shift


## Parameters ##
| Name             | Type |
|------------------|------|
| Input Array | String |


## Required Arrays ##

| Type | Default Array Name |
|------|--------------------|
| UInt8  | ImageData     |


## Created Arrays ##

None


## Authors: ##




## License & Copyright ##

Please see the description file distributed with this plugin.

## DREAM3D Mailing Lists ##

If you need more help with a filter, please consider asking your question on the DREAM3D Users mailing list:
https://groups.google.com/forum/?hl=en#!forum/dream3d-users

