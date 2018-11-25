#include <nex/opengl/texture/TextureManagerGL.hpp>

//use stb_image -- TODO: replace SOIL completely with this library
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <stb/stb_image_write.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

//#define STB_DEFINE                                                     
//#include <stb/stb.h>


#include <nex/opengl/renderer/RendererOpenGL.hpp>
#include <nex/util/ExceptionHandling.hpp>
#include <imgui/imgui.h>

using namespace std;
using namespace nex;


TextureManagerGL TextureManagerGL::instance;

TextureManagerGL::TextureManagerGL() : m_logger("TextureManagerGL"), mDefaultImageSampler(nullptr), mFileSystem(nullptr)
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

void TextureManagerGL::writeHDR(const GenericImageGL& imageData, const char* filePath)
{
	stbi__flip_vertically_on_write = true;
	stbi_write_hdr(filePath, imageData.width, imageData.height, imageData.components, (float*)imageData.pixels);
}

void TextureManagerGL::readImage(GenericImageGL* imageData, const char* filePath)
{
	FILE* file = nullptr;
	errno_t err;
	if ((err = fopen_s(&file, filePath, "rb")) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));

	std::fread(imageData, sizeof(GenericImageGL), 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));


	imageData->pixels = new char[imageData->bufSize];

	std::fread(imageData->pixels, imageData->bufSize, 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));

	fclose(file);
}

void TextureManagerGL::writeImage(const GenericImageGL& imageData, const char* filePath)
{
	FILE* file = nullptr;
	errno_t err;
	if ((err = fopen_s(&file, filePath, "w+b")) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file " + std::string(filePath)));

	std::fwrite(&imageData, sizeof(GenericImageGL), 1, file);
	std::fwrite(imageData.pixels, imageData.bufSize, 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file " + std::string(filePath)));

	fclose(file);
}

TextureManagerGL::~TextureManagerGL()
{
	delete mDefaultImageSampler;
}

void TextureManagerGL::init()
{
	//GLuint sampler = mDefaultImageSampler.getID();
	//glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);

	mDefaultImageSampler = new SamplerGL();
	mDefaultImageSampler->setMinFilter(TextureFilter::Linear_Mipmap_Linear);
	mDefaultImageSampler->setMagFilter(TextureFilter::Linear);
	mDefaultImageSampler->setWrapR(TextureUVTechnique::Repeat);
	mDefaultImageSampler->setWrapS(TextureUVTechnique::Repeat);
	mDefaultImageSampler->setWrapT(TextureUVTechnique::Repeat);
	mDefaultImageSampler->setAnisotropy(16.0f);
}

CubeMapGL * TextureManagerGL::addCubeMap(CubeMapGL cubemap)
{
	cubeMaps.emplace_back(move(cubemap));
	return &cubeMaps.back();
}

CubeMapGL* TextureManagerGL::createCubeMap(const string& right, const string& left, const string& top,
	const string& bottom, const string& back, const string& front, bool useSRGBOnCreation)
{

	string rightCStr = mFileSystem->resolvePath(right).generic_string();
	string leftCStr = mFileSystem->resolvePath(left).generic_string();
	string topCStr = mFileSystem->resolvePath(top).generic_string();
	string bottomCStr = mFileSystem->resolvePath(bottom).generic_string();
	string backCStr = mFileSystem->resolvePath(back).generic_string();
	string frontCStr = mFileSystem->resolvePath(front).generic_string();

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
	return getImage("_intern/black.png", 
		{
			TextureFilter::Linear_Mipmap_Linear,
			TextureFilter::Linear,
			TextureUVTechnique::Repeat,
			ColorSpace::RGB,
			PixelDataType::UBYTE,
			InternFormat::RGB8,
			true
		});
}

TextureGL * TextureManagerGL::getDefaultNormalTexture()
{
	//normal maps shouldn't use mipmaps (important for shading!)
	return getImage("_intern/default_normal.png", 
		{
			TextureFilter::Linear_Mipmap_Linear,
			TextureFilter::Linear,
			TextureUVTechnique::Repeat,
			ColorSpace::RGB,
			PixelDataType::UBYTE,
			InternFormat::RGB8,
			true
		});
}

TextureGL * TextureManagerGL::getDefaultWhiteTexture()
{
	return getImage("_intern/white.png", 
		{
			TextureFilter::Linear_Mipmap_Linear, 
			TextureFilter::Linear, 
			TextureUVTechnique::Repeat, 
			ColorSpace::RGB, 
			PixelDataType::UBYTE, 
			InternFormat::RGB8, 
			true 
		});
}


TextureGL* TextureManagerGL::getHDRImage(const string& file, TextureData data)
{
	auto it = textureLookupTable.find(file);

	// Don't create duplicate textures!
	if (it != textureLookupTable.end())
	{
		return it->second;
	}

	string path = mFileSystem->resolvePath(file).generic_string();


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
	GLuint internalFormat = static_cast<GLuint>(data.internalFormat);
	GLuint pixelDataType = static_cast<GLuint>(data.pixelDataType);

	GLCall(glActiveTexture(GL_TEXTURE0));
	glGenTextures(1, &hdrTexture);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, pixelDataType, rawData);

	GLint minFilter = static_cast<GLuint>(data.minFilter);
	GLint magFilter = static_cast<GLuint>(data.magFilter);
	GLint uvTechnique = static_cast<GLuint>(data.uvTechnique);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uvTechnique);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uvTechnique);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);

	if (data.generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	stbi_image_free(rawData);

	return createTextureGL(file, hdrTexture);
}

