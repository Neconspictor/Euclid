#include <texture/opengl/TextureManagerGL.hpp>
#include <util/Globals.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

//use stb_image -- TODO: replace SOIL completely with this library
#define STB_IMAGE_IMPLEMENTATION
#include <texture/stb/stb_image.h>

#include <renderer/opengl/RendererOpenGL.hpp>

using namespace std;
using namespace platform;

unique_ptr<TextureManagerGL> TextureManagerGL::instance = make_unique<TextureManagerGL>(TextureManagerGL());

GLuint TextureManagerGL::rgba_float_resolutions[] = { GL_RGBA8, GL_RGBA16F, GL_RGBA32F};
GLuint TextureManagerGL:: rgb_float_resolutions[] = { GL_RGB8, GL_RGB16F, GL_RGB32F };
GLuint TextureManagerGL:: rg_float_resolutions[] = { GL_RG8, GL_RG16F, GL_RG32F };



GLint TextureManagerGL::mapFilter(TextureFilter filter, bool useMipMaps)
{
	switch(filter)
	{
	case NearestNeighbor:
		return GL_NEAREST;
	case Linear:
		return GL_LINEAR;
	case Bilinear:
		return GL_LINEAR;
	case Near_Mipmap_Near:
		if (!useMipMaps) return GL_NEAREST;
		return GL_NEAREST_MIPMAP_NEAREST;
	case Near_Mipmap_Linear:
		if (!useMipMaps) return GL_NEAREST;
		return GL_NEAREST_MIPMAP_LINEAR;
	case Linear_Mipmap_Near:
		if (!useMipMaps) return GL_LINEAR;
		return GL_LINEAR_MIPMAP_NEAREST;
	case Linear_Mipmap_Linear:
		if (!useMipMaps) return GL_LINEAR;
		 return GL_LINEAR_MIPMAP_LINEAR;
	default:
		throw runtime_error("TextureManagerGL::mapFilter(TextureFilter): Unknown filter enum: " + to_string(filter));
	}
}

GLint TextureManagerGL::mapUVTechnique(TextureUVTechnique technique)
{
	switch (technique)
	{
	case ClampToEdge:
		return GL_CLAMP_TO_EDGE;
	case Repeat:
		return GL_REPEAT;
	default:
		throw runtime_error("TextureManagerGL::mapUVTechnique(TextureUVTechnique): Unknown uv technique enum: " + to_string(technique));
	}
}

GLuint TextureManagerGL::getFormat(ColorSpace colorspace)
{
	switch (colorspace) {
	case RGBA:
		return GL_RGBA;
	case RGB:
		return GL_RGB;
	case RG:
		return GL_RG;
	default: {
		throw runtime_error("TextureManagerGL::getFormat(Colorspace): Unknown colorspace: " + colorspace);
	}
	}
}

GLuint TextureManagerGL::getFormat(int numberComponents)
{
	switch (numberComponents) {
		case 4: return GL_RGBA;
		case 3: return GL_RGB;
		case 2: return GL_RG;
		default: {
			throw runtime_error("TextureManagerGL::getFormat(int): Not supported number of components " + numberComponents);
		}
	}
}

GLuint TextureManagerGL::getInternalFormat(GLuint format, bool useSRGB, bool isFloatData, Resolution resolution)
{
	if (!isFloatData) {

		if (!useSRGB) {
			return format;
		}

		if (resolution != BITS_8) {
			throw runtime_error("TextureManagerGL::getInternalFormat(): SRGB only supported for BITS_8, not for " + resolution);
		}


		switch (format) {
			case GL_RGBA:
				return GL_SRGB_ALPHA;
			case GL_RGB:
				return GL_SRGB;
			default: {
				throw runtime_error("TextureManagerGL::getInternalFormat(): Not supported format for SRGB: " + format);
			}
		}
	}

	switch (format) {
	case GL_RGBA:
		return rgba_float_resolutions[resolution];
	case GL_RGB:
		return rgb_float_resolutions[resolution];
	case GL_RG:
		return rg_float_resolutions[resolution];
	default: {
		throw runtime_error("TextureManagerGL::getInternalFormat(): Unknown format: " + format);
	}
	}
}

