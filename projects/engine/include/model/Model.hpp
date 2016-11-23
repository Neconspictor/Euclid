#ifndef ENGINE_MODEL_MODEL_HPP
#define ENGINE_MODEL_MODEL_HPP
#include <mesh/Mesh.hpp>
#include <glm/glm.hpp>

/**
 * A model is an object in a 3d space, thus it consists of a mesh representing the shape
 * and it consists also of a transformation matrix representing the orientation and 
 * position of the object in the 3d space.
 */
class Model
{
public:
	explicit Model(std::string meshName, glm::mat4 = glm::mat4());
	virtual ~Model(){}

	/** 
	 * Provides the name of the 3d mesh this model uses.
	 */
	 std::string const& getMeshName() const;

	/**
	 * Provides read/write access to the transformation matrix
	 * of this model.
	 */
	 glm::mat4 const& getTrafo() const;

	 void setTrafo(glm::mat4 mat);

protected:
	glm::mat4 trafo;
	std::string meshName;
};
#endif