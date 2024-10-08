#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D sprite;

in float lf;

void main()
{
    float life = lf;
    vec4 texColor = texture(sprite, TexCoords);
    float h = 1.0f/2.0f;
    float halfN = pow(h,3.0);
    texColor.a = texColor.a * ( -halfN*sin( h*life ) + halfN );
    color = texColor;
}