GLuint TextureManagerGL::getType(bool isFloatData)
{
	if (isFloatData) {
		return GL_FLOAT;
	}
	return GL_UNSIGNED_BYTE;
}

TextureManagerGL::TextureManagerGL() : TextureManager(), logClient(getLogServer())
{
	textureLookupTable = map<string, TextureGL*>();
	logClient.setPrefix("[TextureManagerGL]");
}

void TextureManagerGL::releaseTexture(Texture * tex)
{
	TextureGL* gl = dynamic_cast<TextureGL*>(tex);
	if (!gl) return;

	for (auto it = textures.begin(); it != textures.end(); ++it) {
		if (&(*it) == tex) {
			GLuint id = it->getTexture();
			glDeleteTextures(1, &id);
			textures.erase(it);
			break; // we're done
		}
	}
}

TextureManagerGL::~TextureManagerGL()
{
	for (auto& texture : textures)
	{
		GLuint id = texture.getTexture();
		glDeleteTextures(1, &id);
	}

	for (auto& map : cubeMaps)
	{
		GLuint id = map.getCubeMap();
		glDeleteTextures(1, &id);
	}
}

CubeMapGL * TextureManagerGL::addCubeMap(CubeMapGL cubemap)
{
	cubeMaps.push_back(move(cubemap));
	return &cubeMaps.back();
}

CubeMap* TextureManagerGL::createCubeMap(const string& right, const string& left, const string& top,
	const string& bottom, const string& back, const string& front, bool useSRGBOnCreation)
{
	string rightCStr = ::util::globals::TEXTURE_PATH + right;
	string leftCStr = ::util::globals::TEXTURE_PATH + left;
	string topCStr = ::util::globals::TEXTURE_PATH + top;
	string bottomCStr = ::util::globals::TEXTURE_PATH + bottom;
	string backCStr = ::util::globals::TEXTURE_PATH + back;
	string frontCStr = ::util::globals::TEXTURE_PATH + front;

	/*
	  TODO: implement is with stb_image! 

	GLuint cubeMap = SOIL_load_OGL_cubemap(rightCStr.c_str(), leftCStr.c_str(), topCStr.c_str(),
		bottomCStr.c_str(), backCStr.c_str(), frontCStr.c_str(),
		SOIL_LOAD_AUTO, 0, 0);
    
	if (cubeMap == GL_FALSE)
	{
		LOG(logClient, Fault) << "Couldn't load cubeMap!" << endl <<
			"	right: " << rightCStr << endl <<
			"	left: " << leftCStr << endl <<
			"	top: " << topCStr << endl <<
			"	bottom: " << bottomCStr << endl <<
			"	back: " << backCStr << endl <<
			"	front: " << frontCStr;

		stringstream ss;
		ss << "TextureManagerGL::createCubeMap(): CubeMap couldn't successfully be loaded!";
		throw runtime_error(ss.str());
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	cubeMaps.push_back(CubeMapGL(cubeMap));

	return &cubeMaps.back();*/
	return nullptr;
}

CubeMap * TextureManagerGL::createCubeMap(int sideWidth, int sideHeight, TextureData data)
{
	unsigned int cubeMap;
	glGenTextures(1, &cubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	GLuint format = getFormat(data.colorspace);
	GLuint internalFormat = getInternalFormat(format, data.useSRGB, data.isFloatData, data.resolution);

	GLuint type = getType(data.isFloatData);


	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat, sideWidth, sideHeight, 0, format, type, nullptr);
	}

	GLint minFilter = mapFilter(data.minFilter, data.generateMipMaps);
	GLint magFilter = mapFilter(data.magFilter, data.generateMipMaps);
	GLint uvTechnique = mapUVTechnique(data.uvTechnique);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, uvTechnique);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, uvTechnique);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, uvTechnique);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, magFilter);

	if (data.generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);


	cubeMaps.push_back(CubeMapGL(cubeMap));
	return &cubeMaps.back();
}

