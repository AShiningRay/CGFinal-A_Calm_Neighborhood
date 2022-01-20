#include "../deps/GLADLibs/include/glad/glad.h"
#include "../deps/glm/glm.hpp"
#include "../deps/glm/gtc/matrix_transform.hpp"
#include "shader_compiler.h"

#include <string>
#include <vector>

struct vertex_data
{
    glm::vec3 vert_pos;
    glm::vec3 vert_normal;
    glm::vec2 vert_texcoord;
    glm::vec3 vert_tangent;
    glm::vec3 vert_bitangent;
};

struct texture_data
{
    unsigned int texture_id;
    std::string  texture_type, texture_path;
};

class Mesh_data
{
    public:
        std::vector<vertex_data>     mesh_vertices;
        std::vector<unsigned int>    mesh_vert_indices;
        std::vector<texture_data>    mesh_textures;
        unsigned int VAO; // VAO = Vertex Array Object

        Mesh_data(std::vector<vertex_data> mesh_vertices, std::vector<unsigned int> mesh_vert_indices, std::vector<texture_data> mesh_textures)
        {
            this->mesh_vertices     = mesh_vertices;
            this->mesh_vert_indices = mesh_vert_indices;
            this->mesh_textures     = mesh_textures;

            configureMesh(); // Configures the mesh for rendering by setting its buffers(VBO, VAO, EBO as well as their data) and its attribute array and pointers.
        }

        void renderMesh(Shader &meshshader)
        {
            unsigned int diffusemapnum  = 1;
            unsigned int specularmapnum = 1;
            unsigned int normalmapnum   = 1;
            unsigned int heightmapnum   = 1;

            for(unsigned int i = 0; i < mesh_textures.size(); i++)
            {
                glActiveTexture(GL_TEXTURE0 + i);

                std::string texstruct = "material.", texnumber, texname = mesh_textures[i].texture_type;

                if(texname == "diffuse_texture")
                {
                    texnumber = std::to_string(diffusemapnum);
                    diffusemapnum++;
                }

                else if(texname == "specular_texture")
                {
                    texnumber = std::to_string(specularmapnum);
                    specularmapnum++;
                }

                else if(texname == "normal_texture")
                {
                    texnumber = std::to_string(normalmapnum);
                    normalmapnum++;
                }

                else if(texname == "height_texture")
                {
                    texnumber = std::to_string(heightmapnum);
                    heightmapnum++;
                }


                glUniform1i(glGetUniformLocation(meshshader.shader_id, (texstruct + texname + texnumber).c_str()), i);

                glBindTexture(GL_TEXTURE_2D, mesh_textures[i].texture_id);
            }

            glBindVertexArray(VAO); // sets the mesh's vertex array for drawing
            glDrawElements(GL_TRIANGLES, mesh_vert_indices.size(), GL_UNSIGNED_INT, 0); // draws the mesh
            glBindVertexArray(0); // Resets to the null vertex array after drawing the mesh

            glActiveTexture(GL_TEXTURE0); // Points back to the first texture sampler
        }

    private:
        unsigned int VBO, EBO; // Vertex Buffer Object and Element Buffer Object respectively.

        void configureMesh()
        {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glBufferData(GL_ARRAY_BUFFER, mesh_vertices.size() * sizeof(vertex_data), &mesh_vertices[0], GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_vert_indices.size() * sizeof(unsigned int), &mesh_vert_indices[0], GL_STATIC_DRAW);

            //Attribute pointers: 0-> vertex positions, 1-> vertex normals, 2-> vertex texture coordinates, 3-> vertex tangent angle, 4-> vertex bitangent angle.
            glEnableVertexAttribArray(0); // RETURN HERE IF IT GLITCHES!!!
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void*) 0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void*) sizeof(glm::vec3));

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void*) (2 * sizeof(glm::vec3)) );

            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void*) (2 * sizeof(glm::vec3) + sizeof(glm::vec2)) );

            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_data), (void*) (3 * sizeof(glm::vec3) + sizeof(glm::vec2)) );

            glBindVertexArray(0);
        }
};
