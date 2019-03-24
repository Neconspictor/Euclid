//use stb_image -- TODO: replace SOIL completely with this library
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include <stb/stb_image_write.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

//#define STB_DEFINE                                                     
//#include <stb/stb.h>

#include <nex/util/ExceptionHandling.hpp>
#include <imgui/imgui.h>

/*#include <gli/gli.hpp>
#include <gli/texture2d.hpp>
#include <gli/load.hpp>
#include <gli/save.hpp>*/

#include <nex/FileSystem.hpp>
#include <nex/texture/Image.hpp>
#include <nex/texture/TextureManager.hpp>
#include <nex/texture/TextureSamplerData.hpp>
#include <nex/texture/Sampler.hpp>
#include <nex/texture/Texture.hpp>

using namespace std;

namespace nex {


	TextureManager::TextureManager() : m_logger("TextureManagerGL"), mDefaultImageSampler(nullptr), mFileSystem(nullptr)
	{
	}

	void TextureManager::releaseTexture(Texture * tex)
	{
		for (auto&& it = textures.begin(); it != textures.end(); ++it) {
			if ((&*it) == tex) {
				textures.erase(it);
				break; // we're done
			}
		}
	}

	void TextureManager::writeHDR(const GenericImage& imageData, const char* filePath)
	{
		stbi__flip_vertically_on_write = true;
		stbi_write_hdr(filePath, imageData.width, imageData.height, imageData.components, (float*)imageData.pixels.get());
	}

	void TextureManager::readImage(GenericImage* imageData, const char* filePath)
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

	void TextureManager::writeImage(const GenericImage& imageData, const char* filePath)
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

	/*void TextureManager::readGLITest(const char* filePath)
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
	}*/

	TextureManager::~TextureManager()
	{
		release();
	}

	void TextureManager::init()
	{
		//GLuint sampler = mDefaultImageSampler.getID();
		//glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);

		mDefaultImageSampler = std::make_unique<Sampler>(SamplerDesc());
		mDefaultImageSampler->setMinFilter(TextureFilter::Linear_Mipmap_Linear);
		mDefaultImageSampler->setMagFilter(TextureFilter::Linear);
		mDefaultImageSampler->setWrapR(TextureUVTechnique::Repeat);
		mDefaultImageSampler->setWrapS(TextureUVTechnique::Repeat);
		mDefaultImageSampler->setWrapT(TextureUVTechnique::Repeat);
		mDefaultImageSampler->setAnisotropy(16.0f);

		mPointSampler = std::make_unique<Sampler>(SamplerDesc());
		mPointSampler->setMinFilter(TextureFilter::NearestNeighbor);
		mPointSampler->setMagFilter(TextureFilter::NearestNeighbor);
		mPointSampler->setWrapR(TextureUVTechnique::ClampToEdge);
		mPointSampler->setWrapS(TextureUVTechnique::ClampToEdge);
		mPointSampler->setWrapT(TextureUVTechnique::ClampToEdge);
		mPointSampler->setAnisotropy(1.0f);
	}

	CubeMap* TextureManager::createCubeMap(const string& right, const string& left, const string& top,
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

	void TextureManager::flipYAxis(char* imageSource, size_t pitch, size_t height)
	{
		std::vector<char> backupRowVec(pitch);
		auto* backupRow = backupRowVec.data();

		for (size_t i = 0; i < height; ++i)
		{
			// backup current row
			auto* currentRow = imageSource + i * pitch;
			memcpy_s(backupRow, pitch, currentRow, pitch);

			// get the y-axis mirrored row
			auto mirroredRowIndex = height - i - 1;
			auto* mirroredRow = imageSource + mirroredRowIndex * pitch;

			// Replace the current row with the mirrored row
			memcpy_s(currentRow, pitch, mirroredRow, pitch);

			// Replace the mirrored row with the backup
			memcpy_s(mirroredRow, pitch, backupRow, pitch);
		}
	}

	Texture2D * TextureManager::getDefaultBlackTexture()
	{
		return getImage("_intern/black.png",
			{
				TextureFilter::Linear_Mipmap_Linear,
				TextureFilter::Linear,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				ColorSpace::RGB,
				PixelDataType::UBYTE,
				InternFormat::RGB8,
				true
			});
	}

	Texture2D * TextureManager::getDefaultNormalTexture()
	{
		//normal maps shouldn't use mipmaps (important for shading!)
		return getImage("_intern/default_normal.png",
			{
				TextureFilter::Linear_Mipmap_Linear,
				TextureFilter::Linear,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				ColorSpace::RGB,
				PixelDataType::UBYTE,
				InternFormat::RGB8,
				true
			});
	}

	Texture2D * TextureManager::getDefaultWhiteTexture()
	{
		return getImage("_intern/white.png",
			{
				TextureFilter::Linear_Mipmap_Linear,
				TextureFilter::Linear,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				TextureUVTechnique::Repeat,
				ColorSpace::RGB,
				PixelDataType::UBYTE,
				InternFormat::RGB8,
				true
			});
	}


	Texture2D* TextureManager::getHDRImage(const string& file, const TextureData& data)
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

		textures.emplace_back(std::move(Texture2D(width, height, data, rawData)));

		stbi_image_free(rawData);

		LOG(m_logger, Debug) << "texture to load: " << path;


		auto* result = &textures.back();

		textureLookupTable.insert(std::pair<std::string, nex::Texture2D*>(path, result));

		return result;
	}

