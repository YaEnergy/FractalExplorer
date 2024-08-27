#version 100
precision highp float;

#define PI 3.1415926535897932384626433

//Hard-coded limit
const int LIMIT_ITERATIONS = 300;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform float widthStretch;

uniform vec2 position;
uniform vec2 offset;
uniform float zoom;
uniform int maxIterations;

uniform int colorBanding;

float ComplexAbs(vec2 z)
{
    return sqrt(z.x * z.x + z.y * z.y);
}

float ComplexAbsSquared(vec2 z)
{
    return z.x * z.x + z.y * z.y;
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

vec3 hsv2rgb(vec3 hsv)
{
    //ref https://www.rapidtables.com/convert/color/hsv-to-rgb.html
    float h = hsv.x;
    float s = hsv.y;
    float v = hsv.z;

    float c = s * v;
    float x = c * (1.0 - abs(mod(h / 60.0, 2.0) - 1.0));
    float m = v - c;

    vec3 cx;
    int hInt = int(h / 60.0);

    if (hInt == 0)
        cx = vec3(c, x, 0);
    else if (hInt == 1)
        cx = vec3(x, c, 0);
    else if (hInt == 2)
        cx = vec3(0, c, x);
    else if (hInt == 3)
        cx = vec3(0, x, c);
    else if (hInt == 4)
        cx = vec3(x, 0, c);
    else if (hInt == 5)
        cx = vec3(c, 0, x);

    return vec3(cx.x + m, cx.y + m, cx.z + m);
}

vec4 hsva2rgba(vec4 hsva)
{
    vec3 rgb = hsv2rgb(vec3(hsva.x, hsva.y, hsva.z));

    return vec4(rgb.x, rgb.y, rgb.z, hsva.w);
}

void main()
{
    //next z = (abs(z.x) + i * abs(z.y)) * (abs(z.x) + i * abs(z.y)) + c
    //until magtinude z > escapeRadius or max iterations is reached

     //https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring
    float escapeRadius = colorBanding == 1 ? 2.0 : 16.0;

    vec2 c = ((vec2((fragTexCoord.x + offset.x) / widthStretch, fragTexCoord.y + offset.y)) / zoom) + position;
    vec2 z = vec2(0.0, 0.0);

    int iter = 0;
    for (int i = 0; i < LIMIT_ITERATIONS; i++)
    {
        iter = i;

        if (ComplexAbsSquared(z) > escapeRadius * escapeRadius)
            break;

        if (i >= maxIterations)
            break;

        vec2 shipZ = vec2(abs(z.x), abs(z.y));
        z = ComplexAdd(ComplexMultiply(shipZ, shipZ), c);
    }

    if (ComplexAbsSquared(z) <= escapeRadius * escapeRadius)
        gl_FragColor = vec4(0.0, 0.0, 0.0, 255.0);
    else
    {
        float nu = colorBanding == 1 ? 1.0 : log(log(ComplexAbsSquared(z)) / 2.0 / log(2.0) ) / log(2.0);

        gl_FragColor = hsva2rgba(vec4(mod((float(iter) + 1.0 - nu) * 3.0, 360.0), 1.0, 1.0, 1.0));
    }
} 