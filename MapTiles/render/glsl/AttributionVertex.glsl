#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

uniform mat4 ViewProjectionMatrix;

out vec2 v_TexCoords;

void main()
{
	gl_Position = vec4(aPos.xyz, 1.0);
	v_TexCoords = aTexCoords;
}