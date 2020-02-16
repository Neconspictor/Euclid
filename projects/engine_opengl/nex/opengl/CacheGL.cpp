#include <nex/opengl/CacheGL.hpp>
#include <nex/util/Macro.hpp>
#include <nex/opengl/opengl.hpp>

nex::CacheError::CacheError(const std::string& _Message): runtime_error(_Message)
{
}

nex::CacheError::CacheError(const char* _Message): runtime_error(_Message)
{
}

void nex::GlobalCacheGL::BindDrawFramebuffer(GLuint framebuffer, bool ignoreErrors)
{
	GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer));
	return;

	if (mActiveDrawFrameBuffer != framebuffer)
	{
		mActiveDrawFrameBuffer = framebuffer;
		if (!ignoreErrors) {
			GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer));
		}
		else {
			nex::GLClearError();
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
			nex::GLClearError();
		}
	}
}

void nex::GlobalCacheGL::BindFramebuffer(GLuint framebuffer, bool rebind)
{
	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));
	return;

	if (mActiveDrawFrameBuffer != framebuffer || mActiveReadFrameBuffer != framebuffer || rebind)
	{
		mActiveDrawFrameBuffer = framebuffer;
		mActiveReadFrameBuffer = framebuffer;
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer));
		//GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer));
	}
}

void nex::GlobalCacheGL::BindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer,
	GLenum access, GLenum format)
{

	GLCall(glBindImageTexture(unit,
		texture,
		level,
		layered,
		layer,
		access,
		format));

	return;


	struct Item
	{
		GLuint unit; 
		GLuint texture; 
		GLint level; 
		GLboolean layered; 
		GLint layer;
		GLenum access; 
		GLenum format;
	};

	static std::unordered_map<GLuint, Item> cache;

	Item* item;
	const auto it = cache.find(unit);

	if (it == cache.end())
	{
		cache[unit] = Item();
		item = &cache.find(unit)->second;
	} else
	{
		item = &it->second;
	}

	if (item->texture != texture 
		|| item->level != level
		|| item->layered != layer
		|| item->access != access
		|| item->format != format)
	{
		item->texture = texture;
		item->level = level;
		item->layered = layered;
		item->access = access;
		item->format = format;

		GLCall(glBindImageTexture(unit,
			item->texture,
			item->level,
			item->layered,
			item->layer,
			item->access,
			item->format));
	}
}

void nex::GlobalCacheGL::BindReadFramebuffer(GLuint framebuffer, bool ignoreErrors)
{

	GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer));
	return;

	if (mActiveReadFrameBuffer != framebuffer)
	{
		mActiveReadFrameBuffer = framebuffer;
		if (!ignoreErrors) {
			GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer));
		} else {
			nex::GLClearError();
			glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
			nex::GLClearError();
		}
	}
}

void nex::GlobalCacheGL::BindTextureUnit(GLuint unit, GLuint texture)
{
	GLCall(glBindTextureUnit(unit, texture));
	return;

	static std::unordered_map<GLuint, GLuint> cache;
	const auto it = cache.find(unit);

	if (it == cache.end() || it->second != texture || true)
	{
		cache[unit] = texture;
		static bool test = false;
		if (test) {
			std::cout << "unit = " << unit << ", texture = " << texture << std::endl;
		}
		GLCall(glBindTextureUnit(unit, texture));
	}
}

GLint nex::GlobalCacheGL::GetConstInteger(GLenum pname)
{
	GLint result;
	GLCall(glGetIntegerv(pname, &result));
	return result;

	// check if already cached
	auto it = mConstIntegers.find(pname);
	if (it != mConstIntegers.end())
		return it->second;

	// retrieve value
	//GLint result;
	GLCall(glGetIntegerv(pname, &result));
	mConstIntegers[pname] = result;

	return result;
}

nex::ShaderCacheGL::ShaderCacheGL(GLuint program) : mProgram(program)
{
}

void nex::ShaderCacheGL::Uniform1f(GLint location, GLfloat value)
{
	EUCLID_DEBUG(assertActiveProgram());
	GLCall(glUniform1f(location, value));
	return;

	const auto it = mUniform1fValues.find(location);

	if (it == mUniform1fValues.end() || it->second != value)
	{
		mUniform1fValues[location] = value;
		GLCall(glUniform1f(location, value));
	}
}


void nex::ShaderCacheGL::UseProgram()
{
	nex::GlobalCacheGL::get()->UseProgram(mProgram);
}

void nex::ShaderCacheGL::Revalidate()
{
	nex::GlobalCacheGL::get()->RevalidateProgram(mProgram);
}

void nex::ShaderCacheGL::Uniform1i(GLint location, GLint value)
{
	EUCLID_DEBUG(assertActiveProgram());
	GLCall(glUniform1i(location, value));
	return;

	const auto it = mUniform1iValues.find(location);

	if (it == mUniform1iValues.end() || it->second != value)
	{
		mUniform1iValues[location] = value;
		GLCall(glUniform1i(location, value));
	}
}

void nex::ShaderCacheGL::Uniform1ui(GLint location, GLuint value)
{
	EUCLID_DEBUG(assertActiveProgram());
	GLCall(glUniform1ui(location, value));
	return;

	const auto it = mUniform1uiValues.find(location);

	if (it == mUniform1uiValues.end() || it->second != value)
	{
		mUniform1uiValues[location] = value;
		GLCall(glUniform1ui(location, value));
	}
}

void nex::ShaderCacheGL::assertActiveProgram()
{
	auto globalCacheProgram = nex::GlobalCacheGL::get()->getActiveProgram();
	if (globalCacheProgram != mProgram)
	{
		throw CacheError("nex::ShaderCacheGL::assertActiveProgram: active program and shader program of a cache don't match!");
	}
}

nex::GlobalCacheGL::GlobalCacheGL() : mActiveProgram(GL_FALSE), mActiveDrawFrameBuffer(GL_FALSE), mActiveReadFrameBuffer(GL_FALSE)
{
	GLCall(glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&mActiveProgram)));
	GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&mActiveDrawFrameBuffer));
	GLCall(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&mActiveReadFrameBuffer));
}

void nex::GlobalCacheGL::UseProgram(GLuint program)
{
	//if (program != mActiveProgram)
	//{
		mActiveProgram = program;
		GLCall(glUseProgram(mActiveProgram));
	//}
}

void nex::GlobalCacheGL::RevalidateProgram(GLuint program)
{
	if (program == mActiveProgram) {
		mActiveProgram = 0xFFFFFFFF;
	}
}

GLuint nex::GlobalCacheGL::getActiveProgram() const 
{
	return mActiveProgram;
}

GLuint nex::GlobalCacheGL::getActiveDrawFrameBuffer() const
{
	GLCall(glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, (GLint*)&mActiveDrawFrameBuffer));
	return mActiveDrawFrameBuffer;
}

GLuint nex::GlobalCacheGL::getActiveReadFrameBuffer() const
{
	GLCall(glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, (GLint*)&mActiveReadFrameBuffer));
	return mActiveReadFrameBuffer;
}

nex::GlobalCacheGL* nex::GlobalCacheGL::get()
{
	static thread_local GlobalCacheGL cache;
	return &cache;
}