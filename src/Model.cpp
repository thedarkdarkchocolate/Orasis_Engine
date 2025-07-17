#include "Model.hpp"

#include "Pipeline.hpp"
#include "Utils.hpp"


#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>


namespace std {

    template<>
    struct std::hash<Orasis::Model::Vertex> {

        size_t operator()(Orasis::Model::Vertex const& vertex) const{
            size_t seed = 0;
            Orasis::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }    

    };

}

void Orasis::Model::Builder::loadModel(const std::string& filepath)
{
    tinyobj::attrib_t attrib;                       // attrib stores position, color, normal, texture coord data
    std::vector<tinyobj::shape_t> shapes;           // shapes stores index values 
    std::vector<tinyobj::material_t> materials;     
    std::string warn, err;

    if ( !tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()) )
        throw std::runtime_error(warn + err);
        
    
    vertices.clear();
    indices.clear();

    // Creating an unorder map to keep track of duplicate vertices and to populate the indices array
    std::unordered_map<Vertex, uint32_t> uniqueMap{};

    for (const auto& shape: shapes)
        for (const auto& index: shape.mesh.indices)
        {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                vertex.position = { 
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };
                
                vertex.color = { 
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };
                
            }


            if (index.normal_index >= 0)
                vertex.normal = { 
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };

            if (index.vertex_index >= 0)
                vertex.uv = { 
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1],
                };
            

            if (uniqueMap.count(vertex) == 0) {
                uniqueMap[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(uniqueMap[vertex]);                                
        }

    printf("Vertex count: %zd \n", vertices.size());

}       

  