#version 450 core

uniform sampler2D u_Texture;

in vec2 v_TexCoords;
out vec4 FragColor;

void main()
{
    //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    FragColor = texture(u_Texture, v_TexCoords);
}