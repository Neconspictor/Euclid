#include "texture/TextureManager.hpp"
#include <iostream>
#include <SOIL2/SOIL2.h>
#include <algorithm>
#include <util/GlobalPaths.hpp>

using namespace std;

TextureManager* TextureManager::instance = nullptr;

TextureManager::TextureManager()
{
	textures = map<string, GLuint>();
}

TextureManager::~TextureManager()
{
}

bool TextureManager::init()
{
	return true;
}

TextureManager* TextureManager::getInstance()
{
	if (instance == nullptr)
	{
		instance = new TextureManager();
		instance->init();
	}

	return instance;
}

// Function load a image, turn it into a texture, and return the texture ID as a GLuint for use
GLuint TextureManager::loadImage(const string& file)
{
	auto it = textures.find(file);

	// Don't create duplicate textures!
	if (it != textures.end())
	{
		return it->second;
	}

	string path = util::global_path::TEXTURE_PATH + file;

	GLuint texture = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, 0, SOIL_FLAG_INVERT_Y | SOIL_FLAG_GL_MIPMAPS);
	if (texture == GL_FALSE)
	{
		cerr << "Error: TextureManager::loadImage(): Couldn't load image file: " << file << endl;
	}

	glBindTexture(GL_TEXTURE_2D, texture); // All upcoming GL_TEXTURE_2D operations now have effect on our texture object
										   // Set our texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);    // Note that we set our container wrapping method to GL_CLAMP_TO_EDGE
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);    // Note that we set our container wrapping method to GL_CLAMP_TO_EDGE
																			// Set texture filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//glGenerateMipmap(GL_TEXTURE_2D);

	textures.insert(pair<string, GLuint>(file, texture));

	/*int width, height;
	unsigned char* image = SOIL_load_image(file.c_str(), &width, &height, nullptr, SOIL_LOAD_RGBA);
	if (image == nullptr)
	{
		std::cout << "Error: TextureManager::loadImage: Couldn't load image: " << file << std::endl;
		return GL_FALSE;
	}

	GLuint texture = GL_FALSE;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);*/
	return texture;
}

void TextureManager::release()
{
	if (instance == nullptr) return;
	for_each(instance->textures.begin(), instance->textures.end(), [](pair<string, GLuint> elem) {
		glDeleteTextures(1, &elem.second);
	});
	delete instance;
	instance = nullptr;
}