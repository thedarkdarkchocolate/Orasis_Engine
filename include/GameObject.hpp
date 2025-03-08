#pragma once

#include "Model.hpp"


#include <memory>



namespace Orasis {

    struct Transform2dComponent 
    {
        glm::vec2 translation{};
        glm::vec2 scale{1.f};
        float rotation;

        glm::mat2 mat2()
        {
            const float s = glm::sin(rotation);    
            const float c = glm::cos(rotation);
            glm::mat2 rotation{ {c, s}, {-s, c} };

            return rotation * glm::mat2{ {scale.x, 0.f}, {0.f, scale.y} }; 
        }

    };
    
    
    class GameObject
    {

        public:

            using uint = unsigned int;
            
            uint id;
            std::shared_ptr<Model> model{};
            glm::vec3 color;
            Transform2dComponent transform2d;
        
        
        private:

            GameObject(uint _id) 
            : id{_id}
            {}


        public:
            
            GameObject(const GameObject&) = delete;
            GameObject &operator=(const GameObject&) = delete;
            
            GameObject(GameObject&&) = default; 
            GameObject &operator=(GameObject&&) = default;
            
            


            static GameObject createGameObject()
            {
                static uint currID = 0;
                return GameObject(currID++);
            }

            uint getID() const
            {
                return id;
            }
            


    };





}