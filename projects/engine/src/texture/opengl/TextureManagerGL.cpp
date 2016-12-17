#include <texture/opengl/TextureManagerGL.hpp>
#include <SOIL2/SOIL2.h>
#include <util/Globals.hpp>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;
using namespace platform;

unique_ptr<TextureManagerGL> TextureManagerGL::instance = make_unique<TextureManagerGL>(TextureManagerGL());

TextureManagerGL::TextureManagerGL() : TextureManager(), logClient(getLogServer())
{
	textureLookupTable = map<string, TextureGL*>();
	logClient.setPrefix("[TextureManagerGL]");
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

CubeMap* TextureManagerGL::createCubeMap(const string& right, const string& left, const string& top,
	const string& bottom, const string& back, const string& front)
{
	string rightCStr = ::util::globals::TEXTURE_PATH + right;
	string leftCStr = ::util::globals::TEXTURE_PATH + left;
	string topCStr = ::util::globals::TEXTURE_PATH + top;
	string bottomCStr = ::util::globals::TEXTURE_PATH + bottom;
	string backCStr = ::util::globals::TEXTURE_PATH + back;
	string frontCStr = ::util::globals::TEXTURE_PATH + front;

	GLuint cubeMap = SOIL_load_OGL_cubemap(rightCStr.c_str(), leftCStr.c_str(), topCStr.c_str(),
		bottomCStr.c_str(), backCStr.c_str(), frontCStr.c_str(),
		SOIL_LOAD_RGB, 0, 0);

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

	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	cubeMaps.push_back(CubeMapGL(cubeMap));

	return &cubeMaps.back();
}

TextureGL* TextureManagerGL::getImageGL(const string& file)
{
	return static_cast<TextureGL*>(getImage(file));
}

Texture* TextureManagerGL::getImage(const string& file)
{
	auto it = textureLookupTable.find(file);

	// Don't create duplicate textures!
	if (it != textureLookupTable.end())
	{
		return it->second;
	}

	string path = ::util::globals::TEXTURE_PATH + file;

	GLuint textureID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_RGBA, 0, SOIL_FLAG_INVERT_Y | SOIL_FLAG_GL_MIPMAPS);
	if (textureID == GL_FALSE)
	{
		LOG(logClient, Fault) << "Couldn't load image file: " << file << endl;
		stringstream ss;
		ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
		throw runtime_error(ss.str());
	}

	glBindTexture(GL_TEXTURE_2D, textureID); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
										   // Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
																	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	textures.push_back(TextureGL(textureID));
	TextureGL* pointer = &textures.back();
	textureLookupTable.insert(pair<string, TextureGL*>(file, pointer));
	return pointer;
}

void TextureManagerGL::loadImages(const string& imageFolder)
{
	//TODO!
}

TextureManagerGL* TextureManagerGL::get()
{
	return instance.get();
}