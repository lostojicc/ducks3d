#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D _texture;

void main() {
	vec2 flippedTexCoord = vec2(TexCoord.x, 1.0 - TexCoord.y);
	vec4 texColor = texture(_texture, flippedTexCoord);

	if (texColor.a < 0.1)
		discard;

	FragColor = texColor;

	//FragColor = vec4(1.0f, 0.0f, 0.0f, 0.0f);
}