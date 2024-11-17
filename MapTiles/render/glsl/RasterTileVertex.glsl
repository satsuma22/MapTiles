#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

uniform mat4 ViewProjectionMatrix;

out vec2 v_TexCoords;

void main()
{
	gl_Position = ViewProjectionMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	v_TexCoords = aTexCoords;
}