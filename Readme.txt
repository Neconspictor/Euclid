TODO:

IMPORTANT: 

- put the following to a more suitable place in RendererOpenGL::cullFaces:
		glEnable(GL_POLYGON_OFFSET_FILL); 
		glPolygonOffset(-3.5f, 1.0f);
this is needed for rendering shadow maps and sholdn't be there in general!

- shadow edge flickering
- culling front faces for rendering the shadow maps produces errors -> check if shadow maps
are correctly implemented!
- Read HDR Tutorial (is needed for PBR)

- ARB_SHADING_LANGUAGE_INCLUDE extension is only supported by NVIDIA GPUs. 
We need a modul for parsing #include directives!

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