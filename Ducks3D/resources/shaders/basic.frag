#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D _texture;
uniform vec3 color;

void main() {
    FragColor = texture(_texture, TexCoord) * vec4(color, 1.0f); 
}