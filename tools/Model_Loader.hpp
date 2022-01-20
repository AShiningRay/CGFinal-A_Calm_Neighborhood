#include "../deps/assimp/Importer.hpp"
#include "../deps/assimp/scene.h"
#include "../deps/assimp/postprocess.h"
#include "../deps/stb_image/stb_image.h"
#include "Mesh_loader.hpp"
#include "shader_compiler.h"
#include <string>

class Model_data
{
    public:
        Model_data(std::string const &modelpath) // Change to char if it glitches
        {
            std::cout << "\nTrying to load Model located at:" << modelpath << std::endl;
            load(modelpath);
        }
        void renderModel(Shader &modelshader)
        {
            for(unsigned int i = 0; i < model_meshnum.size(); i++)
                model_meshnum[i].renderMesh(modelshader);
        }

    private:
        std::vector<Mesh_data> model_meshnum;
        std::vector<texture_data> texturesused;
        std::string modeldirectory;

        void load(std::string const &modelpath)
        {
            Assimp::Importer modelimporter;

            // Creates a scene containing the model specified in the modelpath, as well as triangulating it(most modeling softwares work with quads) and flipping its texture coordinates to work better with openGL image's y axis.
            const aiScene *modelscene = modelimporter.ReadFile(modelpath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);

            if(!modelscene || modelscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !modelscene->mRootNode)
            {
                std::cout << "FATAL_ERROR_WHILE_LOADING_THE_MODEL: " << modelimporter.GetErrorString() << std::endl;
                return;
            }
            else
            {
                modeldirectory = modelpath.substr(0, modelpath.find_last_of('/'));
                prepareSceneNodes(modelscene->mRootNode, modelscene);
            }
            std::cout << "Model Loaded.\n\n" << std::endl;
        }

        void prepareSceneNodes(aiNode *rootnode, const aiScene *scenenode)
        {
            for(unsigned int i = 0; i < rootnode->mNumMeshes; i++)
            { // Prepares the rootnode's mesh for rendering
                aiMesh *curmesh = scenenode->mMeshes[rootnode->mMeshes[i]];
                model_meshnum.push_back(prepareMeshNodes(curmesh, scenenode));
            }

            for(unsigned int i = 0; i < rootnode->mNumChildren; i++)
            { // Prepares the rootnode children's mesh for rendering
                prepareSceneNodes(rootnode->mChildren[i], scenenode);
            }
        }

        Mesh_data prepareMeshNodes(aiMesh *meshnode, const aiScene *scenenode)
        {
            std::vector<vertex_data > mesh_vertices;
            std::vector<unsigned int> mesh_vert_indices;
            std::vector<texture_data> mesh_textures;

            for(unsigned int i = 0; i < meshnode->mNumVertices; i++)
            {
                vertex_data newvert;
                glm::vec3 vertexcoord;

                vertexcoord.x = meshnode->mVertices[i].x;
                vertexcoord.y = meshnode->mVertices[i].y;
                vertexcoord.z = meshnode->mVertices[i].z;
                newvert.vert_pos = vertexcoord;

                if(meshnode->mTextureCoords[0])
                { // Checks if the loaded mesh has at least one texture coordinate.
                    glm::vec2 texcoordinate;

                    texcoordinate.x = meshnode->mTextureCoords[0][i].x;
                    texcoordinate.y = meshnode->mTextureCoords[0][i].y;

                    newvert.vert_texcoord = texcoordinate;

                    vertexcoord.x = meshnode->mTangents[i].x;
                    vertexcoord.y = meshnode->mTangents[i].y;
                    vertexcoord.z = meshnode->mTangents[i].z;

                    newvert.vert_tangent = vertexcoord;

                    vertexcoord.x = meshnode->mBitangents[i].x;
                    vertexcoord.y = meshnode->mBitangents[i].y;
                    vertexcoord.z = meshnode->mBitangents[i].z;

                    newvert.vert_bitangent = vertexcoord;
                }
                else
                    newvert.vert_texcoord = glm::vec2(0.0f, 0.0f);

                if(meshnode->HasNormals())
                {
                    vertexcoord.x = meshnode->mNormals[0].x;
                    vertexcoord.y = meshnode->mNormals[0].y;
                    vertexcoord.z = meshnode->mNormals[0].z;

                    newvert.vert_normal = vertexcoord;
                }

                mesh_vertices.push_back(newvert);
            }

            for(unsigned int i = 0; i < meshnode->mNumFaces; i++)
            {
                aiFace meshface = meshnode->mFaces[i];

                for(unsigned int j = 0; j < meshface.mNumIndices; j++)
                    mesh_vert_indices.push_back(meshface.mIndices[j]);
            }

            aiMaterial *meshmaterial = scenenode->mMaterials[meshnode->mMaterialIndex];

            std::vector<texture_data> texdiffusemap = loadModelMaterialTextures(meshmaterial, aiTextureType_DIFFUSE, "diffuse_texture");
            mesh_textures.insert(mesh_textures.end(), texdiffusemap.begin(), texdiffusemap.end() );

            std::vector<texture_data> texspecularmap = loadModelMaterialTextures(meshmaterial, aiTextureType_SPECULAR, "specular_texture");
            mesh_textures.insert(mesh_textures.end(), texspecularmap.begin(), texspecularmap.end() );

            std::vector<texture_data> texnormalmap = loadModelMaterialTextures(meshmaterial, aiTextureType_NORMALS, "normal_texture");
            mesh_textures.insert(mesh_textures.end(), texnormalmap.begin(), texnormalmap.end() );

            std::vector<texture_data> texheightmap = loadModelMaterialTextures(meshmaterial, aiTextureType_HEIGHT, "height_texture");
            mesh_textures.insert(mesh_textures.end(), texheightmap.begin(), texheightmap.end() );

            return Mesh_data(mesh_vertices, mesh_vert_indices, mesh_textures);
        }

