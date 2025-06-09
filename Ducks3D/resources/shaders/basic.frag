#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D _texture;

void main() {
    FragColor = texture(_texture, TexCoord); 
}