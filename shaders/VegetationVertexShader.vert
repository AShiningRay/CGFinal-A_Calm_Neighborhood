#version 430 core
layout (location = 0) in vec3 attributepos;
layout (location = 1) in vec3 attributenormals;
layout (location = 2) in vec2 attribtexcoords;
layout (location = 3) in vec2 attribtangent;
layout (location = 4) in vec2 attribbitangent;

out vec3 diromnifragmentposition;
out vec3 spotfragmentposition;
out vec3 directionalspotnormals;
out vec3 omninormals;

out vec2 texturecoord;

uniform mat4 modelmatrix;
uniform mat4 viewmatrix;
uniform mat4 projectionmatrix;
uniform mat4 transinvmodelmatrix;
uniform mat4 transinvviewmatrix;

uniform float runtime;

uniform float veg_move_speed = 1.5f;
uniform float veg_move_length = 8.0f;
uniform float forcex = 1.0f;
uniform float forcey = 0.4f;
uniform float forcez = 0.4f;

void main()
{
    spotfragmentposition = vec3(modelmatrix * vec4(attributepos, 1.0f));
    diromnifragmentposition = vec3(viewmatrix * modelmatrix * vec4(attributepos, 1.0f));

    directionalspotnormals = mat3(transinvmodelmatrix) * attributenormals;
    omninormals = mat3(transinvviewmatrix) * mat3(transinvmodelmatrix) * attributenormals; // Here we need the transposed inverse view matrix because the light source is a point in a near space, not coming from the camera or an infinitely far distance.

    texturecoord = attribtexcoords;
    vec3 attrib = attributepos;
    attrib.x += sin(attrib.x * veg_move_length * 1.15f + runtime * veg_move_speed) * forcex;
    attrib.y += sin(attrib.y * veg_move_length + runtime * veg_move_speed * 1.27f) * forcey;
    attrib.z += sin(attrib.z * veg_move_length * 0.76f + runtime * veg_move_speed * 1.40f) * forcez;

    gl_Position = projectionmatrix * viewmatrix * modelmatrix * vec4(attrib, 1.0f);
}
