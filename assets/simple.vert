#version 130

uniform vec2 resolution;

in vec3		 position;
in vec2		 texCoordIn; 
out vec2	 texCoord;

void main() {
	gl_Position = vec4(position.xy,0.0,1.0);
	texCoord = texCoordIn;
}