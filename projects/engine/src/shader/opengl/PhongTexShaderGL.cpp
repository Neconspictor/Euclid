#include <shader/opengl/PhongTexShaderGL.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mesh/opengl/MeshGL.hpp>
#include <texture/opengl/TextureManagerGL.hpp>
#include <glad/glad.h>

using namespace glm;
using namespace std;

PhongTexShaderGL::PhongTexShaderGL(const string& vertexShaderFile, const string& fragmentShaderFile, 
	const string& vertexShaderFileInstanced) :
	ShaderGL(vertexShaderFile, fragmentShaderFile), lightColor(1, 1, 1), lightPosition(1, 0, 0), viewPosition(0,0,0), 
	skybox(nullptr)
{
	instancedShaderProgram = loadShaders(vertexShaderFileInstanced, fragmentShaderFile, "");
}

PhongTexShaderGL::~PhongTexShaderGL()
{
}

void PhongTexShaderGL::drawInstanced(Mesh const& meshOriginal, unsigned int amount)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;

	glUseProgram(instancedShaderProgram);
	initForDrawing(meshOriginal, instancedShaderProgram);

	GLuint projectionLoc = glGetUniformLocation(instancedShaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, value_ptr(projection));

	GLuint viewLoc = glGetUniformLocation(instancedShaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, value_ptr(view));

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElementsInstanced(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr, amount);
	glBindVertexArray(0);
}

