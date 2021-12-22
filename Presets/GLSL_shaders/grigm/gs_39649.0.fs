/*
{
  "CATEGORIES" : [
    "Automatically Converted",
    "GLSLSandbox"
  ],
  "INPUTS" : [
	{
		"NAME": "rotate",
		"TYPE": "float",
		"DEFAULT": 0,
		"MIN": -1,
		"MAX": 1
	},
	{
		"NAME": "zoom",
		"TYPE": "float",
		"DEFAULT": 1,
		"MIN": 0.5,
		"MAX": 5
	}
  ],
  "DESCRIPTION" : "Automatically converted from http:\/\/glslsandbox.com\/e#39649.0"
}
*/


#ifdef GL_ES
precision mediump float;
#endif

#extension GL_OES_standard_derivatives : enable

#define PI 3.14159265359

mat2 rotate2d(float _angle){
    return mat2(cos(_angle),-sin(_angle),
                sin(_angle),cos(_angle));
}

float circle(vec2 coord, float spd)
{
    float reso = 12.0;
    float cw = RENDERSIZE.x / reso;

    vec2 p = mod(coord, cw);
    float d = distance(p, vec2(cw / 2.0));

    float rnd = dot(floor(coord / cw), vec2(1323.443, 1412.312));
    float t = TIME * 2.0 + fract(sin(rnd)) * 6.2;

    float l = cw * (sin(t * spd) * 0.25 + 0.25);
    return clamp(l - d, 0.0, 1.0);
}

void main()
{
    vec2 p = (gl_FragCoord.xy - RENDERSIZE / 2.0) / zoom;
    
	
    p = rotate2d(rotate*PI ) * p;
    //p *= zoom;
    
    
    vec2 dp = vec2(7.9438, 1.3335) * TIME;
    float c1 = circle(p - dp, 1.0);
    float c2 = circle(p + dp, 1.4);
    float c = max(0.0, abs(c1 - c2));
    gl_FragColor = vec4(c, c, c, 1);
}