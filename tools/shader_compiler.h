#ifndef SHADER_H
#define SHADER_H

#include "../deps/GLADlibs/include/glad/glad.h"
#include "../deps/glm/glm.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shader
{
    public:
        unsigned int shader_id = 0;

        Shader(const char* vertexShaderFilePath, const char* fragmentShaderFilePath, const char* geometryShaderFilePath)
        {
            std::string     geometryShaderFileContent;
            std::string     vertexShaderFileContent;
            std::string     fragmentShaderFileContent;
            std::ifstream   geometryShaderFile;
            std::ifstream   vertexShaderFile;
            std::ifstream   fragmentShaderFile;


            vertexShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            fragmentShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            geometryShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
            try
            {
                vertexShaderFile.open(vertexShaderFilePath);
                fragmentShaderFile.open(fragmentShaderFilePath);

                std::stringstream vertexShaderStream, fragmentShaderStream;
                vertexShaderStream  << vertexShaderFile.rdbuf();
                fragmentShaderStream << fragmentShaderFile.rdbuf();

                vertexShaderFile.close();
                fragmentShaderFile.close();

                if(geometryShaderFilePath != nullptr)
                {
                    geometryShaderFile.open(geometryShaderFilePath);
                    std::stringstream geometryShaderStream;
                    geometryShaderStream << geometryShaderFile.rdbuf();
                    geometryShaderFile.close();
                    geometryShaderFileContent = geometryShaderStream.str();
                }

                vertexShaderFileContent     = vertexShaderStream.str();
                fragmentShaderFileContent   = fragmentShaderStream.str();
            }
            catch(std::ifstream::failure& e)
            {
                std::cout << "ERROR::SHADER_FILE_COULD_NOT_BE_READ" << std::endl;
            }

            const char* vertexShaderCode    = vertexShaderFileContent.c_str();
            const char* fragmentShaderCode  = fragmentShaderFileContent.c_str();

            unsigned int vertexShader, fragmentShader, geometryShader;
            int compiling_succeeded;
            char compileLog[512];

            if(geometryShaderFilePath != nullptr)
            {
                const char *geometryShaderCode = geometryShaderFileContent.c_str();
                geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(geometryShader, 1, &geometryShaderCode, NULL);
                glCompileShader(geometryShader);
                if(!compiling_succeeded)
                {
                    glGetShaderInfoLog(geometryShader, 512, NULL, compileLog);
                    std::cout << "ERROR::GEOMETRY_SHADER_COMPILATION_FAILED\n" << compileLog <<std::endl;
                }
            }

            //Vertex Shader compilation block
            vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
            glCompileShader(vertexShader);

            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiling_succeeded);
            if(!compiling_succeeded)
            {
             glGetShaderInfoLog(vertexShader, 512, NULL, compileLog);
             std::cout << "ERROR::VERTEX_SHADER_COMPILATION_FAILED\n" << compileLog <<std::endl;
            };

            //Fragment Shader compilation block
            fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
            glCompileShader(fragmentShader);

            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiling_succeeded);
            if(!compiling_succeeded)
            {
             glGetShaderInfoLog(fragmentShader, 512, NULL, compileLog);
             std::cout << "ERROR::FRAGMENT_SHADER_COMPILATION_FAILED\n" << compileLog <<std::endl;
            };


            // Shader ID creation and linking block
            shader_id = glCreateProgram();
            glAttachShader(shader_id, vertexShader);
            glAttachShader(shader_id, fragmentShader);
            if(geometryShaderFilePath != nullptr)
                glAttachShader(shader_id, geometryShader);
            glLinkProgram(shader_id);

            glGetProgramiv(shader_id, GL_LINK_STATUS, &compiling_succeeded);
            if(!compiling_succeeded)
            {
                glGetProgramInfoLog(shader_id, 512, NULL, compileLog);
                std::cout << "ERROR::SHADER_PROGRAM_LINKING_FAILED\n" << compileLog << std::endl;
            };

            // Shader cleanup block
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            if(geometryShaderFilePath != nullptr)
                glDeleteShader(geometryShader);
        }

        void useShader()
        {
            glUseProgram(shader_id);
        }

        void setBool(const std::string &varname, bool varvalue) const
        {
            glUniform1i(glGetUniformLocation(shader_id, varname.c_str()), (int) varvalue);
        }

        void setInt(const std::string &varname, int varvalue) const
        {
            glUniform1i(glGetUniformLocation(shader_id, varname.c_str()), varvalue);
        }

        void setFloat(const std::string &varname, float varvalue) const
        {
            glUniform1f(glGetUniformLocation(shader_id, varname.c_str()), varvalue);
        }

        void setVec2vect(const std::string &varname, const glm::vec2 &vecvalue) const
        {
            glUniform2fv(glGetUniformLocation(shader_id, varname.c_str()), 1, &vecvalue[0]);
        }

        void setVec2(const std::string &varname, float x, float y) const
        {
            glUniform2f(glGetUniformLocation(shader_id, varname.c_str()), x, y);
        }

        void setVec3vect(const std::string &varname, const glm::vec3 &vecvalue) const
        {
            glUniform3fv(glGetUniformLocation(shader_id, varname.c_str()), 1, &vecvalue[0]);
        }
        void setVec3(const std::string &varname, float x, float y, float z) const
        {
            glUniform3f(glGetUniformLocation(shader_id, varname.c_str()), x, y, z);
        }

        void setVec4vect(const std::string &varname, const glm::vec4 &vecvalue) const
        {
            glUniform4fv(glGetUniformLocation(shader_id, varname.c_str()), 1, &vecvalue[0]);
        }
        void setVec4(const std::string &varname, float x, float y, float z, float w)
        {
            glUniform4f(glGetUniformLocation(shader_id, varname.c_str()), x, y, z, w);
        }

        void setMat2(const std::string &varname, const glm::mat2 &matrix) const
        {
            glUniformMatrix2fv(glGetUniformLocation(shader_id, varname.c_str()), 1, GL_FALSE, &matrix[0][0]);
        }

        void setMat3(const std::string &varname, const glm::mat3 &matrix) const
        {
            glUniformMatrix3fv(glGetUniformLocation(shader_id, varname.c_str()), 1, GL_FALSE, &matrix[0][0]);
        }

        void setMat4(const std::string &varname, const glm::mat4 &matrix) const
        {
            glUniformMatrix4fv(glGetUniformLocation(shader_id, varname.c_str()), 1, GL_FALSE, &matrix[0][0]);
        }

    private:
        void checkErrors(GLuint shader_id, std::string shadertype)
        {
            GLint compiling_succeeded;
            GLchar compileLog[1024];
            if(shadertype != "PROGRAM")
            {
                glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiling_succeeded);
                if(!compiling_succeeded)
                {
                    glGetShaderInfoLog(shader_id, 1024, NULL, compileLog);
                    std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << shadertype << "\n" << compileLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            }
            else
            {
                glGetProgramiv(shader_id, GL_LINK_STATUS, &compiling_succeeded);
                if(!compiling_succeeded)
                {
                    glGetProgramInfoLog(shader_id, 1024, NULL, compileLog);
                    std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << shadertype << "\n" << compileLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            }

        }

};

#endif
