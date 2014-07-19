uniform sampler2D texture;

void main(void) {
	vec4 texcolor = texture2D(texture, gl_TexCoord[0].xy);
	if(texcolor!=vec4(0.0,0.0,0.0,1.0))
		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	else
		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}