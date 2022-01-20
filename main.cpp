/*
# **********Calm Neighborhood - A Simple Computer Graphics Project*********
#
# Author: Paulo César de Morais Sousa / Github Alias -> @AShiningRay
#
# Project Explanation: This was a simple Computer Graphics project that worked as
# the course's final practical exam, where it was required that a project
# contained at least the principles of OpenGL's graphics rendering pipeline,
# which are comprised of: Lighting (in its static form, shadows not required),
#                         Shading (gouraud or phong, implemented in shaders)
#                         Texturing
#                         Camera (to look around the environment)
# I decided to go the extra mile and also implemented multiple light types,
# moving lights, a skybox, a ton of static lights, vertex shaders that allow
# the renderer to manipulate a gigantic amount of grass blades and tree leaves
# without tanking performance and also implemented a lighting model closer to PBR
# through shaders that support specular, height, emission and normal maps besides
# the default diffuse texture. (maybe a few other things, but i can't remember).
#
# I'm sure a lot of optimizations could be done to make it render much faster,
# but this project was a sole effort and had around one month of development time
# so compromises had to be made.
*/

#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include "deps/GLADlibs/include/glad/glad.h"
#include "deps/GLFW3/include/glfw3.h"
#include "deps/glm/glm.hpp"
#include "deps/glm/gtc/matrix_transform.hpp"
#include "deps/glm/gtc/type_ptr.hpp"

#include "tools/camera_object.h"
#include "tools/Model_Loader.hpp"


void inputPolling(GLFWwindow *window);
void mousePolling(GLFWwindow *window, double xposition, double yposition);
void mouseWheelPolling(GLFWwindow* window, double xoffset, double yoffset);
void resizewin(GLFWwindow* window, int width, int height);
const unsigned int windowwidth = 1280, windowheight = 720;

float framedeltatime = 0.0f;
float lastframerendered = 0.0f;
float currentframetime = 0.0f;

Camera_Object cam(glm::vec3(-8.0f, 2.5f, -5.0f));

bool firstpolling = true;
float mouselastxposition = windowwidth/2.0f, mouselastyposition = windowheight/2.0f; // windowwidth/2, windowheight/2, basically.