	Texture2D* TextureManager::getImage(const string& file, const TextureData& data)
	{
		if (data.pixelDataType == PixelDataType::FLOAT) {
			return getHDRImage(file, data);
		}


		const auto resolvedPath = mFileSystem->resolvePath(file).generic_string();

		auto it = textureLookupTable.find(resolvedPath);

		// Don't create duplicate textures!
		if (it != textureLookupTable.end())
		{
			return it->second;
		}

		stbi_set_flip_vertically_on_load(true); // opengl uses texture coordinates with origin at bottom left 
		int width, height, nrComponents;
		const unsigned requiredComponents  = getComponents(data.colorspace);
		unsigned char* rawData = stbi_load(resolvedPath.c_str(), &width, &height, &nrComponents, requiredComponents);
		if (!rawData) {
			LOG(m_logger, Fault) << "Couldn't load image file: " << file << endl;
			stringstream ss;
			ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
			throw_with_trace(runtime_error(ss.str()));
		}


		//GLuint format = TextureGL::getFormat(nrComponents);

		textures.emplace_back(std::move(Texture2D(width, height, data, rawData)));
		stbi_image_free(rawData);

		LOG(m_logger, Debug) << "texture to load: " << resolvedPath;


		auto* result = &textures.back();

		textureLookupTable.insert(std::pair<std::string, nex::Texture2D*>(resolvedPath, result));

		return result;
	}

	std::unique_ptr<nex::Texture2D> TextureManager::loadImage(const std::string& file, bool flip, const nex::TextureData& data)
	{
		const auto resolvedPath = mFileSystem->resolvePath(file).generic_string();

		stbi_set_flip_vertically_on_load(flip); // opengl uses texture coordinates with origin at bottom left 
		int width, height, nrComponents;
		const unsigned requiredComponents = getComponents(data.colorspace);
		unsigned char* rawData = stbi_load(resolvedPath.c_str(), &width, &height, &nrComponents, requiredComponents);
		if (!rawData) {
			LOG(m_logger, Fault) << "Couldn't load image file: " << file << endl;
			stringstream ss;
			ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
			throw_with_trace(runtime_error(ss.str()));
		}

		auto texture = std::make_unique<Texture2D>(width, height, data, rawData);
		stbi_image_free(rawData);

		return texture;
	}

	void TextureManager::init(FileSystem* textureFileSystem)
	{
		mFileSystem = textureFileSystem;
	}

	void TextureManager::loadImages(const string& imageFolder)
	{
		//TODO!
	}

	Sampler* TextureManager::getDefaultImageSampler()
	{
		return mDefaultImageSampler.get();
	}
	Sampler* TextureManager::getPointSampler()
	{
		return mPointSampler.get();
	}

	TextureManager* TextureManager::get()
	{
		static TextureManager instance;
		return &instance;
	}

	void TextureManager::release()
	{
		textures.clear();
		cubeMaps.clear();

		textureLookupTable.clear();
		mDefaultImageSampler.reset(nullptr);
		mPointSampler.reset(nullptr);
	}


	TextureManager_Configuration::TextureManager_Configuration(TextureManager* textureManager) : m_textureManager(textureManager)
	{
	}

	void TextureManager_Configuration::drawSelf()
	{

		Sampler* sampler = m_textureManager->getDefaultImageSampler();
		float anisotropy = sampler->getState().maxAnisotropy;

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
}