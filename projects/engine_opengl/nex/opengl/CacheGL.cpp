#include <nex/opengl/CacheGL.hpp>
#include <nex/util/Macro.hpp>
#include <nex/opengl/opengl.hpp>

nex::CacheError::CacheError(const std::string& _Message): runtime_error(_Message)
{
}

nex::CacheError::CacheError(const char* _Message): runtime_error(_Message)
{
}

void nex::GlobalCacheGL::BindTextureUnit(GLuint unit, GLuint texture)
{
	static std::unordered_map<GLuint, GLuint> cache;
	const auto it = cache.find(unit);

	if (it == cache.end() || it->second != texture)
	{
		cache[unit] = texture;
		GLCall(glBindTextureUnit(unit, texture));
	}
}

nex::ShaderCacheGL::ShaderCacheGL(GLuint program) : mProgram(program)
{
	if (mGlobalCache == nullptr)
	{
		mGlobalCache = nex::GlobalCacheGL::get();
	}
}


nex::GlobalCacheGL* nex::ShaderCacheGL::mGlobalCache = nullptr;

void nex::ShaderCacheGL::UseProgram()
{
	mGlobalCache->UseProgram(mProgram);
}

void nex::ShaderCacheGL::Uniform1i(GLint location, GLint value)
{
	EUCLID_DEBUG(assertActiveProgram());

	const auto it = mUniform1iValues.find(location);

	if (it == mUniform1iValues.end() || it->second != value)
	{
		mUniform1iValues[location] = value;
		GLCall(glUniform1i(location, value));
	}
}

void nex::ShaderCacheGL::assertActiveProgram()
{
	if (mGlobalCache->getActiveProgram() != mProgram)
	{
		throw CacheError("nex::ShaderCacheGL::assertActiveProgram: active program and shader program of a cache don't match!");
	}
}

nex::GlobalCacheGL::GlobalCacheGL() : mActiveProgram(GL_FALSE)
{
	GLCall(glGetIntegerv(GL_CURRENT_PROGRAM, reinterpret_cast<GLint*>(&mActiveProgram)));
}

void nex::GlobalCacheGL::UseProgram(GLuint program)
{
	if (program != mActiveProgram)
	{
		mActiveProgram = program;
		GLCall(glUseProgram(mActiveProgram));
	}
}

GLuint nex::GlobalCacheGL::getActiveProgram() const 
{
	return mActiveProgram;
}

nex::GlobalCacheGL* nex::GlobalCacheGL::get()
{
	static GlobalCacheGL cache;
	return &cache;
}