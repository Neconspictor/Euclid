#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <nex/material/Material.hpp>


namespace nex
{
	class StaticMeshContainer;
	class StaticMeshManager;


	/**
	 * A model is an object in a 3d space, thus it consists of a mesh representing the shape
	 * and it consists also of a transformation matrix representing the orientation and
	 * position of the object in the 3d space.
	 */
	class Vob
	{
	public:
		explicit Vob(std::string meshName, MaterialType  materialType);

		explicit Vob(StaticMeshContainer* model);

		/**
		 * Calculates the transformatino matrix of this model
		 * based on its position, scale and rotation.
		 */
		void calcTrafo();

		/**
		* Provides the shader type of the mesh of this vob.
		*/
		MaterialType getMaterialType() const;


		/**
		 * Provides the name of the 3d mesh this model uses.
		 */
		std::string const& getMeshName() const;

		StaticMeshContainer* getModel();


		glm::vec3 getPosition() const;

		/**
		 * Provides read/write access to the transformation matrix
		 * of this model.
		 */
		const glm::mat4& getTrafo() const;


		void init();

		/**
		 * Sets the rotation measured in euler angles int XYZ order.
		 */
		void setEulerXYZ(glm::vec3 rotation);

		/**
		 * Sets the model of this vob.
		 */
		void setModel(StaticMeshContainer* model);

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
		StaticMeshContainer* m_model;
		MaterialType mMaterialType;
		glm::quat orientation;
		glm::vec3 position;
		glm::vec3 scale;
		glm::mat4 trafo;
	};	
}