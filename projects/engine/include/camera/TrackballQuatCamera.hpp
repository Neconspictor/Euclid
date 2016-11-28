#ifndef CAMERA_TRACKBALL_QUAT_CAMERA_HPP
#define CAMERA_TRACKBALL_QUAT_CAMERA_HPP
#include <camera/Camera.hpp>
#include <glm/gtc/quaternion.hpp>

class Input;

class TrackballQuatCamera : public Camera
{
public:

	struct SphericalCoord
	{
		float polar;
		float azimuth;
		float radius;
	};

	TrackballQuatCamera(Window* window);
	TrackballQuatCamera(Window* window, glm::vec3 trackPosition, float radius, glm::vec3 up);
	TrackballQuatCamera(const TrackballQuatCamera& other);
	virtual ~TrackballQuatCamera();


	virtual void calcView() override;

	float getRadius() const;
	const glm::quat& getOrientation() const;
	const glm::vec3& getTrackPosition() const;


	virtual void setHalfScreenWidth(float width);

	virtual void setHalfScreenHeight(float height);

	void setRadius(float radius);

	void setTrackPosition(glm::vec3 trackPosition);

	/**
	* A trackball camera assumes the positive y axis as the up direction.
	* Therefore this function override does nothing change on this camera.
	*/
	virtual void setUpDirection(const glm::vec3& up) override;

	virtual void update(int mouseXFrameOffset, int mouseYFrameOffset) override;

	virtual void updateOnResize(int screenWidth, int screenHeight);



protected:
	float radius;
	glm::vec3 trackPosition;
	float prevMouseX, prevMouseY;
	int halfScreenWidth, halfScreenHeight;
	glm::quat orientation, prevOrientation;

	/**
	 * Checks if two provided vectors are nearly (ignoring rounding errors) identical.
	 */
	bool equal(const glm::vec3& v1, const glm::vec3& v2, float epsilon);

	/**
	 * Provides a rotation quaternion that represents a rotation from 
	 * vector 'from' to vector 'to'.
	 */
	glm::quat getRotation(const glm::vec3& from, const glm::vec3& to);

	glm::vec3 getUnitVector(float x, float y);

	glm::vec3 getVector(float x, float y);


	/*
	 * use the mouse distance from the centre of screen as arc length on the sphere
	 * x = R * sin(a) * cos(b)
	 * y = R * sin(a) * sin(b)
	 * z = R * cos(a)
	 * where a = angle on x-z plane, b = angle on x-y plane
	 *
	 * NOTE: the calculation of arc length is an estimation using linear distance
	 * from screen center (0,0) to the cursor position
	 */
	glm::vec3 getVectorWithArc(float x, float y);

	std::string toString(const glm::vec3& vec) const;
	std::string toString(const glm::quat& quaternion) const;
};

#endif