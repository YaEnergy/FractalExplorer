#version 330 core

#define PI 3.1415926535897932384626433

in vec2 fragTexCoord;
in vec4 fragColor;

uniform float widthStretch = 1.0;

uniform int maxIterations = 20;

uniform vec2 position = vec2(0.0, 0.0);
uniform vec2 offset = vec2(0.0, 0.0);

uniform float zoom = 1.0;

uniform int colorBanding = 0;

uniform vec2 a = vec2(1.0, 0.0);

out vec4 finalColor;

//z = a * b
vec2 ComplexMultiply(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

//z = a / b
vec2 ComplexDivide(vec2 a, vec2 b)
{
    //ref: Delta 4B, p. 133
			
			//z1 / z2
			//(a + bi) / (c + di)
			//(a + bi) / (c + di) * ((c - di) / (c - di))
			//(ac - adi + bco - bdi*i)/(c*c-d*d*i*i)
			//(ac + bd - adi + bci)/(c*c+d*d)
			//((ac + bd) + (bc - ad)i) / (c*c+d*d)
			//(ac + bd) / (c*c+d*d) + (bc - ad) / (c*c + d * d) i

			//real: (ac + bd) / (c*c+d*d)
			//imaginary: (bc - ad) / (c*c + d * d) i

    return vec2( (a.x *  b.x - a.y * b.y) / (b.x * b.x + b.y * b.y), (a.y * b.x - a.x * b.y) / (b.x * b.x + b.y * b.y) );
}

vec2 ComplexSin(vec2 z)
{
    //ref 1: https://proofwiki.org/wiki/Sine_of_Complex_Number
    //sin(a+bi)=sin(a) * cosh(b) + i * cos(a) * sinh(b)

    return vec2(sin(z.x) * cosh(z.y), cos(z.x) * sinh(z.y));
}

vec2 ComplexCos(vec2 z)
{
    //ref 1: https://proofwiki.org/wiki/Sine_of_Complex_Number
    //ref 2: https://en.wikipedia.org/wiki/Sine_and_cosine#Complex_numbers_relationship
    //cos(a+bi)=cos(a) * cosh(b) - i * sin(a) * sinh(b)

    return vec2(cos(z.x) * cosh(z.y), -sin(z.x) * sinh(z.y));
}

vec3 hsv2rgb(vec3 hsv)
{
    //ref https://www.rapidtables.com/convert/color/hsv-to-rgb.html
    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;

    float c = s * v;
    float x = c * (1 - abs(mod(h / 60.0, 2.0) - 1));
    float m = v - c;

    vec3 cx;
    switch(int(h / 60.0))
    {
        case 0:
            cx = vec3(c, x, 0);
            break;
        case 1:
            cx = vec3(x, c, 0);
            break;
        case 2:
            cx = vec3(0, c, x);
            break;
        case 3:
            cx = vec3(0, x, c);
            break;
        case 4:
            cx = vec3(x, 0, c);
            break;
        case 5:
            cx = vec3(c, 0, x);
            break;
    }

    return vec3(cx.x + m, cx.y + m, cx.z + m);
}

vec4 hsva2rgba(vec4 hsva)
{
    vec3 rgb = hsv2rgb(vec3(hsva.x, hsva.y, hsva.z));

    return vec4(rgb.x, rgb.y, rgb.z, hsva.w);
}

void main()
{
    //Newton fractal:
    //next z = z - (P(z) / P'(z))
    //until root or max iterations is reached (no root)

    float tolerance = 0.01;

    //P(z) = sin z
    //=> P'(z) = cos z

    //https://en.wikipedia.org/wiki/Newton_fractal

    vec2 z = ((vec2((fragTexCoord.x + offset.x) / widthStretch, fragTexCoord.y + offset.y)) / zoom) + position;
    
    for (int iteration = 0; iteration < maxIterations; iteration++)
    {
        vec2 rz = ComplexMultiply(a, ComplexDivide(ComplexSin(z), ComplexCos(z)));

        z -= rz;

        //check if we are near any roots
        //if so, set the finalColor and stop iterating
        
        vec2 zsin = ComplexSin(z);
        if (abs(zsin.x) <= tolerance && abs(zsin.y) <= tolerance)
        {
            finalColor = hsva2rgba(vec4(mod(float(iteration) * 5.0, 360.0), 1.0, 1.0, 1.0));
            return;
        }
    }

    //no root found, set finalcolor to black
    finalColor = vec4(0.0, 0.0, 0.0, 1.0);
} 