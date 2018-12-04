#pragma once
#include <map>
#include <memory>
#include <nex/shader/Shader.hpp>
#include <nex/common/Log.hpp>

namespace nex
{
	/**
	 * An opengl implementation of a shader manager
	 */
	 /**
	  * An interface for creating, receiving and storing renderer independent shaders
	  */
	class ShaderManagerGL
	{
	public:
		virtual ~ShaderManagerGL();

		/**
		 * Provides a singleton of a shader by its shader enumeration.
		 * NOTE: A ShaderInitException can be thrown if the specified has to be created but
		 * an error occured during initialization.
		 */
		Shader* getShader(ShaderType shader);

		/**
		 * Loads all shaders.
		 * NOTE: A ShaderInitException is thrown if one shaders couldn't be created.
		 */
		void loadShaders();

		/**
		* Provides access the shader manager singleton.
		*/
		static ShaderManagerGL* get();

	private:
		std::map<ShaderType, Shader*> shaderMap;
		nex::Logger m_logger;

		// this class is a singleton, thus private constructor
		ShaderManagerGL();

		/**
		 * Creates a shader by its enum and registers it to the shader map.
		 * @return : A pointer to the created shader.
		 *
		 * NOTE: A ShaderInitException will be thrown if the shader can't be created.
		 */
		Shader* createShader(ShaderType shaderEnum);

		static std::unique_ptr<ShaderManagerGL> instance;
	};
}
