#version 100
precision highp float;

#define PI 3.1415926535897932384626433

//Hard-coded limit
const int LIMIT_ITERATIONS = 300;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform float widthStretch;

uniform float power;
uniform int maxIterations;

uniform vec2 position;
uniform vec2 offset;

uniform float zoom;

uniform int colorBanding;

//2-argument arctangent, used to (for example:) get the angle of a complex number
float atan2(float y, float x)
{
    return x > 0.0 ? atan(y / x) : atan(y / x) + PI;
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

//z = a + b
vec2 ComplexAdd(vec2 a, vec2 b)
{
    return vec2(a.x + b.x, a.y + b.y);
}

//z = a * b
vec2 ComplexMultiply(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

//z^power
vec2 ComplexPow(vec2 z, float power)
{
    return vec2(pow(z.x * z.x + z.y * z.y, power / 2.0) * cos(power * atan2(z.y, z.x)), pow(z.x * z.x + z.y * z.y, power / 2.0) * sin(power * atan2(z.y, z.x)));
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
    //next z = z ^ n + c
    //until magtinude z > escapeRadius or max iterations is reached

    //https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
    float escapeRadius = colorBanding == 1 ? 2.0 : 16.0;

    vec2 c = ((vec2((fragTexCoord.x + offset.x) / widthStretch, fragTexCoord.y + offset.y)) / zoom) + position;
    vec2 z = vec2(0.0, 0.0);

    for (int i = 0; i < LIMIT_ITERATIONS; i++)
    {
        if (i >= maxIterations)
            break;

        //Why no power for loops for whole exponents (integer power value):
        //  for loops can only use constant values, so we can't use the power value converted into an integer in our loop.
        //  for perfomance, power 1, 2, 3, 4 & 5 are the only exceptions where ComplexMultiply will be applied multiple times instead of ComplexPow
        //  if this was v330, we could use for loops instead for every power value that is an whole number
        //
        //I know this looks bad, but it's better than just using ComplexPow

        vec2 conjZ = ComplexConjugate(z);

        if (power == 1.0)
            z = conjZ + c;
        else if (power == 2.0)
            z = ComplexMultiply(conjZ, conjZ) + c;
        else if (power == 3.0)
            z = ComplexMultiply(ComplexMultiply(conjZ, conjZ), conjZ) + c;
        else if (power == 4.0)
            z = ComplexMultiply(ComplexMultiply(ComplexMultiply(conjZ, conjZ), conjZ), conjZ) + c;
        else if (power == 5.0)
            z = ComplexMultiply(ComplexMultiply(ComplexMultiply(ComplexMultiply(conjZ, conjZ), conjZ), conjZ), conjZ) + c;
        else
            z = ComplexPow(conjZ, power) + c;

        if (ComplexAbsSquared(z) > escapeRadius * escapeRadius)
        {
            float nu = colorBanding == 1 ? 1.0 : log(log(ComplexAbsSquared(z)) / 2.0 / log(2.0) ) / log(power);

            gl_FragColor = hsva2rgba(vec4(mod((float(i) + 1.0 - nu) * 3.0, 360.0), 1.0, 1.0, 1.0));
            return;
        }
    }
    
    gl_FragColor = vec4(0.0, 0.0, 0.0, 255.0);
} 