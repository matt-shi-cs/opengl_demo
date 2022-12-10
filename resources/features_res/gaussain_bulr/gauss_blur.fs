#version 330 core
/*
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform vec2 resolution;
uniform sampler2D ourTexture;

const float pi = 3.1415926;
const int samples = 15;
const float sigma = float(samples) * 0.25;
const float s = 2 * sigma * sigma; 

float gauss(vec2 i)
{
    return exp(-(i.x * i.x + i.y * i.y) / s) / (pi * s);
}

vec3 gaussianBlur(sampler2D sp, vec2 uv, vec2 scale)
{
    vec3 pixel = vec3(0.0);
    float weightSum = 0.0;
    float weight = 0.0;
    vec2 offset;

    for(int i = -samples / 2; i < samples / 2; i++)
    {
        for(int j = -samples / 2; j < samples / 2; j++)
        {
            offset = vec2(i, j);
            weight = gauss(offset);
            pixel += texture(sp, uv + scale * offset).rgb * weight;
            weightSum += weight;
        }
    }
    return pixel / weightSum;
}

void main()
{
    //vec2 ps = vec2(1.0) / resolution.xy;
    //vec2 ps = vec2(0.0);
    //FragColor = vec4(gaussianBlur(ourTexture,TexCoord, ps).rgb, 1.0);
    FragColor = texture(ourTexture, TexCoord);//texture
}
*/

out vec4 FragColor;
  
in vec3  DefaultColor;
in vec2  TexCoords;


uniform sampler2D image;
uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, TexCoords).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}

