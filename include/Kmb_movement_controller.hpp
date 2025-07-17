#pragma once 

#include "GameObject.hpp"
#include "Window.hpp"

namespace Orasis
{

    class KmbMovementController {


        public:

            struct KeyMappings {

                int moveLeft = GLFW_KEY_A;
                int moveRight = GLFW_KEY_D;
                int moveForward = GLFW_KEY_W;
                int moveBackward = GLFW_KEY_S;
                int moveUp = GLFW_KEY_SPACE;
                int moveDown = GLFW_KEY_LEFT_CONTROL;
                int lookLeft = GLFW_KEY_LEFT;
                int lookRight = GLFW_KEY_RIGHT;
                int lookUp = GLFW_KEY_UP;
                int lookDown = GLFW_KEY_DOWN;
                int esc = GLFW_KEY_ESCAPE;

            };

            KeyMappings keys{};
            float moveSpeed{3.f};
            float lookSpeed{1.5f};



            void moveInPlaneXZ(GLFWwindow* window, GameObject& gameObject, float dt)
            {
                
                if (glfwGetKey(window, keys.esc) == GLFW_PRESS) 
                    glfwSetWindowShouldClose(window, GL_TRUE);
                
                glm::vec3 rotate{0};

                if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1;
                if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1;
                if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1;
                if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1;
                
                if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
                    gameObject.transform.rotation += glm::normalize(rotate) * lookSpeed * dt;
                    
                gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
                gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
                
                glm::vec3 moveDir{0.f};

                float yaw = gameObject.transform.rotation.y;
                const glm::vec3 forwardDir{sin(yaw), 0.f, cos(yaw)};
                const glm::vec3 rightDir{forwardDir.z, 0.f, -forwardDir.x};
                const glm::vec3 upDir{0.f, -1.f, 0.f};
                
                if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
                if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
                if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
                if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
                if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
                if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;
                    
                if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
                    gameObject.transform.translation += glm::normalize(moveDir) * moveSpeed * dt;
            }

        
        
    };


}