void PhongTexShaderGL::draw(Mesh const& meshOriginal)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& projection = *data.projection;
	mat4 const& view = *data.view;
	mat4 const& model = *data.model;

	glUseProgram(this->programID);
	initForDrawing(meshOriginal, programID);

	GLuint transformLoc = glGetUniformLocation(programID, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, value_ptr(projection * view * model));

	GLuint modelLoc = glGetUniformLocation(programID, "modelView");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, value_ptr(model));

	GLint normalMatrixLoc = glGetUniformLocation(programID, "normalMatrix");
	mat4 normalMatrix = transpose(inverse(model));
	glUniformMatrix4fv(normalMatrixLoc, 1, GL_FALSE, value_ptr(normalMatrix));

	glBindVertexArray(mesh.getVertexArrayObject());
	GLsizei indexSize = static_cast<GLsizei>(mesh.getIndexSize());
	glDrawElements(GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

const vec3& PhongTexShaderGL::getLightColor() const
{
	return lightColor;
}

const vec3& PhongTexShaderGL::getLightPosition() const
{
	return lightPosition;
}

void PhongTexShaderGL::release()
{
	ShaderGL::release();
}

void PhongTexShaderGL::setLightColor(vec3 color)
{
	lightColor = move(color);
}

void PhongTexShaderGL::setLightPosition(vec3 position)
{
	lightPosition = move(position);
}

void PhongTexShaderGL::setPointLightPositions(vec3* positions)
{
	for (int i = 0; i < 4; ++i)
	{
		pointLightPositions[i] = positions[i];
	}
}

void PhongTexShaderGL::setSkyBox(CubeMap* sky)
{
	this->skybox = static_cast<CubeMapGL*>(sky);
}

void PhongTexShaderGL::setSpotLightDiection(vec3 direction)
{
	spotLightDirection = move(direction);
}

void PhongTexShaderGL::initLights(GLuint programID)
{
	// == ==========================
	// Here we set all the uniforms for the 5/6 types of lights we have. We have to set them manually and index 
	// the proper PointLight struct in the array to set each uniform variable. This can be done more code-friendly
	// by defining light types as classes and set their values in there, or by using a more efficient uniform approach
	// by using 'Uniform buffer objects', but that is something we discuss in the 'Advanced GLSL' tutorial.
	// == ==========================
	// Directional light
	glUniform3f(glGetUniformLocation(programID, "dirLight.direction"), lightPosition.x, lightPosition.y, lightPosition.z);
	glUniform4f(glGetUniformLocation(programID, "dirLight.ambient"), 0.2f, 0.2f, 0.2f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "dirLight.diffuse"), 0.5f, 0.5f, 0.5f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "dirLight.specular"), 0.3f, 0.3f, 0.3f, 1.0f);
	// Point light 1
	glUniform3f(glGetUniformLocation(programID, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
	glUniform4f(glGetUniformLocation(programID, "pointLights[0].ambient"), 0.05f, 0.05f, 0.05f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "pointLights[0].diffuse"), 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "pointLights[0].specular"), 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[0].quadratic"), 0.032f);
	// Point light 2
	glUniform3f(glGetUniformLocation(programID, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
	glUniform4f(glGetUniformLocation(programID, "pointLights[1].ambient"), 0.05f, 0.05f, 0.05f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "pointLights[1].diffuse"), 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "pointLights[1].specular"), 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[1].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[1].quadratic"), 0.032f);
	// Point light 3
	glUniform3f(glGetUniformLocation(programID, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
	glUniform4f(glGetUniformLocation(programID, "pointLights[2].ambient"), 0.05f, 0.05f, 0.05f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "pointLights[2].diffuse"), 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "pointLights[2].specular"), 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[2].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[2].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[2].quadratic"), 0.032f);
	// Point light 4
	glUniform3f(glGetUniformLocation(programID, "pointLights[3].position"), pointLightPositions[3].x, pointLightPositions[3].y, pointLightPositions[3].z);
	glUniform4f(glGetUniformLocation(programID, "pointLights[3].ambient"), 0.05f, 0.05f, 0.05f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "pointLights[3].diffuse"), 0.8f, 0.8f, 0.8f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "pointLights[3].specular"), 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[3].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[3].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(programID, "pointLights[3].quadratic"), 0.032f);
	// SpotLight
	glUniform3f(glGetUniformLocation(programID, "spotLight.position"), viewPosition.x, viewPosition.y, viewPosition.z);
	glUniform3f(glGetUniformLocation(programID, "spotLight.direction"), spotLightDirection.x, spotLightDirection.y, spotLightDirection.z);
	glUniform4f(glGetUniformLocation(programID, "spotLight.ambient"), 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "spotLight.diffuse"), 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform4f(glGetUniformLocation(programID, "spotLight.specular"), 1.0f, 1.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.linear"), 0.09f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.quadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(programID, "spotLight.cutOff"), cos(radians(12.5f)));
	glUniform1f(glGetUniformLocation(programID, "spotLight.outerCutOff"), cos(radians(15.0f)));
}

void PhongTexShaderGL::use()
{
}

void PhongTexShaderGL::initForDrawing(Mesh const& meshOriginal, GLuint programID)
{
	MeshGL const& mesh = dynamic_cast<MeshGL const&>(meshOriginal);
	mat4 const& model = *data.model;

	initLights(programID);

	GLint viewPosLoc = glGetUniformLocation(programID, "viewPos");
	glUniform3f(viewPosLoc, viewPosition.x, viewPosition.y, viewPosition.z);

	// set model material data
	//GLint matAmbientLoc = glGetUniformLocation(programID, "material.ambient");
	GLint matShineLoc = glGetUniformLocation(programID, "material.shininess");

	//const vec3& ambient = material->getAmbient();
	const Material& material = mesh.getMaterial();

	TextureGL* diffuseMap = static_cast<TextureGL*>(material.getDiffuseMap());
	TextureGL* reflectionMap = static_cast<TextureGL*>(material.getReflectionMap());
	TextureGL* specularMap = static_cast<TextureGL*>(material.getSpecularMap());
	TextureGL* emissionMap = static_cast<TextureGL*>(material.getEmissionMap());

	//glUniform3f(matAmbientLoc, ambient.x, ambient.y, ambient.z);
	glUniform1f(matShineLoc, material.getShininess());

	// Bind diffuse map
	if (diffuseMap)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap->getTexture());
		glUniform1i(glGetUniformLocation(getProgramID(), "material.diffuseMap"), 0);
	}
	else {
		GLuint textureID = TextureManagerGL::get()->getImageGL("black.png")->getTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(programID, "material.diffuseMap"), 3);
	}

	// Bind emission map
	if (emissionMap)
	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, emissionMap->getTexture());
		glUniform1i(glGetUniformLocation(programID, "material.emissionMap"), 1);
	}
	else {
		GLuint textureID = TextureManagerGL::get()->getImageGL("black.png")->getTexture();
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(programID, "material.emissionMap"), 3);
	}

	// Bind reflection map
	if (reflectionMap)
	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, reflectionMap->getTexture());
		glUniform1i(glGetUniformLocation(programID, "material.reflectionMap"), 2);
	}
	else
	{
		GLuint textureID = TextureManagerGL::get()->getImageGL("black.png")->getTexture();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(programID, "material.reflectionMap"), 2);
	}

	// Bind specular map
	if (specularMap)
	{
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, specularMap->getTexture());
		glUniform1i(glGetUniformLocation(programID, "material.specularMap"), 3);
	}
	else {
		GLuint textureID = TextureManagerGL::get()->getImageGL("black.png")->getTexture();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(glGetUniformLocation(programID, "material.specularMap"), 3);
	}

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->getCubeMap());
	glUniform1i(glGetUniformLocation(programID, "skybox"), 4);
}

void PhongTexShaderGL::setViewPosition(vec3 position)
{
	viewPosition = move(position);
}
