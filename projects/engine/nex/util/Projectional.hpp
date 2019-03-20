#pragma once

#include <glm/glm.hpp>
#include "nex/common/Log.hpp"

namespace nex
{

	struct Frustum
	{
		float left;
		float right;
		float bottom;
		float top;
		float nearPlane;
		float farPlane;
	};

	struct FrustumPlane
	{
		glm::vec3 leftBottom;
		glm::vec3 leftTop;
		glm::vec3 rightBottom;
		glm::vec3 rightTop;
	};

	struct FrustumCuboid
	{
		FrustumPlane m_near;
		FrustumPlane m_far;
	};

	enum ProjectionMode
	{
		Orthographic,
		Perspective
	};

	class Projectional
	{
	public:

		explicit Projectional(float aspectRatio = 16.0f / 9.0f,
			float fov = 45.0f,
			float perspNear = 0.1f,
			float perspFar = 100.0f,
			Frustum frustum = { -10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 30.0f },
			//Frustum frustum = Frustum(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 150.0f),
			glm::vec3 look = { 0,0,-1 },
			glm::vec3 position = { 0,0,0 },
			glm::vec3 up = { 0,1,0 }
		);

		virtual ~Projectional() = default;

		virtual void calcView();
		float getAspectRatio() const;
		const glm::vec3& getLook() const;
		float getFOV() const;
		const Frustum& getFrustum(ProjectionMode mode);
		FrustumCuboid getFrustumCuboid(ProjectionMode mode, float zStart = 0.0f, float zEnd = 1.0f);
		FrustumPlane getFrustumPlane(ProjectionMode mode, float zValue);
		const glm::mat4& getOrthoProjection();
		const glm::mat4& getPerspProjection();
		const glm::mat4& getProjection(ProjectionMode mode);
		const glm::vec3& getPosition() const;
		const glm::vec3& getRight() const;
		const glm::vec3& getUp() const;
		const glm::mat4& getView();

		/**
		 * Calculate viewspace z from a plane distance (which is always positive!)
		 */
		float getViewSpaceZfromPlaneDistance(float distance);
		glm::vec2 getNearFarPlaneViewSpace(ProjectionMode mode);

		void lookAt(glm::vec3 location);

		/**
		 * NOTE: ratio as to be greater 0, otherwise a runtime_error will be thrown!
		 */
		void setAspectRatio(float ratio);

		void setFOV(float fov);

		void setOrthoFrustum(Frustum frustum);

		virtual void setNearPlane(float nearPlane);

		virtual void setFarPlane(float farPlane);

		/**
		* NOTE: Has to be a vector that isn't a null vector. So it's length has to be > 0
		*/
		virtual void setLook(glm::vec3 look);

		virtual void setPosition(glm::vec3 position);

		virtual void setUp(glm::vec3 up);

		virtual void update(bool updateAlways = false);

	protected:
		float aspectRatio;
		float fov;
		nex::Logger m_logger;
		glm::vec3 look;
		Frustum orthoFrustum;
		glm::mat4 orthographic;
		glm::vec3 position;
		glm::mat4 perspective;
		Frustum perspFrustum;
		bool revalidate;
		glm::vec3 up;
		glm::mat4 view;
		glm::vec3 right;

		void calcPerspFrustum();
	};
}