#pragma once
#include <glad/glad.h>
#include <nex/opengl/texture/TextureGL.hpp>


/*
*
* TEXTURE_BORDER_COLOR      | n x C     | GetSamplerParameter  | 0,0,0,0                | Border color                 | 3.9    | -          |
| TEXTURE_MIN_FILTER        | n x Z6    | GetSamplerParameter  | NEAREST_MIPMAP_LINEAR  | Minification function        | 3.9.9  | -          |
| TEXTURE_MAG_FILTER        | n x Z2    | GetSamplerParameter  | LINEAR                 | Magnification function       | 3.9.10 | -          |
| TEXTURE_WRAP_S            | n x Z5    | GetSamplerParameter  | REPEAT                 | Texcoord s wrap mode         | 3.9.9  | -          |
| TEXTURE_WRAP_T            | n x Z5    | GetSamplerParameter  | REPEAT                 | Texcoord t wrap mode         | 3.9.9  | -          |
| TEXTURE_WRAP_R            | n x Z5    | GetSamplerParameter  | REPEAT                 | Texcoord r wrap mode         | 3.9.9  | -          |
| TEXTURE_MIN_LOD           | n x R     | GetSamplerParameter  | -1000                  | Minimum level of detail      | 3.9    | -          |
| TEXTURE_MAX_LOD           | n x R     | GetSamplerParameter  | 1000                   | Maximum level of detail      | 3.9    | -          |
| TEXTURE_LOD_BIAS          | n x R     | GetSamplerParameter  | 0.0                    | Texture level of detail      | 3.9.9  | -          |
|                           |           |                      |                        | bias (biastexobj)            |        |            |
| TEXTURE_COMPARE_MODE      | n x Z2    | GetSamplerParameter  | NONE                   | Comparison mode              | 3.9.16 | -          |
| TEXTURE_COMPARE_FUNC      | n x Z8    | GetSamplerParameter  | LEQUAL                 | Comparison function          | 3.9.16 | -          |
| TEXTURE_MAX_ANISOTROPY_EXT| n x R     | GetSamplerParameter  | 1.0                    | Maximum degree of anisotropy | 3.9    | -
*
*/

struct SamplerState
{
	glm::vec4 borderColor = { 0,0,0,0 };
	TextureFilter minFilter = Near_Mipmap_Linear;
	TextureFilter magFilter = Linear;
	TextureUVTechnique wrapS = Repeat;
	TextureUVTechnique wrapT = Repeat;
	TextureUVTechnique wrapR = Repeat;
	int minLOD = -1000;
	int maxLOD = 1000;
	float biasLOD = 0.0f;
	//compareMode TODO
	//compareFunc TODO
	float anisotropy = 1.0f;
};



class SamplerGL
{
public:
	explicit SamplerGL();
	SamplerGL(GLuint samplerID);
	SamplerGL(SamplerGL&& o);
	SamplerGL& operator=(SamplerGL&& o);


	SamplerGL(const SamplerGL&) = delete;
	SamplerGL& operator=(const SamplerGL&) = delete;

	virtual ~SamplerGL();

	GLuint getID() const;

	void setID(GLuint sampler);

protected:
	GLuint m_samplerID;
};
