#ifndef CELL_CAMERA_CAMERA_H
#define CELL_CAMERA_CAMERA_H

#include "camera_frustum.h"
#include <glm/glm.hpp>

namespace nex
{
    // defines several possible options for camera movement. Used as abstraction to stay away from 
    // window-system specific input methods.
    enum CAMERA_MOVEMENT {
        CAMERA_FORWARD,
        CAMERA_BACK,
        CAMERA_LEFT,
        CAMERA_RIGHT,
        CAMERA_UP,
        CAMERA_DOWN,
    };

    /* 

      Basic root camera. Only does relevant camera calculations with manual forced direction 
      setters. This camera should only be used in code and not respond to user input; the
      derived cameras are for user/player interaction.

    */
    // TODO(Joey): do we want the camera to be a SceneNode as well so we can attach it to other entities?
    // what about FPS cameras then? 
    class CellCamera
    {
    public:
		glm::mat4 Projection;
		glm::mat4 View;

		glm::vec3 Position = glm::vec3(0.0f, 0.0f,  0.0f);
		glm::vec3 Forward  = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 Up       = glm::vec3(0.0f, 1.0f,  0.0f);
		glm::vec3 Right    = glm::vec3(1.0f, 0.0f,  0.0f);

        float FOV;
        float Aspect;
        float Near;
        float Far;
        bool  Perspective;

        CameraFrustum Frustum;
    private:
    public:
		CellCamera();
		~CellCamera();
		CellCamera(glm::vec3 position, glm::vec3 forward, glm::vec3 up);

        void Update(float dt);
        
        void SetPerspective(float fov, float aspect, float near, float far);
        void SetOrthographic(float left, float right, float top, float bottom, float near, float far);

        void UpdateView();

        float FrustumHeightAtDistance(float distance);
        float DistanceAtFrustumHeight(float frustumHeight);

    };
}
#endif