#version 100
precision highp float;

#define PI 3.1415926535897932384626433
#define E 2.71828182845904523536 //euler's number

//Hard-coded limit
const int LIMIT_ITERATIONS = 300;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform float widthStretch;

uniform int maxIterations;

uniform vec2 position;
uniform vec2 offset;

uniform float zoom;

uniform int colorBanding;

uniform vec2 a;

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

//not implemented as a global function in glsl v100
float sinh(float x)
{
    //ref: https://www.mathcentre.ac.uk/resources/workbooks/mathcentre/hyperbolicfunctions.pdf
    return (pow(E, x) - pow(E, -x)) / 2.0;
}

//not implemented as a global function in glsl v100
float cosh(float x)
{
    //ref: https://www.mathcentre.ac.uk/resources/workbooks/mathcentre/hyperbolicfunctions.pdf
    return (pow(E, x) + pow(E, -x)) / 2.0;
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
    //ref https://www.rapidtables.com/convert/color/hsv-to-rgb.html & https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB

    float hue = hsv.x;
    float saturation = hsv.y;
    float value = hsv.z;

    float hueAccent = hue / 60.0;

    float c = saturation * value;
    float x = c * (1.0 - abs(mod(hueAccent, 2.0) - 1.0));

    int hueAccentFloor = int(hueAccent);
    float m = value - c;

    vec3 color = vec3(m, m, m);

    //red
    if (hueAccentFloor == 0 || hueAccentFloor == 5)
        color.x += c;
    else if (hueAccentFloor == 1 || hueAccentFloor == 4)
        color.x += x;

    //green
    if (hueAccentFloor == 0 || hueAccentFloor == 3)
        color.y += x;
    else if (hueAccentFloor == 1 || hueAccentFloor == 2)
        color.y += c;

    //blue
    if (hueAccentFloor == 2 || hueAccentFloor == 5)
        color.z += x;
    else if (hueAccentFloor == 3 || hueAccentFloor == 4)
        color.z += c;

    return color;
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

    for (int i = 0; i < LIMIT_ITERATIONS; i++)
    {
        if (i >= maxIterations)
            break;

        vec2 rz = ComplexMultiply(a, ComplexDivide(ComplexSin(z), ComplexCos(z)));

        z -= rz;

        //check if we are near any roots
        //if so, set the finalColor and stop iterating
        
        vec2 zsin = ComplexSin(z);
        if (abs(zsin.x) <= tolerance && abs(zsin.y) <= tolerance)
        {
            gl_FragColor = hsva2rgba(vec4(mod(float(i) * 5.0, 360.0), 1.0, 1.0, 1.0));
            return;
        }
    }
    
    //no root found, set finalcolor to black
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
} 