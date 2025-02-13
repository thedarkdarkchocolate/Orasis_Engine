#define GLFW_INCLUDE_VULKAN

#include <iostream>
#include "App.hpp"
#include <stdexcept>


int main () 
{

    Orasis::App app;

    try{
        app.run();
    }
    catch(std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}