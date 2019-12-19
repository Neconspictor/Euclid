#pragma once

namespace nex
{

	class Shader;

	class ShaderProvider {
	public:

		ShaderProvider(Shader* shader) {
			mCurrentShader = shader;
		}

		virtual ~ShaderProvider() = default;

		constexpr Shader* getShader() {
			return mCurrentShader;
		};

		constexpr const Shader* getShader() const {
			return mCurrentShader;
		};

		void setShader(Shader* shader) {
			mCurrentShader = shader;
		}

	protected:
		Shader* mCurrentShader;
	};


	template<class ShaderType>
	class TypedShaderProvider : public ShaderProvider {
	public:

		TypedShaderProvider(ShaderType* shader) : ShaderProvider(shader) {
		}

		virtual ~TypedShaderProvider() = default;

		ShaderType* getShader() {
			return static_cast<ShaderType*>(mCurrentShader);
		}
	};

	template<class ShaderType>
	class TypedOwningShaderProvider : public TypedShaderProvider<ShaderType> {
	public:

		TypedOwningShaderProvider(std::unique_ptr<ShaderType> shader) : TypedShaderProvider(shader.get()) 
		{
			setOwningShader(std::move(shader));
		}

		virtual ~TypedOwningShaderProvider() = default;

		void setOwningShader(std::unique_ptr<ShaderType> shader) {
			mShader = std::move(shader);
			setShader(mShader.get());
		}

	protected:
		std::unique_ptr<ShaderType> mShader;
		
	};

};