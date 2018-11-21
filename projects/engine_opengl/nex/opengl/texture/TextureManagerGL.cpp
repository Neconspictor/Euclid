#include <nex/opengl/texture/TextureManagerGL.hpp>
#include <nex/util/Globals.hpp>

//use stb_image -- TODO: replace SOIL completely with this library
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <imgui/imgui.h>

using namespace std;
using namespace nex;


TextureManagerGL TextureManagerGL::instance;

TextureManagerGL::TextureManagerGL() : m_logger("TextureManagerGL"), m_anisotropy(0.0f)
{
	textureLookupTable = map<string, TextureGL*>();

	//TextureManagerGL::setAnisotropicFiltering(m_anisotropy);
}

void TextureManagerGL::releaseTexture(TextureGL * tex)
{
	for (auto&& it = textures.begin(); it != textures.end(); ++it) {
		if (&(*it) == tex) {
			GLuint id = it->getTexture();
			GLCall(glDeleteTextures(1, &id));
			textures.erase(it);
			break; // we're done
		}
	}
}

void TextureManagerGL::setAnisotropicFiltering(float value)
{
	m_anisotropy = value;

	GLfloat maxAnisotropy = getMaxAnisotropicFiltering();

	m_anisotropy = std::min(m_anisotropy, maxAnisotropy);

	GLCall(glSamplerParameterf(mDefaultImageSampler.getID(), GL_TEXTURE_MAX_ANISOTROPY, m_anisotropy));
}

float TextureManagerGL::getAnisotropicFiltering() const
{
	return m_anisotropy;
}

float TextureManagerGL::getMaxAnisotropicFiltering() const
{
	GLfloat maxAnisotropy;
	GLCall(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy));
	return maxAnisotropy;
}

TextureManagerGL::~TextureManagerGL()
{
}

void TextureManagerGL::init()
{
	GLuint sampler;
	glGenSamplers(1, &sampler);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, 2.0f);

	mDefaultImageSampler.setID(sampler);

	setAnisotropicFiltering(16.0f);
}

CubeMapGL * TextureManagerGL::addCubeMap(CubeMapGL cubemap)
{
	cubeMaps.emplace_back(move(cubemap));
	return &cubeMaps.back();
}

CubeMapGL* TextureManagerGL::createCubeMap(const string& right, const string& left, const string& top,
	const string& bottom, const string& back, const string& front, bool useSRGBOnCreation)
{
	string texturePath = ::util::Globals::getTexturePath();

	string rightCStr = texturePath + right;
	string leftCStr = texturePath + left;
	string topCStr = texturePath + top;
	string bottomCStr = texturePath + bottom;
	string backCStr = texturePath + back;
	string frontCStr = texturePath + front;

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

TextureGL * TextureManagerGL::getDefaultBlackTexture()
{
	return getImage("_intern/black.png", { false, true, Linear_Mipmap_Linear, Linear, Repeat, RGB, BITS_8 });
}

TextureGL * TextureManagerGL::getDefaultNormalTexture()
{
	//normal maps shouldn't use mipmaps (important for shading!)
	return getImage("_intern/default_normal.png", { false, true, Linear_Mipmap_Linear, Linear, Repeat, RGB, BITS_8 });
}

TextureGL * TextureManagerGL::getDefaultWhiteTexture()
{
	return getImage("_intern/white.png", { false, true, Linear_Mipmap_Linear, Linear, Repeat, RGB, BITS_8 });
}



TextureGL* TextureManagerGL::getHDRImage2(const string& file, TextureData data)
{
	auto it = textureLookupTable.find(file);

	// Don't create duplicate textures!
	if (it != textureLookupTable.end())
	{
		return it->second;
	}

	string path = ::util::Globals::getTexturePath() + file;


	stbi_set_flip_vertically_on_load(true); // opengl uses texture coordinates with origin at bottom left 
	int width, height, nrComponents;
	float *rawData = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
	unsigned int hdrTexture;
	if (!rawData) {
		LOG(m_logger, Fault) << "Couldn't load image file: " << file << endl;
		stringstream ss;
		ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
		throw_with_trace(runtime_error(ss.str()));
	}

	GLuint format = TextureGL::getFormat(nrComponents);
	GLuint internalFormat = TextureGL::getInternalFormat(format, data.useSRGB, data.isFloatData, data.resolution);

	GLCall(glActiveTexture(GL_TEXTURE0));
	glGenTextures(1, &hdrTexture);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, rawData);

	GLint minFilter = TextureGL::mapFilter(data.minFilter);
	GLint magFilter = TextureGL::mapFilter(data.magFilter);
	GLint uvTechnique = TextureGL::mapUVTechnique(data.uvTechnique);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_anisotropy);

	if (data.generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	stbi_image_free(rawData);

	return createTextureGL(file, hdrTexture);
}




