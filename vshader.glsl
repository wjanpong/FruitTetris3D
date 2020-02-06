#version 130

in vec4 vPosition;
in vec4 vColor;
out vec4 color;

uniform mat4 M;

void main() 
{
	gl_Position = M * vPosition;

	color = vColor;	
} 