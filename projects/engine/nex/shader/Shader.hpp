#pragma once
#include "nex/util/StringUtils.hpp"
#include <nex/util/Memory.hpp>
//namespace glm { class mat3; class mat4; class vec2; class vec3; class vec4; }

namespace nex
{

	
	class Material;
	class Texture;
	struct ShaderStageDesc;
	//class ShaderSourceFileGenerator;
	using FilePath = const char*;

	enum class ShaderStageType
	{
		COMPUTE = 0, SHADER_STAGE_FIRST = COMPUTE,
		FRAGMENT,
		GEOMETRY,
		TESSELATION_CONTROL,
		TESSELATION_EVALUATION,
		VERTEX, SHADER_STAGE_LAST = VERTEX
	};

	/**
	* Enumerates all shaders that can be used for shading models.
	*/
	enum class ShaderType
	{
		Unknown = 0,
		BlinnPhongTex,
		Pbr,
		Pbr_Deferred_Geometry,
		Pbr_Deferred_Lighting,
		Pbr_Convolution,
		Pbr_Prefilter,
		Pbr_BrdfPrecompute,
		CubeDepthMap,
		DepthMap,
		GaussianBlurHorizontal,
		GaussianBlurVertical,
		Normals,
		Shadow,
		ShadowPoint,
		SimpleColor,
		SimpleExtrude,
		Screen,
		SkyBox,
		SkyBoxEquirectangular,
		SkyBoxPanorama,
		VarianceDepthMap,
		VarianceShadow,
	};

	/**
* Maps shader enumerations to a string representation.
*/
	static const nex::util::EnumString<nex::ShaderType> shaderEnumConversion[] = {
		{nex::ShaderType::BlinnPhongTex, "BLINN_PHONG_TEX" },
		{ nex::ShaderType::Pbr, "PBR" },
		{ nex::ShaderType::Pbr_Deferred_Geometry, "PBR_DEFERRED_GEOMETRY" },
		{ nex::ShaderType::Pbr_Deferred_Lighting, "PBR_DEFERRED_LIGHTING" },
		{ nex::ShaderType::Pbr_Convolution, "PBR_CONVOLUTION" },
		{ nex::ShaderType::Pbr_Prefilter, "PBR_PREFILTER" },
		{ nex::ShaderType::Pbr_BrdfPrecompute, "PBR_BRDF_PRECOMPUTE" },
		{ nex::ShaderType::CubeDepthMap, "CUBE_DEPTH_MAP" },
		{ nex::ShaderType::DepthMap, "DEPTH_MAP" },
		{ nex::ShaderType::GaussianBlurHorizontal, "GAUSSIAN_BLUR_HORIZONTAL" },
		{ nex::ShaderType::GaussianBlurVertical, "GAUSSIAN_BLUR_VERTICAL" },
		{ nex::ShaderType::Normals, "NORMALS" },
		{ nex::ShaderType::Shadow, "SHADOW" },
		{ nex::ShaderType::ShadowPoint, "SHADOW_POINT" },
		{ nex::ShaderType::SimpleColor, "SIMPLE_COLOR" },
		{ nex::ShaderType::SimpleExtrude, "SIMPLE_EXTRUDE" },
		{ nex::ShaderType::Screen, "SCREEN" },
		{ nex::ShaderType::SkyBox, "SKY_BOX" },
		{ nex::ShaderType::SkyBoxEquirectangular, "SKY_BOX_EQUIRECTANGULAR" },
		{ nex::ShaderType::SkyBoxPanorama, "SKY_BOX_PANORAMA" },
		{ nex::ShaderType::VarianceShadow, "VARIANCE_DEPTH_MAP" },
		{ nex::ShaderType::VarianceShadow, "VARIANCE_SHADOW" }
	};

	struct TransformData
	{
		glm::mat4 const* projection = nullptr;
		glm::mat4 const* view = nullptr;
		glm::mat4 const* model = nullptr;
	};

	enum class UniformType
	{
		CUBE_MAP,
		FLOAT,
		INT,
		MAT3,
		MAT4,
		TEXTURE2D,
		TEXTURE2D_ARRAY,
		VEC2,
		VEC3,
		VEC4
	};