TextureGL* TextureManagerGL::getHDRImage(const string& file, TextureData data)
{
	return getHDRImage2(file, data);
}

TextureGL* TextureManagerGL::getImage(const string& file, TextureData data)
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

	string path = ::util::Globals::getTexturePath() + file;



	stbi_set_flip_vertically_on_load(true); // opengl uses texture coordinates with origin at bottom left 
	int width, height, nrComponents;
	unsigned char* rawData = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	unsigned int textureID;
	if (!rawData) {
		LOG(m_logger, Fault) << "Couldn't load image file: " << file << endl;
		stringstream ss;
		ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
		throw_with_trace(runtime_error(ss.str()));
	}


	GLuint format = TextureGL::getFormat(nrComponents);

	GLuint internalFormat = TextureGL::getInternalFormat(format, data.useSRGB, data.isFloatData, data.resolution);



	GLCall(glActiveTexture(GL_TEXTURE0));
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	if (data.isFloatData) {
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, rawData);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, rawData);
	}

	GLint minFilter = TextureGL::mapFilter(data.minFilter);
	GLint magFilter = TextureGL::mapFilter(data.magFilter);
	GLint uvTechnique = TextureGL::mapUVTechnique(data.uvTechnique);

	if (data.generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 16.0f);

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	stbi_image_free(rawData);

	float aniso = 0.0f;
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	LOG(m_logger, Debug) << "texture to load: " << path;

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);

	return createTextureGL(file, textureID);
}

string TextureManagerGL::getImagePath()
{
	return ::util::Globals::getTexturePath();
}

string TextureManagerGL::getFullFilePath(const string& localFilePath)
{
	return ::util::Globals::getTexturePath() + localFilePath;
}

void TextureManagerGL::loadImages(const string& imageFolder)
{
	//TODO!
}

const SamplerGL* TextureManagerGL::getDefaultImageSampler() const
{
	return &mDefaultImageSampler;
}

TextureManagerGL* TextureManagerGL::get()
{
	return &instance;
}

void TextureManagerGL::release()
{
	for (auto& texture : textures)
	{
		texture.release();
	}

	textures.clear();

	for (auto& map : cubeMaps)
	{
		map.release();
	}

	cubeMaps.clear();
}


TextureManager_Configuration::TextureManager_Configuration(TextureManagerGL* textureManager) : m_textureManager(textureManager)
{
}

void TextureManager_Configuration::drawSelf()
{

	float anisotropyMax = m_textureManager->getMaxAnisotropicFiltering();
	float anisotropy = m_textureManager->getAnisotropicFiltering();

	//float anisotropyBackup = anisotropy;

	// render configuration properties
	ImGui::PushID(m_id.c_str());
	if (ImGui::InputFloat("Anisotropic Filtering (read-only)", &anisotropy, 1.0f, 1.0f, "%.3f")) //ImGuiInputTextFlags_ReadOnly
	{
		m_textureManager->setAnisotropicFiltering(anisotropy);
	}
	ImGui::PopID();

	//throw_with_trace(std::runtime_error("Hello exception!"));
}