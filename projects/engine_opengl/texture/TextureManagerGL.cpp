#include <texture/TextureManagerGL.hpp>
#include <util/Globals.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

//use stb_image -- TODO: replace SOIL completely with this library
#define STB_IMAGE_IMPLEMENTATION
#include <texture/stb/stb_image.h>

#include <renderer/RendererOpenGL.hpp>

using namespace std;
using namespace platform;


TextureManagerGL TextureManagerGL::instance;

TextureManagerGL::TextureManagerGL() : TextureManager(), logClient(getLogServer())
{
	textureLookupTable = map<string, TextureGL*>();
	logClient.setPrefix("[TextureManagerGL]");
}

void TextureManagerGL::releaseTexture(Texture * tex)
{
	TextureGL* gl = dynamic_cast<TextureGL*>(tex);
	if (!gl) return;

	for (auto&& it = textures.begin(); it != textures.end(); ++it) {
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
	cubeMaps.emplace_back(move(cubemap));
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

TextureGL* TextureManagerGL::createTextureGL(string localPathFileName, GLuint textureID)
{
	textures.emplace_back(move(TextureGL(textureID)));
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

	GLuint format = TextureGL::getFormat(nrComponents);
	GLuint internalFormat = TextureGL::getInternalFormat(format, data.useSRGB, data.isFloatData, data.resolution);

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &hdrTexture);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, rawData);

	GLint minFilter = TextureGL::mapFilter(data.minFilter, data.generateMipMaps);
	GLint magFilter = TextureGL::mapFilter(data.magFilter, data.generateMipMaps);
	GLint uvTechnique = TextureGL::mapUVTechnique(data.uvTechnique);

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


	GLuint format = TextureGL::getFormat(nrComponents);

	GLuint internalFormat = TextureGL::getInternalFormat(format, data.useSRGB, data.isFloatData, data.resolution);



	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (data.isFloatData) {
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, rawData);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, rawData);
	}

	GLint minFilter = TextureGL::mapFilter(data.minFilter, data.generateMipMaps);
	GLint magFilter = TextureGL::mapFilter(data.magFilter, data.generateMipMaps);
	GLint uvTechnique = TextureGL::mapUVTechnique(data.uvTechnique);

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
	return &instance;
}