	class UniformLocation;

	struct Uniform
	{
		UniformLocation const* location = nullptr;
		UniformType type = UniformType::INT;
	};
	

	struct UniformTex : public Uniform
	{
		unsigned int bindingSlot = 0;
	};

	class ShaderStage
	{
	public:

		~ShaderStage();

		static nex::ShaderStage* compileShaderStage(nex::ShaderStageType type, const nex::ShaderStageDesc& desc);

		ShaderStageType getType() const
		{
			return type;
		}

	protected:
		ShaderStage();

		ShaderStageType type;

	private:
		nex::ShaderStage& operator=(const nex::ShaderStage& other) = delete;
		ShaderStage(const nex::ShaderStage& other) = delete;
	};


/**
 * Represents a shader program for an OpenGL renderer.
 */
	class ShaderProgram
	{
	public:

		/**
		 * Binds this shader program.
		 */
		void bind();

		/**
		 * Provides the uniform location by name. The memory is managed by this class and mustn't be freed manually.
		 */
		UniformLocation* getUniformLocation(const char* name);

		static ShaderProgram* create(const FilePath& vertexFile, const FilePath& fragmentFile,
			const FilePath& geometryShaderFile = "");

		static ShaderProgram* create(const std::list<ShaderStage*>& stages);

		void release();

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound 
		 */
		void setInt(const UniformLocation* locationID, int data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setFloat(const UniformLocation* locationID, float data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setVec2(const UniformLocation* locationID, const glm::vec2& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setVec3(const UniformLocation* locationID, const glm::vec3& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setVec4(const UniformLocation* locationID, const glm::vec4& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setMat3(const UniformLocation* locationID, const glm::mat3& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setMat4(const UniformLocation* locationID, const glm::mat4& data);

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void setTexture(const UniformLocation* locationID, const nex::Texture* data, unsigned int bindingSlot);

		/**
		 * Sets a name for this shader program useful when debugging this class.
		 */
		void setDebugName(const char* name);

		/**
		 * Unbinds this shader program.
		 */
		void unbind();

		/**
		 * @throws ShaderNotBoundException if this shader program isn't currently bound
		 */
		void updateBuffer(const UniformLocation* locationID, void* data, size_t bufferSize);


	protected:

		ShaderProgram();

		// We don't allow to delete shader programs by user code. 
		// By this, it is safe to specify the destructor as non-virtual (avoids vtable)
		~ShaderProgram() = default;

		bool mIsBound;
		std::string mDebugName;

	private:
		ShaderProgram& operator=(const ShaderProgram& other) = delete;
		ShaderProgram(const ShaderProgram& other) = delete;
	};


	class Shader
	{
	public:

		Shader(ShaderProgram* program = nullptr);

		virtual ~Shader();

		/**
		 * Binds this shader and the underlying shader program.
		 */
		void bind();

		ShaderProgram* getProgram();

		void setProgram(ShaderProgram* program);

		/**
		 * Unbinds this shader and the underlying shader program.
		 */
		void unbind();

		virtual void onModelMatrixUpdate(const glm::mat4& modelMatrix);

		virtual void onMaterialUpdate(const Material* material);

		// Function that should be called before render calls
		virtual void setupRenderState();

		// Reverse the state of the function setupRenderState
		// TODO
		virtual void reverseRenderState();

	protected:

		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;

		nex::Guard<ShaderProgram> mProgram;
	};

	class TransformShaderGL : public Shader
	{
	public:
		TransformShaderGL(ShaderProgram* program = nullptr);
		virtual ~TransformShaderGL() = default;

		virtual void onTransformUpdate(const TransformData& data) = 0;
	};
};

/**
	* Maps a string to a shader enum.
	* @param str: The string to be mapped
	* @return: The mapped shader enum.
	*
	* ATTENTION: If the string couldn't be mapped, a EnumFormatException
	* will be thrown.
	*/
nex::ShaderType stringToShaderEnum(const std::string& str);

/**
* Puts a string representation of a shader enum to an output stream.
*/
std::ostream& operator<<(std::ostream& os, nex::ShaderType shader);