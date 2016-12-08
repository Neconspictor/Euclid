#include <texture/opengl/TextureManagerGL.hpp>
#include <SOIL2/SOIL2.h>
#include <util/Globals.hpp>
#include <algorithm>
#include <platform/logging/GlobalLoggingServer.hpp>

using namespace std;
using namespace platform;

unique_ptr<TextureManagerGL> TextureManagerGL::instance = make_unique<TextureManagerGL>(TextureManagerGL());

TextureManagerGL::TextureManagerGL() : TextureManager(), logClient(getLogServer())
{
	textures = map<string, GLuint>();
	logClient.setPrefix("[TextureManagerGL]");
}

TextureManagerGL::~TextureManagerGL()
{
	for_each(textures.begin(), textures.end(), [](pair<string, GLuint> elem) {
		glDeleteTextures(1, &elem.second);
	});
}

GLuint TextureManagerGL::getImage(const string& file)
{
	auto it = textures.find(file);

	// Don't create duplicate textures!
	if (it != textures.end())
	{
		return it->second;
	}

	string path = ::util::globals::TEXTURE_PATH + file;

	GLuint texture = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, 0, SOIL_FLAG_INVERT_Y | SOIL_FLAG_GL_MIPMAPS);
	if (texture == GL_FALSE)
	{
		LOG(logClient, Error) << "Couldn't load image file: " << file << endl;
		stringstream ss;
		ss << "TextureManagerGL::getImage(const string&): Couldn't load image file: " << file;
		throw runtime_error(ss.str());
	}

	glBindTexture(GL_TEXTURE_2D, texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
										   // Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
																	// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	textures.insert(pair<string, GLuint>(file, texture));
	return texture;
}

void TextureManagerGL::loadImages(const string& imageFolder)
{
	//TODO!
}

TextureManagerGL* TextureManagerGL::get()
{
	return instance.get();
}