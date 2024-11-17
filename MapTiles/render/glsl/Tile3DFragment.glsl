#version 450 core

in vec3 v_Normal;
in vec3 v_Colour;
in vec3 v_DirToCamera;
out vec4 FragColor;

void main()
{
    //float brightness = max(0, dot(v_Normal, normalize(vec3(3, -1, 1))));      //fixed lighting
    float brightness = max(0,dot(v_Normal, normalize(v_DirToCamera)));
    FragColor = vec4(v_Colour, 1.0f) * brightness * 0.8;
}