#version 430 core
struct shader_material
{
    float shininessval;
    sampler2D diffuse_texture1;
    sampler2D diffuse_texture2;
    sampler2D diffuse_texture3;
    sampler2D diffuse_texture4;
    sampler2D specular_texture1;
    sampler2D specular_texture2;
    sampler2D specular_texture3;
    sampler2D specular_texture4;
    sampler2D height_texture1;
    sampler2D height_texture2;
    sampler2D height_texture3;
    sampler2D height_texture4;
    sampler2D normal_texture1;
    sampler2D normal_texture2;
    sampler2D normal_texture3;
    sampler2D normal_texture4;
    sampler2D emissionmap;
};

struct shader_light
{
    vec3 direction;
    vec3 ambientstrength;
    vec3 diffusestrength;
    vec3 specularstrength;
};

struct DirectionalLight
{
    vec3 direction;
    vec3 ambientstrength;
    vec3 diffusestrength;
    vec3 specularstrength;
};

struct OmniLight
{
    float constantattenuation;
    float linearattenuation;
    float quadraticattenuation;

    vec3 position;
    vec3 ambientstrength;
    vec3 diffusestrength;
    vec3 specularstrength;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;

    float coneinnercutoff;
    vec3 ambientstrength;
    vec3 diffusestrength;
    vec3 specularstrength;
};

#define POINT_LIGHTS 2
#define SPOT_LIGHTS 1

in vec2 texturecoord;
in vec3 diromnifragmentposition;
in vec3 spotfragmentposition;
in vec3 directionalspotnormals;
in vec3 omninormals;

uniform shader_material material;
uniform DirectionalLight dlight;
uniform OmniLight olight[POINT_LIGHTS];
uniform SpotLight slight[SPOT_LIGHTS];
uniform bool emit = false;


out vec4 fragmentColor;


vec3 calculateDirectionalLight(DirectionalLight lightobj, vec3 directionalnormals, vec3 fragmentposition);
vec3 calculateOmniLight(OmniLight lightobj, vec3 omninormals, vec3 fragmentposition);
vec3 calculateSpotLight(SpotLight lightobj, vec3 spotnormals, vec3 fragmentposition);

void main()
{
    vec3 resultantlighting;
    resultantlighting += calculateDirectionalLight(dlight, directionalspotnormals, diromnifragmentposition);
    int i = 0;

    if(texture(material.diffuse_texture1, texturecoord).a < 0.18) // Checks if the texture has transparency(alpha channel), and if it is, discards fragments less opaque than 0.18(18%)
        discard;

    //if(POINT_LIGHTS > 0)
    for(i = 0; i < POINT_LIGHTS; i++)
        resultantlighting += calculateOmniLight(olight[i], omninormals, diromnifragmentposition);

    //for(i = 0; i < SPOT_LIGHTS; i++)
    //    resultantlighting += calculateSpotLight(slight[i], directionalspotnormals, spotfragmentposition);

    //vec3 emissionmap = texture(material.emissionmap, texturecoord).rgb;
    //if(useemissionmap)
    //    resultantlighting += emissionmap;

    if(emit)
    {
        vec3 emissionmap = texture(material.emissionmap, texturecoord).rgb;
        resultantlighting += emissionmap;
        fragmentColor = vec4(resultantlighting, 1.0);
    }


    fragmentColor = vec4(resultantlighting, 1.0);
}

vec3 calculateDirectionalLight(DirectionalLight lightobj, vec3 directionalnormals, vec3 fragmentposition)
{
    vec3 ambientlight = vec3(texture(material.diffuse_texture1, texturecoord)) * lightobj.ambientstrength; // The first float value is the strength of the ambient light

    vec3 normalized = normalize(directionalnormals);
    vec3 lightdirection = normalize(lightobj.direction);
    float diffuselightvalue = max(dot(normalized, lightdirection), 0.0);
    vec3 diffusemap = vec3(texture(material.diffuse_texture1, texturecoord)) * lightobj.diffusestrength * diffuselightvalue;

    vec3 viewdirection = normalize(-fragmentposition);
    vec3 reflectdirection = reflect(-lightdirection, normalized);
    float specularlightvalue = pow(max(dot(viewdirection, reflectdirection), 0.0), material.shininessval);
    vec3 specularmap = specularlightvalue * vec3(texture(material.specular_texture1, texturecoord)) * lightobj.specularstrength; // The first float value is the general strength of the diffuse light

    // Returns a vec3, so the parenthesis are needed, and only the directional light has an ambient value, to prevent it from adding with other lights and giving maximum lighting if there are too many lights
    return 2*(ambientlight + diffusemap);// + specularmap); Specularmap is glitchy, so it was removed from the directional light calculations
}

vec3 calculateOmniLight(OmniLight lightobj, vec3 omninormals, vec3 fragmentposition)
{
    float lightdist = length(lightobj.position - fragmentposition);
    float lightattenuation = 1.0/(lightobj.constantattenuation + lightobj.linearattenuation * lightdist + lightobj.quadraticattenuation * (lightdist * lightdist));

    vec3 normalized = normalize(omninormals);
    vec3 lightdirection = normalize(lightobj.position - fragmentposition);
    float diffuselightvalue = max(dot(normalized, lightdirection), 0.0);
    vec3 diffuse_texture1 = vec3(texture(material.diffuse_texture1, texturecoord)) * lightobj.diffusestrength * diffuselightvalue;

    vec3 viewdirection = normalize(-fragmentposition);
    vec3 reflectdirection = reflect(-lightdirection, normalized);
    float specularlightvalue = pow(max(dot(viewdirection, reflectdirection), 0.0), material.shininessval);
    vec3 specular_texture1 = specularlightvalue * vec3(texture(material.specular_texture1, texturecoord)) * lightobj.specularstrength; // The first float value is the general strength of the diffuse light

    diffuse_texture1 *= lightattenuation;
    specular_texture1 *= lightattenuation;

    return (diffuse_texture1 + specular_texture1); // Returns a vec3, so the parenthesis are needed
}

vec3 calculateSpotLight(SpotLight lightobj, vec3 spotnormals, vec3 fragmentposition)
{
    vec3 normalized = normalize(spotnormals);
    vec3 lightdirection = normalize(lightobj.position - fragmentposition);
    float diffuselightvalue = max(dot(normalized, lightdirection), 0.0f);
    vec3 diffuse_texture1 = vec3(texture(material.diffuse_texture1, texturecoord)) * lightobj.diffusestrength * diffuselightvalue;

    vec3 viewdirection = normalize(-fragmentposition);
    vec3 reflectdirection = reflect(-lightdirection, normalized);
    float specularlightvalue = pow(max(dot(viewdirection, reflectdirection), 0.0f), material.shininessval);
    vec3 specular_texture1 = specularlightvalue * vec3(texture(material.specular_texture1, texturecoord)) * lightobj.specularstrength; // The first float value is the general strength of the diffuse light

    float thetaval = dot(lightdirection, normalize(-lightobj.direction));
    float gammaval = lightobj.coneinnercutoff - (lightobj.coneinnercutoff*0.995f); // The closer the last multiplying float is to 1, the sharper the borders of the spotlight's light cone will be
    float spotlightintensity = clamp((thetaval - (lightobj.coneinnercutoff*0.995f)) / gammaval, 0.0f, 1.0f);

    return (diffuse_texture1*spotlightintensity + specular_texture1*spotlightintensity);
}