TextureGL* TextureManagerGL::createTextureGL(string localPathFileName, GLuint textureID)
{
	textures.push_back(TextureGL(textureID));
	TextureGL* pointer = &textures.back();
	textureLookupTable.insert(pair<string, TextureGL*>(localPathFileName, pointer));
	return pointer;
}

TextureGL* TextureManagerGL::getImageGL(const string& file)
{
	return static_cast<TextureGL*>(getImage(file));
}

Texture * TextureManagerGL::getDefaultBlackTexture()
{
	return getImageGL("_intern/black.png");
}

Texture * TextureManagerGL::getDefaultNormalTexture()
{
	//normal maps shouldn't use mipmaps (important for shading!)
	return getImage("_intern/default_normal.png", { false, true, Linear, Linear, Repeat, RGB, BITS_8 });
}

Texture * TextureManagerGL::getDefaultWhiteTexture()
{
	return getImage("_intern/white.png", { true, false, Linear, Linear, Repeat, RGB, BITS_8 });
}



Texture* TextureManagerGL::getHDRImage2(const string& file, TextureData data)
{
	auto it = textureLookupTable.find(file);

	// Don't create duplicate textures!
	if (it != textureLookupTable.end())
	{
		return it->second;
	}

	string path = ::util::globals::TEXTURE_PATH + file;


	stbi_set_flip_vertically_on_load(true); // opengl uses texture coordinates with origin at bottom left 
	int width, height, nrComponents;
	float *rawData = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
	unsigned int hdrTexture;
	if (!rawData) {
		LOG(logClient, Fault) << "Couldn't load image file: " << file << endl;
		stringstream ss;
		ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
		throw runtime_error(ss.str());
	}

	GLuint format = getFormat(nrComponents);
	GLuint internalFormat = getInternalFormat(format, data.useSRGB, data.isFloatData, data.resolution);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &hdrTexture);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, rawData);

	GLint minFilter = mapFilter(data.minFilter, data.generateMipMaps);
	GLint magFilter = mapFilter(data.magFilter, data.generateMipMaps);
	GLint uvTechnique = mapUVTechnique(data.uvTechnique);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

	if (data.generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(rawData);

	return createTextureGL(file, hdrTexture);
}




Texture* TextureManagerGL::getHDRImage(const string& file, TextureData data)
{
	return getHDRImage2(file, data);
}

Texture* TextureManagerGL::getImage(const string& file, TextureData data)
{


	if (data.isFloatData) {
		return getHDRImage2(file, data);
	}

	auto it = textureLookupTable.find(file);

	// Don't create duplicate textures!
	if (it != textureLookupTable.end())
	{
		return it->second;
	}

	string path = ::util::globals::TEXTURE_PATH + file;



	stbi_set_flip_vertically_on_load(true); // opengl uses texture coordinates with origin at bottom left 
	int width, height, nrComponents;
	unsigned char* rawData = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	unsigned int textureID;
	if (!rawData) {
		LOG(logClient, Fault) << "Couldn't load image file: " << file << endl;
		stringstream ss;
		ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
		throw runtime_error(ss.str());
	}


	GLuint format = getFormat(nrComponents);

	GLuint internalFormat = getInternalFormat(format, data.useSRGB, data.isFloatData, data.resolution);



	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (data.isFloatData) {
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, rawData);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, rawData);
	}

	GLint minFilter = mapFilter(data.minFilter, data.generateMipMaps);
	GLint magFilter = mapFilter(data.magFilter, data.generateMipMaps);
	GLint uvTechnique = mapUVTechnique(data.uvTechnique);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	if (data.generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(rawData);

	float aniso = 0.0f;
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	LOG(logClient, Debug) << "texture to load: " << path;

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);

	return createTextureGL(file, textureID);
}

string TextureManagerGL::getImagePath()
{
	return ::util::globals::TEXTURE_PATH;
}

string TextureManagerGL::getFullFilePath(const string& localFilePath)
{
	return ::util::globals::TEXTURE_PATH + localFilePath;
}

void TextureManagerGL::loadImages(const string& imageFolder)
{
	//TODO!
}

TextureManagerGL* TextureManagerGL::get()
{
	return instance.get();
}