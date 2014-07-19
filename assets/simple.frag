#version 130

uniform sampler2D	texture;
in vec2				texCoord;
out vec4			fragmentColor;

void main() 
{
	vec4 texcolor = texture2D(texture, texCoord.xy);
	fragmentColor = vec4(texcolor.xyzw);
}