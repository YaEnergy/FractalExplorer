#version 330 core

#define PI 3.1415926535897932384626433

in vec2 fragTexCoord;
in vec4 fragColor;

uniform float widthStretch = 1.0;
uniform vec2 offset = vec2(0.0, 0.0);

uniform vec2 position = vec2(0.0, 0.0);
uniform float zoom = 1.0;
uniform int maxIterations = 20;

//Allows multicorns
uniform float power = 2.0;

uniform int colorBanding = 0;

out vec4 finalColor;

//2-argument arctangent, used to (for example:) get the angle of a complex number
float atan2(float y, float x)
{
    return x > 0 ? atan(y / x) : atan(y / x) + PI;
}

float ComplexAbs(vec2 z)
{
    return sqrt(z.x * z.x + z.y * z.y);
}

float ComplexAbsSquared(vec2 z)
{
    return z.x * z.x + z.y * z.y;
}

vec2 ComplexConjugate(vec2 z)
{
    return vec2(z.x, -z.y);
}

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

//z^power
vec2 ComplexPow(vec2 z, float power)
{
    //ref: https://registry.khronos.org/OpenGL-Refpages/gl4/
    //according to the above ref, pow(x, y) with (x < 0) or (x = 0 & y <= 0) produces undefined.
    //but on certain GPUs, for some reason (x = 0) also causes issues.
    //I find this very strange as, for example: 0 * 0 (or 0^2) would equal 0, right?
    //I'm glad that I found this issue, because this would've gone unnoticed if I didn't have my school computer.
    //this only proved to be an issue for multibrot & multicorn as their z starts at vec2(0.0, 0.0), which meant z.x * z.x + z.y * z.y produced 0
    //the julia fractal however, does not.
    //I'm not completely sure if they produced undefined, but given what was happening it was most likely the case.

    //z.x * z.x + z.y * z.y, will never be less than zero, so only this check is neccessary
    if (z.x * z.x + z.y * z.y == 0.0)
        return vec2(0.0, 0.0);

    return vec2(pow(z.x * z.x + z.y * z.y, power / 2.0) * cos(power * atan2(z.y, z.x)), pow(z.x * z.x + z.y * z.y, power / 2.0) * sin(power * atan2(z.y, z.x)));
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
    //next z = ComplexConjugate(z) * ComplexConjugate(z) + c
    //until magtinude z > escapeRadius or max iterations is reached

     //https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
    float escapeRadius = colorBanding == 1 ? 2.0 : 16.0;

    int complexIterations = 0;
    vec2 c = ((vec2((fragTexCoord.x + offset.x) / widthStretch, fragTexCoord.y + offset.y)) / zoom) + position;
    vec2 z = vec2(0.0, 0.0);

    //if power is a whole number & above 0 (excluding 0), multiply the complex number by itself that many times instead of using ComplexPow, it's less expensive
    if (mod(power, 1.0) == 0.0 && power > 0.0)
    {
        while (ComplexAbsSquared(z) <= escapeRadius * escapeRadius && complexIterations < maxIterations)
        {
            vec2 newZ = ComplexConjugate(z);

            for (int i = 1; i < int(power); i++)
            {
                newZ = ComplexMultiply(newZ, ComplexConjugate(z));
            }

            z = newZ + c;
        
            complexIterations++;
        }
    }
    else
    {
        while (ComplexAbsSquared(z) <= escapeRadius * escapeRadius && complexIterations < maxIterations)
        {
            z = ComplexPow(ComplexConjugate(z), power) + c;
        
            complexIterations++;
        }
    }

    if (complexIterations == maxIterations)
    {
        finalColor = vec4(0.0, 0.0, 0.0, 255.0);
    }
    else
    {
        float nu = colorBanding == 1 ? 1.0 : log(log(ComplexAbsSquared(z)) / 2.0 / log(2.0) ) / log(power);

        finalColor = hsva2rgba(vec4(mod((complexIterations + 1 - nu) * 3.0, 360.0), 1.0, 1.0, 1.0));
    }
} 