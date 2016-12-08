#ifndef ENGINE_VOB_VOB_HPP
#define ENGINE_VOB_VOB_HPP
#include <mesh/Mesh.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 * A model is an object in a 3d space, thus it consists of a mesh representing the shape
 * and it consists also of a transformation matrix representing the orientation and 
 * position of the object in the 3d space.
 */
class Vob
{
public:
	explicit Vob(std::string meshName);
	Vob(const Vob& other);
	Vob(Vob&& other);
	Vob& operator=(const Vob& other);
	Vob& operator=(Vob&& other);

	virtual ~Vob();

	/**
	 * Calculates the transformatino matrix of this model
	 * based on its position, scale and rotation.
	 */
	void calcTrafo();

	/** 
	 * Provides the name of the 3d mesh this model uses.
	 */
	 std::string const& getMeshName() const;

	/**
	 * Provides read/write access to the transformation matrix
	 * of this model.
	 */
	 glm::mat4 const& getTrafo() const;

	 /**
	  * Sets the rotation measured in euler angles int XYZ order.
	  */
	 void setEulerXYZ(glm::vec3 rotation);

	 /**
	  * Sets the position of this model.
	  */
	 void setPosition(glm::vec3 position);

	 /**
	  * Sets the scale of this model.
	  */
	 void setScale(glm::vec3 scale);

	 /**
	  * Sets the transformation matrix of this model.
	  */
	 void setTrafo(glm::mat4 mat);

protected:
	std::string meshName;
	glm::quat orientation;
	glm::vec3 position;
	glm::vec3 scale;
	glm::mat4 trafo;
};
#endif