TextureGL* TextureManagerGL::getImage(const string& file, TextureData data)
{
	if (data.pixelDataType == PixelDataType::FLOAT) {
		return getHDRImage(file, data);
	}

	auto it = textureLookupTable.find(file);

	// Don't create duplicate textures!
	if (it != textureLookupTable.end())
	{
		return it->second;
	}


	const auto resolvedPath = mFileSystem->resolvePath(file);



	stbi_set_flip_vertically_on_load(true); // opengl uses texture coordinates with origin at bottom left 
	int width, height, nrComponents;
	unsigned char* rawData = stbi_load(resolvedPath.generic_string().c_str(), &width, &height, &nrComponents, 0);
	unsigned int textureID;
	if (!rawData) {
		LOG(m_logger, Fault) << "Couldn't load image file: " << file << endl;
		stringstream ss;
		ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
		throw_with_trace(runtime_error(ss.str()));
	}


	GLuint format = TextureGL::getFormat(nrComponents);
	GLuint pixelDataType = static_cast<GLuint>(data.pixelDataType);
	GLuint internalFormat = static_cast<GLuint>(data.internalFormat);

	GLuint minFilter = static_cast<GLuint>(data.minFilter);
	GLuint magFilter = static_cast<GLuint>(data.magFilter);
	GLuint uvTechnique = static_cast<GLuint>(data.uvTechnique);



	GLCall(glActiveTexture(GL_TEXTURE0));
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, pixelDataType, rawData);

	if (data.generateMipMaps)
		glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uvTechnique);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uvTechnique);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, 0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	stbi_image_free(rawData);

	float aniso = 0.0f;
	//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);

	LOG(m_logger, Debug) << "texture to load: " << resolvedPath;

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);

	return createTextureGL(file, textureID);
}

void TextureManagerGL::init(FileSystem* textureFileSystem)
{
	mFileSystem = textureFileSystem;
}

void TextureManagerGL::loadImages(const string& imageFolder)
{
	//TODO!
}

SamplerGL* TextureManagerGL::getDefaultImageSampler()
{
	return mDefaultImageSampler;
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

	SamplerGL* sampler = m_textureManager->getDefaultImageSampler();
	float anisotropy = sampler->getAnisotropy();

	//float anisotropyBackup = anisotropy;

	// render configuration properties
	ImGui::PushID(m_id.c_str());
	if (ImGui::InputFloat("Anisotropic Filtering (read-only)", &anisotropy, 1.0f, 1.0f, "%.3f")) //ImGuiInputTextFlags_ReadOnly
	{
		sampler->setAnisotropy(anisotropy);
	}
	ImGui::PopID();

	//throw_with_trace(std::runtime_error("Hello exception!"));
}