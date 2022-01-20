#include "../deps/GLADlibs/include/glad/glad.h"
#include "../deps/glm/glm.hpp"
#include "../deps/glm/gtc/matrix_transform.hpp"

enum Movement_Directions
{
    MOVE_FORWARD,
    MOVE_BACKWARDS,
    MOVE_LEFT,
    MOVE_RIGHT
};

const float CAMERAYAW       = -90.0f;
const float CAMERAPITCH     = 0.0f;
const float CAMERAMOVESPEED = 3.8f;
const float CAMERALOOKSPEED = 0.1f;
const float CAMERAZOOM      = 75.0f;

class Camera_Object
{
    public:
        glm::vec3 position;
        glm::vec3 viewtarget;
        glm::vec3 directionvector;
        glm::vec3 upaxis; // +Y will basically be the World's Up Axis
        glm::vec3 right;
        glm::vec3 up;
        glm::vec3 front;

        float yaw;
        float pitch;

        float movespeed;
        float lookspeed;
        float zoom;

        Camera_Object(glm::vec3 camerapos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 worldup = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = CAMERAYAW, float pitch = CAMERAPITCH )
        : front(glm::vec3(0.0f, 0.0f, -1.0f)), movespeed(CAMERAMOVESPEED), lookspeed(CAMERALOOKSPEED), zoom(CAMERAZOOM)
        {
            position = camerapos;
            upaxis = worldup;
            yaw = yaw;
            pitch = pitch;
            updateDirectionVectors();
        }

        Camera_Object(float cameraposX, float cameraposY, float cameraposZ, float upX, float upY, float upZ, float camerayaw, float camerapitch)
        : front(glm::vec3(0.0f, 0.0f, -1.0f)), movespeed(CAMERAMOVESPEED), lookspeed(CAMERALOOKSPEED), zoom(CAMERAZOOM)
        {
            position = glm::vec3(cameraposX, cameraposY, cameraposZ);
            upaxis = glm::vec3(upX, upY, upZ);
            yaw = camerayaw;
            pitch = camerapitch;
            updateDirectionVectors();
        }

        glm::mat4 getViewMatrix()
        {
            return glm::lookAt(position, position + front, up);
        }

        void checkKeyboardPresses(Movement_Directions movedirection, bool running, bool crouching, float timedelta)
        {
            float movementvelocity = movespeed * timedelta;
            if(running)
                movementvelocity *= 2.7;
            else if(crouching)
                movementvelocity *= 0.4;

            if (movedirection == MOVE_FORWARD)
                position += front * movementvelocity;

            else if(movedirection == MOVE_BACKWARDS)
                position -= front * movementvelocity;

            if(movedirection == MOVE_LEFT)
                position -= right * movementvelocity;

            else if(movedirection == MOVE_RIGHT)
                position += right * movementvelocity;

        }

        void checkMouseMovement(double xmovement, double ymovement, GLboolean preventflip = true)
        {
            xmovement *= lookspeed;
            ymovement *= lookspeed;

            yaw     += xmovement;
            pitch   += ymovement;

            if(preventflip)
            {
                if (pitch > 85.0f)
                    pitch = 85.0f;
                if (pitch < -85.0f)
                    pitch = -85.0f;
            }

            updateDirectionVectors();
        }

        void checkMouseWheel(float zoomoffset)
        {
            zoom -= zoomoffset;
            if(zoom < 45.0f)
                zoom = 45.0f;
            else if(zoom > 75.0f)
                zoom = 75.0f;
        }

    private:

        void updateDirectionVectors()
        {
            glm::vec3 camerafront;
            camerafront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            camerafront.y = sin(glm::radians(pitch));
            camerafront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            front         = glm::normalize(camerafront);
            // Normalizes the right and up vectors, in order to reduce the amount of movement when the camera angle gets closer to oblique angles
            right         = glm::normalize(glm::cross(front, upaxis));
            up            = glm::normalize(glm::cross(right, front));
        }

};
