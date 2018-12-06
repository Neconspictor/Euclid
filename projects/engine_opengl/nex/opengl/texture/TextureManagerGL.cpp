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

#include <gli/gli.hpp>
#include <gli/texture2d.hpp>
#include <gli/load.hpp>
#include <gli/save.hpp>

#include <nex/FileSystem.hpp>

using namespace std;
using namespace nex;


TextureManagerGL::TextureManagerGL() : m_logger("TextureManagerGL"), mDefaultImageSampler(nullptr), mFileSystem(nullptr)
{
	textureLookupTable = map<string, Texture*>();

	//TextureManagerGL::setAnisotropicFiltering(m_anisotropy);
}

void TextureManagerGL::releaseTexture(Texture * tex)
{
	for (auto&& it = textures.begin(); it != textures.end(); ++it) {
		if ((*it) == tex) {
			delete *it;
			textures.erase(it);
			break; // we're done
		}
	}
}

void TextureManagerGL::writeHDR(const GenericImage& imageData, const char* filePath)
{
	stbi__flip_vertically_on_write = true;
	stbi_write_hdr(filePath, imageData.width, imageData.height, imageData.components, (float*)imageData.pixels.get());
}

void TextureManagerGL::readImage(GenericImage* imageData, const char* filePath)
{
	FILE* file = nullptr;
	errno_t err;
	if ((err = fopen_s(&file, filePath, "rb")) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));

	std::fread(imageData, sizeof(GenericImage), 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));


	imageData->pixels = new char[imageData->bufSize];

	std::fread(imageData->pixels.get(), imageData->bufSize, 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't read from file " + std::string(filePath)));

	fclose(file);
}

void TextureManagerGL::writeImage(const GenericImage& imageData, const char* filePath)
{
	FILE* file = nullptr;
	errno_t err;
	if ((err = fopen_s(&file, filePath, "w+b")) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file " + std::string(filePath)));

	std::fwrite(&imageData, sizeof(GenericImage), 1, file);
	std::fwrite(imageData.pixels.get(), imageData.bufSize, 1, file);

	if (std::ferror(file) != 0)
		throw_with_trace(std::runtime_error("Couldn't write to file " + std::string(filePath)));

	fclose(file);
}

void TextureManagerGL::readGLITest(const char* filePath)
{
	gli::texture tex = gli::load(filePath);

	if (tex.empty())
	{
		LOG(m_logger, Error) << "Couldn't load file" << filePath;
		return;
	}

	gli::texture2d tex2D(tex);

	if (tex2D.empty())
	{
		LOG(m_logger, Error) << "Couldn't create texture 2D from " << filePath;
	}

	gli::gl GL(gli::gl::PROFILE_GL33);
	GLenum target = GL.translate(tex.target());
	gli::gl::format const format = GL.translate(tex.format(), tex.swizzles());
}

TextureManagerGL::~TextureManagerGL()
{
	release();
}

void TextureManagerGL::init()
{
	//GLuint sampler = mDefaultImageSampler.getID();
	//glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);

	mDefaultImageSampler = Sampler::create({});
	mDefaultImageSampler->setMinFilter(TextureFilter::Linear_Mipmap_Linear);
	mDefaultImageSampler->setMagFilter(TextureFilter::Linear);
	mDefaultImageSampler->setWrapR(TextureUVTechnique::Repeat);
	mDefaultImageSampler->setWrapS(TextureUVTechnique::Repeat);
	mDefaultImageSampler->setWrapT(TextureUVTechnique::Repeat);
	mDefaultImageSampler->setAnisotropy(16.0f);
}

void TextureManagerGL::addCubeMap(CubeMap* cubemap)
{
	cubeMaps.push_back(cubemap);
}

CubeMap* TextureManagerGL::createCubeMap(const string& right, const string& left, const string& top,
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

Texture * TextureManagerGL::getDefaultBlackTexture()
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

Texture * TextureManagerGL::getDefaultNormalTexture()
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

Texture * TextureManagerGL::getDefaultWhiteTexture()
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


Texture* TextureManagerGL::getHDRImage(const string& file, const TextureData& data)
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

	if (!rawData) {
		LOG(m_logger, Fault) << "Couldn't load image file: " << file << endl;
		stringstream ss;
		ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
		throw_with_trace(runtime_error(ss.str()));
	}

	Texture* texture = Texture::createTexture2D(width, height, data, rawData);
	stbi_image_free(rawData);

	LOG(m_logger, Debug) << "texture to load: " << path;

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);

	textures.push_back(texture);
	textureLookupTable.insert(std::pair<std::string, nex::Texture*>(path, texture));

	return texture;
}

Texture* TextureManagerGL::getImage(const string& file, const TextureData& data)
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


	const auto resolvedPath = mFileSystem->resolvePath(file).generic_string();

	stbi_set_flip_vertically_on_load(true); // opengl uses texture coordinates with origin at bottom left 
	int width, height, nrComponents;
	unsigned char* rawData = stbi_load(resolvedPath.c_str(), &width, &height, &nrComponents, 0);
	if (!rawData) {
		LOG(m_logger, Fault) << "Couldn't load image file: " << file << endl;
		stringstream ss;
		ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
		throw_with_trace(runtime_error(ss.str()));
	}


	//GLuint format = TextureGL::getFormat(nrComponents);

	Texture* texture = Texture::createTexture2D(width, height, data, rawData);
	stbi_image_free(rawData);

	LOG(m_logger, Debug) << "texture to load: " << resolvedPath;

	RendererOpenGL::checkGLErrors(BOOST_CURRENT_FUNCTION);

	textures.push_back(texture);
	textureLookupTable.insert(std::pair<std::string, nex::Texture*>(resolvedPath, texture));

	return texture;
}

void TextureManagerGL::init(FileSystem* textureFileSystem)
{
	mFileSystem = textureFileSystem;
}

void TextureManagerGL::loadImages(const string& imageFolder)
{
	//TODO!
}

Sampler* TextureManagerGL::getDefaultImageSampler()
{
	return mDefaultImageSampler;
}

TextureManagerGL* TextureManagerGL::get()
{
	static TextureManagerGL instance;
	return &instance;
}

void TextureManagerGL::release()
{
	for (auto& texture : textures)
	{
		delete texture;
	}

	textures.clear();

	for (auto& map : cubeMaps)
	{
		delete map;
	}

	cubeMaps.clear();

	textureLookupTable.clear();

	delete mDefaultImageSampler;
	mDefaultImageSampler = nullptr;
}


TextureManager_Configuration::TextureManager_Configuration(TextureManagerGL* textureManager) : m_textureManager(textureManager)
{
}

void TextureManager_Configuration::drawSelf()
{

	Sampler* sampler = m_textureManager->getDefaultImageSampler();
	float anisotropy = sampler->getState().anisotropy;

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