        std::vector<texture_data> loadModelMaterialTextures(aiMaterial *material, aiTextureType textype, std::string textypename)
        {
            std::vector<texture_data> mesh_textures;
            bool loadtexture = true;


            for(unsigned int i = 0; i < material->GetTextureCount(textype); i++)
            {
                aiString texturepath;
                material->GetTexture(textype, i, &texturepath);
                loadtexture = true; // Sets the texture to be loaded by default

                for(unsigned int j = 0; j < texturesused.size(); j++)
                { // Loops through all textures currently in use by the mesh in search of a identical texture to prevent the loading of duplicate textures.
                    if(std::strcmp(texturesused[j].texture_path.data(), texturepath.C_Str()) == 0)
                    { // If any repeated texture(by location and name) is found, do not load the texture again.
                        mesh_textures.push_back(texturesused[j]);
                        loadtexture = false;
                        break;
                    }
                }

                if(loadtexture)
                {
                    texture_data texdata;
                    texdata.texture_id = loadTexture(texturepath.C_Str(), this->modeldirectory);
                    texdata.texture_type = textypename;
                    texdata.texture_path = texturepath.C_Str();
                    mesh_textures.push_back(texdata);
                    texturesused.push_back(texdata);
                }
            }

            return mesh_textures;
        }

        unsigned int loadTexture(const char *modelpath, const std::string &texdirectory)
        {
            std::string texturefilepath = std::string(modelpath);
            texturefilepath = texdirectory + "/" + texturefilepath;

            std::cout << "Trying to load texture located at:" << texturefilepath.c_str() << std::endl;

            int texwidth, texheight, texchannels;
            unsigned char *texturedata = stbi_load(texturefilepath.c_str(), &texwidth, &texheight, &texchannels, 0);
            unsigned int texture_id;

            if(texturedata)
            {
                GLenum textureformat;
                if (texchannels == 1)
                    textureformat = GL_RED;

                else if(texchannels == 3)
                    textureformat = GL_RGB;

                else if(texchannels == 4)
                    textureformat = GL_RGBA;

                glGenTextures(1, &texture_id);
                glBindTexture(GL_TEXTURE_2D, texture_id);
                glTexImage2D(GL_TEXTURE_2D, 0, textureformat, texwidth, texheight, 0, textureformat, GL_UNSIGNED_BYTE, texturedata);
                glGenerateMipmap(GL_TEXTURE_2D);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                std::cout << "Texture Loaded" << std::endl;
            }
            else
            {
                std::cout << "Failed to load the texture located at: " << modelpath << std::endl;
            }

            stbi_image_free(texturedata);
            return texture_id;
        }

};
