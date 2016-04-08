#version 410 core

#define PI 3.14159265358979323846

in vec4 gl_FragCoord;
in vec2 UV;

uniform float iGlobalTime;
uniform vec4 inColour;

out vec4 fragColour;

float rand(vec2 c){
    return fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 p, float freq ){
    float unit = 16.0f / freq;
    vec2 ij = floor(p/unit);
    vec2 xy = mod(p,unit)/unit;
    //xy = 3.*xy*xy-2.*xy*xy*xy;
    xy = .5*(1.-cos(PI*xy));
    float a = rand((ij+vec2(0.,0.)));
    float b = rand((ij+vec2(1.,0.)));
    float c = rand((ij+vec2(0.,1.)));
    float d = rand((ij+vec2(1.,1.)));
    float x1 = mix(a, b, xy.x);
    float x2 = mix(c, d, xy.x);
    return mix(x1, x2, xy.y);
}

void main()
{
    vec2 tUV = UV * iGlobalTime;

    float dist = length(vec2(0.5, 0.5) - UV);
    fragColour = vec4(0.5, 0.5, 0.5, 1.0);
    fragColour.a = mix(1.0, 0.0, dist * 2.0);
    fragColour.a *= noise(tUV, 1.0) * noise(-tUV, 0.25);
    fragColour *= inColour;
}