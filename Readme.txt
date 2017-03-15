TODO:

- vld and Brofiler are incompatible and cannot be run both on the same time.
  ->Maybe find alternative solutions
- Support for left handiness
- Support for [0,1] range for the z ccordinate in clip space 
- Alpha Blending disturbes texture targets with less than 4 components (so no alpha channel is addressable)
  -> create a 'smart' solution


unimportant:
- util::relativePathToBuildDirectory is very slow, make it faster

Nice to have:
- Check, if window width and height is in ranch of the current screen resolution for windowed mode
- Check if fullscreen resolution is supported and adapt it if necessary 