int main ()
{
    //GLFW Window and Viewport Properties Definition.
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_DECORATED, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 0); // Sets MSAA to 0X, 2X, 4X or 8X, providing some sort of anti-aliasing in order to smooth jagged edges.


    GLFWwindow *lightingWindow = glfwCreateWindow(windowwidth, windowheight, "OpenGL4.3: CG-Final", NULL, NULL);
    glfwSetWindowAspectRatio(lightingWindow, 16, 9);
    //glfwSetWindowPos(lightingWindow, 1920 - windowwidth, 1080 - windowheight); // Centers the window, but only in 720P on a 1080P display
    glfwSetInputMode(lightingWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Disables the cursor in order to use the mouse on the camera without obstructions
    glfwSetCursorPosCallback(lightingWindow, mousePolling);
    glfwSetScrollCallback(lightingWindow, mouseWheelPolling);
    glfwSetFramebufferSizeCallback(lightingWindow, resizewin);

    if(lightingWindow == NULL)
    {
        std::cout << "Failed to create the main window\n" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(lightingWindow);


    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to load GLAD GL function Loader\n" << std::endl;
        glfwTerminate();
        return -2;
    }

    glViewport(0, 0, windowwidth, windowheight);
    glEnable(GL_DEPTH_TEST); // Face culling
    glEnable(GL_MULTISAMPLE); // Enables MSAA, in its primitive form
    glEnable(GL_BLEND); // Enables partial transparency


    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // This one basically allows a opaque object to contribute a certain percent to the final fragment's color
    /*Here's how it works:
        If we have 2 objects, one of them is white and has 65% opacity(alpha = 0.6) and the other is black with 100% opacity(alpha = 1.0), then the calculus is as follows

        resultingfragment = (1.0, 1.0, 1.0, 0.65) + (0.0, 0.0, 0.0, (1.0 - 0.65))

        So the resulting fragment on the screen will have 65% of its color contributed by the partially transparent white object, and 35% contributed by the black opaque
        object, as long as the transparent object is in front of the completely opaque object.

        This however is bound to have depth issues, when an object closer to the camera occludes another object further away, despite both being partially transparent.
        It can be "solved" by rendering the farthest objects first and then the closest ones, guaranteeing that all objects will be analyzed by the depth buffer.
    */

    glfwSwapInterval(1); // Enables Vsync.

    std::cout << "Vendor:" << glGetString(GL_VENDOR) << "\nRenderer:" << glGetString(GL_RENDERER) << "\nOpenGL version in use:" << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading language version in use:" << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    //Textures load setup.
    //stbi_set_flip_vertically_on_load(true); Only needed if importing textures with flipped Y coordinates


    //Setup of multiple objects in the scene

    // Base matrices(model, view and projection) setup
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);

    projectionMatrix = glm::perspective(glm::radians(cam.zoom), (float) windowwidth / (float) windowheight, 0.1f, 5000.0f);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    //Object Shader creation(external header)
    Shader vegetationshader("shaders/VegetationVertexShader.vert", "shaders/VegetationFragmentShader.frag", nullptr);
    Shader coloredlightshader("shaders/PointLightSourceVertexShader.vert", "shaders/PointLightSourceFragmentShader.frag", nullptr);
    Shader basicshader("shaders/BasicVertexShader.vert", "shaders/BasicFragmentShader.frag", nullptr);


    // Model loading procedures.
    Model_data terrain("models/Terrain/Terrain.obj");
    Model_data grass_1("models/Terrain/Grass 1.obj");
    Model_data grass_2("models/Terrain/Grass 2.obj");
    Model_data grass_small("models/Terrain/Grass Small.obj");
    Model_data shrubs("models/Terrain/Shrubs.obj");
    Model_data moon("models/Terrain/Moon.obj");
    Model_data skybox("models/Terrain/Skybox.obj");

    Model_data big_tree("models/Big Tree/Big Tree.obj");
    Model_data tree_leaves("models/Big Tree/Big Tree Leaves.obj");
    Model_data maple_tree("models/Maple Tree/Maple Tree.obj");
    Model_data maple_tree_leaves("models/Maple Tree/Maple Leaves.obj");
    Model_data plant_holder("models/Smaller Objects/Plant Holder.obj");
    Model_data ext_build_5("models/Buildings/Ext Build 5.obj");
    Model_data ext_build_4("models/Buildings/Ext Build 4.obj");
    Model_data ext_build_1("models/Buildings/Ext Build 1.obj");
    Model_data ext_build_3("models/Buildings/Ext Build 3.obj");
    Model_data ext_build_2("models/Buildings/Ext Build 2.obj");

    Model_data lamp_post("models/Smaller Objects/Lamp Post.obj");
    Model_data stop_sign("models/Smaller Objects/Stop Sign.obj");


    Model_data lightcube("models/fireflies/lightcube.obj");

    // Model Translation procedures.
    // When converting blender's coordinate system to the engine's, remember to swap blender's Z and Y axis(blender treats Z as world up, while the engine treats Y as world up)
    // Also, invert the Z values. If it is something like 12.04f, make it -12.04f. for reference: glm::vec3(X, Y, Z).
    glm::vec3 tree_and_leaves_translation[2] =
    {
        glm::vec3(-12.69f, 3.17f, -65.35f), // Tree
        glm::vec3(-15.73f, 14.68f, -66.08f) // Leaves
    };
    glm::vec3 grass_types_translation[3] =
    {
        glm::vec3(-23.35f, 0.65f, -40.02f), // Grass 1(purple)
        glm::vec3(-11.96f, 1.50f, -52.90f), // Grass 1(Orange)
        glm::vec3(-11.30f, 0.83f, -47.41f)  // Small Grass(darker orange)
    };
    glm::vec3 shrubs_translation = glm::vec3(-69.51f, 0.90f, -0.74f);
    glm::vec3 mapletree_and_leaves_translation[4] =
    {
        glm::vec3(14.75f, 0.57f, 38.78f),
        glm::vec3(14.75f, 0.57f, 48.48f),
        glm::vec3(14.75f, 0.57f, 58.86f),
        glm::vec3(14.75f, 0.57f, 69.91f)
    };

    glm::vec3 moon_translation = glm::vec3(0.0f, 750.0f, -1500.0f);
    glm::vec3 plant_holder_translation = glm::vec3(14.84f, 0.0f, 55.55f);


    glm::vec3 lamp_posts_translation[42] =
    {
        glm::vec3(0.0f, 0.19f, -20.62f),
        glm::vec3(10.0f, 0.19f, -30.12f),
        glm::vec3(20.0f, 0.19f, -20.62f),
        glm::vec3(26.5f, 0.19f, -15.62f),
        glm::vec3(26.5f, 0.19f, -35.12f),
        glm::vec3(39.0f, 0.19f, -45.12f),
        glm::vec3(26.5f, 0.19f, -55.12f),
        glm::vec3(39.0f, 0.19f, -65.12f),
        glm::vec3(26.5f, 0.19f, -74.12f),
        glm::vec3(-10.0f, 0.19f, -30.12f),
        glm::vec3(-30.0f, 0.19f, -30.12f),
        glm::vec3(-50.0f, 0.19f, -30.12f),
        glm::vec3(-70.0f, 0.19f, -30.12f),
        glm::vec3(-20.0f, 0.19f, 20.62f),
        glm::vec3(-40.0f, 0.19f, -20.62f),
        glm::vec3(-66.0f, 0.19f, -20.62f),
        glm::vec3(26.5f, 0.19f, -2.36f),
        glm::vec3(26.5f, 0.19f, -20.88f),
        glm::vec3(40.0f, 0.19f, -30.12f),
        glm::vec3(50.0f, 0.19f, -20.62f),
        glm::vec3(60.0f, 0.19f, -30.12f),
        glm::vec3(70.0f, 0.19f, -20.62f),
        glm::vec3(39.0f, 0.19f, -5.62f),
        glm::vec3(39.0f, 0.19f, 12.38f),
        glm::vec3(0.0f, 0.19f, 32.73f),
        glm::vec3(10.0f, 0.19f, 21.1f),
        glm::vec3(20.0f, 0.19f, 32.74f),
        glm::vec3(-10.0f, 0.19f, 21.1f),
        glm::vec3(-30.0f, 0.19f, 21.1f),
        glm::vec3(-50.0f, 0.19f, 21.1f),
        glm::vec3(-70.0f, 0.19f, 21.1f),
        glm::vec3(-20.0f, 0.19f, 32.74f),
        glm::vec3(-40.0f, 0.19f, 32.74f),
        glm::vec3(-66.0f, 0.19f, 32.74f),
        glm::vec3( 40.0f, 0.19f, 21.1f),
        glm::vec3( 50.0f, 0.19f, 32.74f),
        glm::vec3( 60.0f, 0.19f, 21.1f),
        glm::vec3( 70.0f, 0.19f, 32.74f),
        glm::vec3(-65.5f, 0.19f, -15.62f),
        glm::vec3(-65.5f, 0.19f,  2.38f),
        glm::vec3(-53.0f, 0.19f, -5.62f),
        glm::vec3(-53.0f, 0.19f, 12.38f),
    };

    float lamp_posts_rotation[42] =
    {
        0.0f, 180.0f, 0.0f, -90.0f, 90.0f, -90.0f, 90.0f, -90.0f, 180.0f, 180.0f, 180.0f, 180.0f, 0.0f,
        0.0f, 0.0f, -90.0f, -90.0f, 180.0f, 0.0f, 180.0f, 0.0f, 90.0f, 90.0f, 0.0f, 180.0f, 0.0f, 180.0f,
        180.0f, 180.0f, 180.0f, 0.0f, 0.0f, 0.0f, 180.0f, 0.0f, 180.0f, 0.0f, -90.0f, -90.0f, 90.0f, 90.0f
    };

    glm::vec3 stop_signs_translation[5] =
    {
        glm::vec3(26.38f,  1.63f, -20.63f),
        glm::vec3(38.74f,  1.63f, -30.63f),
        glm::vec3(-65.51f, 1.63f, 21.13f),
        glm::vec3(38.73f,  1.63f, 21.34f),
        glm::vec3(-53.17f, 1.63f, -20.92f)
    };

    glm::vec3 ext_build_5_translation = glm::vec3(58.0f, 0.25f, 0.0f);
    glm::vec3 ext_build_1_translation = glm::vec3(58.0f, 0.25f, -54.09f);
    glm::vec3 ext_build_4_translation = glm::vec3(-62.19f, 0.25f, -53.26f);
    glm::vec3 ext_build_3_translation = glm::vec3(-35.14f, 0.25f, 55.49f);
    glm::vec3 ext_build_2_translation = glm::vec3( 53.71f, -1.87f, 53.85f);

    glm::vec3 lightcube_positions[2] =
    {
        glm::vec3(-24.50f, 1.25f, -40.05f),
        glm::vec3(-17.0f, 5.75f, -55.25f)
    };

    // Main Render loop
    while(!glfwWindowShouldClose(lightingWindow))
    {
        currentframetime  = glfwGetTime();
        framedeltatime    = currentframetime - lastframerendered;
        lastframerendered = currentframetime;

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        inputPolling(lightingWindow);

        viewMatrix = cam.getViewMatrix();
        modelMatrix = glm::mat4(1.0f);
        projectionMatrix = glm::perspective(glm::radians(cam.zoom), (float) windowwidth / (float) windowheight, 0.1f, 5000.0f);


        //animates the fireflies in the grass section
        lightcube_positions[0].x -= 0.8*sin(glfwGetTime());
        lightcube_positions[0].y += 0.10*sin(glfwGetTime()*6);

        lightcube_positions[1].x -= 0.5*sin(glfwGetTime()*1);
        lightcube_positions[1].y += 0.3*sin(glfwGetTime()*3);
        lightcube_positions[1].z -= 0.5*cos(glfwGetTime()*1);

        glm::vec3 lightcolor = glm::vec3(1.0f, 1.0f, 0.85f);

        glm::vec3 diffusecolor = glm::vec3(1.0f);
        glm::vec3 ambientcolor = glm::vec3(1.0f);
        glm::vec3 specularcolor = glm::vec3(0.501f, 0.501f, 0.501f);

        ambientcolor = lightcolor * glm::vec3(0.0f, 0.1f, 0.06f); // Decreases the ambient strength to 15% of its total strength
        diffusecolor = lightcolor * glm::vec3(0.0f, 0.509f, 0.509f); // Decreases the diffuse strength to 55% of its total strength

        basicshader.useShader();

        basicshader.setVec3vect("material.ambientlight", ambientcolor);
        basicshader.setVec3vect("material.diffuselight", diffusecolor);
        basicshader.setVec3vect("material.specularlight", specularcolor);
        basicshader.setFloat("material.shininessval", 1.0f);

        basicshader.setVec3vect("dlight.ambientstrength",  lightcolor * 0.10f);
        basicshader.setVec3vect("dlight.diffusestrength",  lightcolor * 0.10f);
        basicshader.setVec3vect("dlight.specularstrength", lightcolor);

        basicshader.setVec3vect("dlight.direction", glm::vec3(0.2f, 1.0f, 0.1f) ); // Directional Light

        basicshader.setVec3vect("olight[0].position", glm::vec3(viewMatrix * glm::vec4(lightcube_positions[0], 1.0f) ) ); // Point (Omnidirectional) Light
        basicshader.setVec3vect("olight[0].diffusestrength",  lightcolor * 0.95f);
        basicshader.setVec3vect("olight[0].specularstrength", lightcolor);
        basicshader.setFloat("olight[0].constantattenuation",  1.0f);
        basicshader.setFloat("olight[0].linearattenuation",    0.09f);
        basicshader.setFloat("olight[0].quadraticattenuation", 0.032f);
        basicshader.setVec3vect("olight[1].position", glm::vec3(viewMatrix * glm::vec4(lightcube_positions[1], 1.0f) ) ); // Point (Omnidirectional) Light
        basicshader.setVec3vect("olight[1].diffusestrength",  lightcolor * 0.95f);
        basicshader.setVec3vect("olight[1].specularstrength", lightcolor);
        basicshader.setFloat("olight[1].constantattenuation",  1.0f);
        basicshader.setFloat("olight[1].linearattenuation",    0.09f);
        basicshader.setFloat("olight[1].quadraticattenuation", 0.032f);


        for(int i = 0; i < 42; i++) // Renders 42 extra point lights in order to destroy the iGPU, localized on the map's lamp posts.
        {
            std::stringstream olightstr;

            olightstr  << "olight[" << i+2 << "].position" ;
            if(lamp_posts_rotation[i] == 0.0f)
                basicshader.setVec3vect( (const std::string&) olightstr.str(), glm::vec3(viewMatrix * glm::vec4(lamp_posts_translation[i] + glm::vec3(-0.8f, 2.0f, 0.0f), 1.0f) ) );

            else if (lamp_posts_rotation[i] == 90.0f)
                basicshader.setVec3vect( (const std::string&) olightstr.str(), glm::vec3(viewMatrix * glm::vec4(lamp_posts_translation[i] + glm::vec3(0.0f, 2.0f, -0.8f), 1.0f) ) );

            else if (lamp_posts_rotation[i] == -90.0f)
                basicshader.setVec3vect( (const std::string&) olightstr.str(), glm::vec3(viewMatrix * glm::vec4(lamp_posts_translation[i] + glm::vec3(0.0f, 2.0f, 0.8f), 1.0f) ) );

            else if (lamp_posts_rotation[i] == 180.0f)
                basicshader.setVec3vect( (const std::string&) olightstr.str(), glm::vec3(viewMatrix * glm::vec4(lamp_posts_translation[i] + glm::vec3(0.8f, 2.0f, 0.0f), 1.0f) ) );


            olightstr.str("");
            olightstr  << "olight[" << i+2 << "].diffusestrength" ;
            basicshader.setVec3vect( (const std::string&) olightstr.str(),  lightcolor * 0.95f);

            olightstr.str("");
            olightstr  << "olight[" << i+2 << "].specularstrength" ;
            basicshader.setVec3vect( (const std::string&) olightstr.str(), lightcolor);

            olightstr.str("");
            olightstr  << "olight[" << i+2 << "].constantattenuation" ;
            basicshader.setFloat( (const std::string&) olightstr.str(), 1.0f);

            olightstr.str("");
            olightstr  << "olight[" << i+2 << "].linearattenuation" ;
            basicshader.setFloat( (const std::string&) olightstr.str(), 0.09f);

            olightstr.str("");
            olightstr  << "olight[" << i+2 << "].quadraticattenuation" ;
            basicshader.setFloat( (const std::string&) olightstr.str(), 0.032f);
        }

        basicshader.setVec3vect("slight[0].diffusestrength",  lightcolor * 0.55f);
        basicshader.setVec3vect("slight[0].specularstrength", lightcolor);
        basicshader.setMat4("slight[0].viewmatrix", viewMatrix); // Spotlight (Flashlight coming from the camera's position)
        basicshader.setVec3vect("slight[0].position", cam.position); // Spotlight (Flashlight coming from the camera's position)
        basicshader.setVec3vect("slight[0].direction", cam.front); // Spotlight (Flashlight coming from the camera's position)
        basicshader.setFloat("slight[0].coneinnercutoff", glm::cos(glm::radians(12.5f)));


        basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        basicshader.setMat4("modelmatrix", modelMatrix);
        basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));


        vegetationshader.useShader();

        vegetationshader.setVec3vect("material.ambientlight", ambientcolor);
        vegetationshader.setVec3vect("material.diffuselight", diffusecolor);
        vegetationshader.setVec3vect("material.specularlight", specularcolor);
        vegetationshader.setFloat("material.shininessval", 1.0f);

        vegetationshader.setVec3vect("dlight.ambientstrength",  lightcolor * 0.10f);
        vegetationshader.setVec3vect("dlight.diffusestrength",  lightcolor * 0.10f);
        vegetationshader.setVec3vect("dlight.specularstrength", lightcolor);

        vegetationshader.setVec3vect("dlight.direction", glm::vec3(0.2f, 1.0f, 0.1f) ); // Directional Light


        vegetationshader.setVec3vect("olight[0].position", glm::vec3(viewMatrix * glm::vec4(lightcube_positions[0], 1.0f) ) ); // Point (Omnidirectional) Light
        vegetationshader.setVec3vect("olight[0].diffusestrength",  lightcolor * 1.0f);
        vegetationshader.setVec3vect("olight[0].specularstrength", lightcolor);
        vegetationshader.setFloat("olight[0].constantattenuation",  1.0f);
        vegetationshader.setFloat("olight[0].linearattenuation",    0.04f);
        vegetationshader.setFloat("olight[0].quadraticattenuation", 0.007f);
        vegetationshader.setVec3vect("olight[1].position", glm::vec3(viewMatrix * glm::vec4(lightcube_positions[1], 1.0f) ) ); // Point (Omnidirectional) Light
        vegetationshader.setVec3vect("olight[1].diffusestrength",  lightcolor * 1.0f);
        vegetationshader.setVec3vect("olight[1].specularstrength", lightcolor);
        vegetationshader.setFloat("olight[1].constantattenuation",  1.0f);
        vegetationshader.setFloat("olight[1].linearattenuation",    0.09f);
        vegetationshader.setFloat("olight[1].quadraticattenuation", 0.032f);
        vegetationshader.setVec3vect("olight[1].position", glm::vec3(viewMatrix * glm::vec4(lightcube_positions[1], 1.0f) ) ); // Point (Omnidirectional) Light
        vegetationshader.setVec3vect("olight[1].diffusestrength",  lightcolor * 1.0f);
        vegetationshader.setVec3vect("olight[1].specularstrength", lightcolor);
        vegetationshader.setFloat("olight[1].constantattenuation",  1.0f);
        vegetationshader.setFloat("olight[1].linearattenuation",    0.09f);
        vegetationshader.setFloat("olight[1].quadraticattenuation", 0.032f);

        vegetationshader.setVec3vect("slight[0].diffusestrength",  lightcolor * 0.55f);
        vegetationshader.setVec3vect("slight[0].specularstrength", lightcolor);
        vegetationshader.setMat4("slight[0].viewmatrix", viewMatrix); // Spotlight (Flashlight coming from the camera's position)
        vegetationshader.setVec3vect("slight[0].position", cam.position); // Spotlight (Flashlight coming from the camera's position)
        vegetationshader.setVec3vect("slight[0].direction", cam.front); // Spotlight (Flashlight coming from the camera's position)
        vegetationshader.setFloat("slight[0].coneinnercutoff", glm::cos(glm::radians(12.5f)));
        vegetationshader.setFloat("runtime", glfwGetTime());
        vegetationshader.setFloat("forcex", 1.0f);
        vegetationshader.setFloat("forcey", 0.4f);
        vegetationshader.setFloat("forcez", 0.4f);

        vegetationshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        vegetationshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        vegetationshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        vegetationshader.setMat4("modelmatrix", modelMatrix);
        vegetationshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

        basicshader.useShader();

        terrain.renderModel(basicshader);

        vegetationshader.useShader();

        vegetationshader.setFloat("forcex", 0.1f);
        vegetationshader.setFloat("forcey", 0.0f);
        vegetationshader.setFloat("forcez", 0.0f);
        vegetationshader.setBool("emit", true);

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, grass_types_translation[0]);


        vegetationshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        vegetationshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        vegetationshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        vegetationshader.setMat4("modelmatrix", modelMatrix);
        vegetationshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

        grass_1.renderModel(vegetationshader);



        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, grass_types_translation[1]);


        vegetationshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        vegetationshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        vegetationshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        vegetationshader.setMat4("modelmatrix", modelMatrix);
        vegetationshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));
        vegetationshader.setBool("emit", false);

        grass_2.renderModel(vegetationshader);


        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, grass_types_translation[2]);

        vegetationshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        vegetationshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        vegetationshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        vegetationshader.setMat4("modelmatrix", modelMatrix);
        vegetationshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

        grass_small.renderModel(vegetationshader);

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, shrubs_translation);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        vegetationshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        vegetationshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        vegetationshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        vegetationshader.setMat4("modelmatrix", modelMatrix);
        vegetationshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

        shrubs.renderModel(vegetationshader);

        basicshader.useShader();

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, moon_translation + cam.position); // Makes the moon seem "Infinitely far away" and not be affected by the player's position

        basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        basicshader.setMat4("modelmatrix", modelMatrix);
        basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));
        basicshader.setBool("emit", true);
        basicshader.setFloat("emitmul", 1.8f);

        moon.renderModel(basicshader);

        modelMatrix = glm::mat4(1.0f);

        basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        basicshader.setMat4("modelmatrix", modelMatrix);
        basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));
        basicshader.setFloat("emitmul", 1.0f);

        skybox.renderModel(basicshader);



        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, tree_and_leaves_translation[0]);


        basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        basicshader.setMat4("modelmatrix", modelMatrix);
        basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));
        basicshader.setBool("emit", false);

        big_tree.renderModel(basicshader);



        vegetationshader.useShader();

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, tree_and_leaves_translation[1]);

        vegetationshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        vegetationshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        vegetationshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        vegetationshader.setMat4("modelmatrix", modelMatrix);
        vegetationshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));
        vegetationshader.setFloat("forcex", 1.0f);
        vegetationshader.setFloat("forcey", 0.4f);
        vegetationshader.setFloat("forcez", 0.4f);

        tree_leaves.renderModel(vegetationshader);


        for(int i = 0; i < 4; i++) // Renders the maple trees and their leaves.
        {
            basicshader.useShader();

            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, mapletree_and_leaves_translation[i]);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(i*90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
            basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
            basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
            basicshader.setMat4("modelmatrix", modelMatrix);
            basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

            maple_tree.renderModel(basicshader);

            vegetationshader.useShader();

            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, mapletree_and_leaves_translation[i]);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(i*90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

            vegetationshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
            vegetationshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
            vegetationshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
            vegetationshader.setMat4("modelmatrix", modelMatrix);
            vegetationshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

            maple_tree_leaves.renderModel(vegetationshader);
        }

        basicshader.useShader();

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, plant_holder_translation);


        basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
        basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
        basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
        basicshader.setMat4("modelmatrix", modelMatrix);
        basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

        plant_holder.renderModel(basicshader);

        for(int i = 0; i < 42; i++) // Renders the lamp posts spread throughout the scene
        {
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, lamp_posts_translation[i]);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(lamp_posts_rotation[i]), glm::vec3(0.0f, 1.0f, 0.0f));


            basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
            basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
            basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
            basicshader.setMat4("modelmatrix", modelMatrix);
            basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

            lamp_post.renderModel(basicshader);
        }


        for(int i = 0; i < 5; i++) // Renders the stop signs
        {
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, stop_signs_translation[i]);
            modelMatrix = glm::rotate(modelMatrix, glm::radians(i*90.0f), glm::vec3(0.0f, 1.0f, 0.0f));


            basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
            basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
            basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
            basicshader.setMat4("modelmatrix", modelMatrix);
            basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

            stop_sign.renderModel(basicshader);
        }


            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, ext_build_5_translation);

            basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
            basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
            basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
            basicshader.setMat4("modelmatrix", modelMatrix);
            basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

            ext_build_5.renderModel(basicshader);

            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, ext_build_4_translation);

            basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
            basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
            basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
            basicshader.setMat4("modelmatrix", modelMatrix);
            basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

            ext_build_4.renderModel(basicshader);

            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, ext_build_1_translation);

            basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
            basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
            basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
            basicshader.setMat4("modelmatrix", modelMatrix);
            basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

            ext_build_1.renderModel(basicshader);

            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, ext_build_3_translation);

            basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
            basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
            basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
            basicshader.setMat4("modelmatrix", modelMatrix);
            basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

            ext_build_3.renderModel(basicshader);

            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, ext_build_2_translation);

            basicshader.setMat4("viewmatrix", viewMatrix); // Gets the camera's position in order to calculate lighting normals and fragments according to it.
            basicshader.setMat4("transinvviewmatrix", glm::transpose(glm::inverse(viewMatrix)));
            basicshader.setMat4("projectionmatrix", projectionMatrix); // Both of those matrices can be put inside the for() render loop, but are not necessary
            basicshader.setMat4("modelmatrix", modelMatrix);
            basicshader.setMat4("transinvmodelmatrix", glm::transpose(glm::inverse(modelMatrix)));

            ext_build_2.renderModel(basicshader);



        for(int i = 0; i < 2; i ++) // renders the little "fireflies" near the grass section
        {
            modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, lightcube_positions[i]);
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.075f));

            coloredlightshader.useShader();

            coloredlightshader.setMat4("projectionmatrix", projectionMatrix);
            coloredlightshader.setMat4("viewmatrix", viewMatrix);
            coloredlightshader.setMat4("modelmatrix", modelMatrix);
            coloredlightshader.setVec3vect("lightcolor", lightcolor);

            lightcube.renderModel(coloredlightshader);
        }

        //std::cout << "Cam Pos: X " << cam.position.x << " | Y " << cam.position.y << " | Z " << cam.position.z << std::endl;


        glfwSwapBuffers(lightingWindow);
        glfwPollEvents();
    }

    //OpenGL cleanup, and Window termination.
    glDeleteShader(vegetationshader.shader_id);
    glDeleteShader(coloredlightshader.shader_id);
    glDeleteShader(basicshader.shader_id);
    glfwTerminate();
    return 0;
}


void inputPolling(GLFWwindow *window)
{
    bool running = false, crouching = false;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        running = true;

    else if(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        crouching = true;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam.checkKeyboardPresses(MOVE_FORWARD, running, crouching, framedeltatime);
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam.checkKeyboardPresses(MOVE_BACKWARDS, running, crouching, framedeltatime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam.checkKeyboardPresses(MOVE_LEFT, running, crouching, framedeltatime);
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam.checkKeyboardPresses(MOVE_RIGHT, running, crouching, framedeltatime);
}


void mousePolling(GLFWwindow *window, double xposition, double yposition)
{
    if(firstpolling)
    {
        mouselastxposition = xposition;
        mouselastyposition = yposition;
        firstpolling = false;
    }

    float mousexoffset = xposition - mouselastxposition;
    float mouseyoffset = mouselastyposition - yposition;
    mouselastxposition = xposition;
    mouselastyposition = yposition;

    cam.checkMouseMovement(mousexoffset, mouseyoffset);
}


void mouseWheelPolling(GLFWwindow* window, double xoffset, double yoffset)
{
    cam.checkMouseWheel(yoffset);
}

void resizewin(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
