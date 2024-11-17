#version 450 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColour;
layout(location = 2) in vec3 aNormal;

uniform mat4 ViewProjectionMatrix;
uniform vec3 CameraPos;

out vec3 v_Normal;
out vec3 v_Colour;
out vec3 v_DirToCamera;

void main()
{
	gl_Position = ViewProjectionMatrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);
	v_Normal = aNormal;
	v_Colour = aColour;
	v_DirToCamera = CameraPos - aPos;
}