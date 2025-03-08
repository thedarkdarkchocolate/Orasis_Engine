#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Window.hpp"
#include "Render.hpp"
#include "GameObject.hpp"
#include "RenderSystem.hpp"

#include <memory>
#include <vector>
#include <stdexcept>
#include <array>

namespace Orasis {


    class App {

        // -------- MEMBER VARIABLES -------- //

        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        Window ors_Window{WIDTH, HEIGHT, "AAApp!!"};
        Device ors_Device{ors_Window};
        Render ors_Render{ors_Window, ors_Device};

        std::vector<GameObject> gameObjects;
        
        // -------- -------- -------- -------- //


        public:

        // -------- CONSTRUCTOR etc -------- //

        App()
        {
            loadGameObjects();
        }

        ~App()
        {}

        App(const App&) = delete;
        App &operator=(const App&) = delete;

        // -------- -------- -------- -------- //





        // -------- FUNCTIONS -------- //

        void run() {

            RenderSystem renderSys{ors_Device, ors_Render.getSwapChainRenderPass()};    

            while(!ors_Window.shouldClose())
            {
                glfwPollEvents();
                
                if (VkCommandBuffer cmndBuffer = ors_Render.beginFrame())
                {
                    ors_Render.startSwapChainRenderPass(cmndBuffer);
                    renderSys.renderGameObjects(cmndBuffer, gameObjects);
                    ors_Render.endSwapChainRenderPass(cmndBuffer);
                    ors_Render.endFrame();

                }
                
            }

            vkDeviceWaitIdle(ors_Device.device());

        }

        private:

        void loadGameObjects()
        {
            std::vector<Model::Vertex> vertices
            {
                {{0,   -1}, {1.0f, 0.0f, 0.0f}},    
                {{1,   1}, {0.0f, 1.0f, 0.0f}},
                {{-1,   1}, {0.0f, 0.0f, 1.0f}}

                
            };  

            // vertices = {};  
            // glm::mat3x2 startVert = {0.f,   -1.f, -1.f,   1.f, 1.f,   1.f};
            // verticesForTringleInATringle(8, startVert, vertices);

            std::shared_ptr<Model> ors_Model = std::make_unique<Model>(ors_Device, vertices);

            GameObject triangle = GameObject::createGameObject();
            triangle.model = ors_Model;
            triangle.color = {.1f, .8f, .1f};
            triangle.transform2d.translation.x = .2f;
            triangle.transform2d.scale = {0.5, 0.5};
            triangle.transform2d.rotation = glm::pi<float>();
            
            gameObjects.push_back(std::move(triangle));

        }

        void verticesForTringleInATringle(int depth, glm::mat3x2 startVert, std::vector<Model::Vertex>& out)
        {
            
            if (depth-- <= 0) return;
            
            // for(int i = 0; i < 3; i++)
            // {
            //     glm::vec2 top = startVert[0];
            //     glm::vec2 left = (startVert[0] + startVert[1]) / 2.f;
            //     glm::vec2 right = (startVert[0] + startVert[2]) / 2.f;
                
                
            //     out.push_back({{top.x, top.y}});
            //     out.push_back({{left.x, left.y}});
            //     out.push_back({{right.x, right.y}});
                
            //     verticesForTringleInATringle(depth, {top, left, right}, out);

            // }

            glm::vec2 top = startVert[0];
            glm::vec2 left = (startVert[0] + startVert[1]) / 2.f;
            glm::vec2 right = (startVert[0] + startVert[2]) / 2.f;
            
            if (!depth){
            out.push_back({{top.x, top.y}});
            out.push_back({{left.x, left.y}});
            out.push_back({{right.x, right.y}});
            }
            verticesForTringleInATringle(depth, {top, left, right}, out);

            top = (startVert[0] + startVert[1]) / 2.f;;
            left = startVert[1];
            right = (startVert[1] + startVert[2]) / 2.f;

            if (!depth){
            out.push_back({{top.x, top.y}});
            out.push_back({{left.x, left.y}});
            out.push_back({{right.x, right.y}});
            }
            verticesForTringleInATringle(depth, {top, left, right}, out);

            top = (startVert[0] + startVert[2]) / 2.f;
            left = (startVert[1] + startVert[2]) / 2.f;
            right = (startVert[2]);

            if (!depth){
            out.push_back({{top.x, top.y}});
            out.push_back({{left.x, left.y}});
            out.push_back({{right.x, right.y}});
             }   
            verticesForTringleInATringle(depth, {top, left, right}, out);


        }



    };


}