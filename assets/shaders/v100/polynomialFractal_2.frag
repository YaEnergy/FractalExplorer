#version 100
precision highp float;

#define PI 3.1415926535897932384626433

#define NUM_ROOTS 2

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

uniform vec2 roots[NUM_ROOTS];

float ComplexAbs(vec2 z)
{
    return sqrt(z.x * z.x + z.y * z.y);
}

float ComplexAbsSquared(vec2 z)
{
    return z.x * z.x + z.y * z.y;
}

//z = a * b
vec2 ComplexMultiply(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

//Second-degree polynomial function for complex numbers (az^3 + az^2 + bz + c)
vec2 SecondDegreePolynomial(vec2 z, vec2 a, vec2 b, vec2 c)
{
    //P(z) = az^2 + bz + c (second-degree polynomial, due to having 2 roots);

    //az^2
    vec2 secondDegree = ComplexMultiply(a, ComplexMultiply(z, z));

    //bz
    vec2 firstDegree = ComplexMultiply(b, z);

    //add together with constant c
    return secondDegree + firstDegree + c;
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

    //(z-a)(z-b) = (z^2-bz-az+ab) = (z^2-(b+a)z+ab)

    //second degree factor: 1
    vec2 secondDegreeFactor = vec2(1.0, 0.0);

    //first degree factor: -(b+a) = -b-a
    vec2 firstDegreeFactor = -roots[1] - roots[0];

    //constant: ab
    vec2 constant = ComplexMultiply(roots[0], roots[1]);

    float power = 2.0;

    for (int i = 0; i < LIMIT_ITERATIONS; i++)
    {
        if (i >= maxIterations)
            break;

        z = SecondDegreePolynomial(z, secondDegreeFactor, firstDegreeFactor, constant) + c;

        if (ComplexAbsSquared(z) > escapeRadius * escapeRadius)
        {
            float nu = colorBanding == 1 ? 1.0 : log(log(ComplexAbsSquared(z)) / 2.0 / log(2.0) ) / log(power);

            gl_FragColor = hsva2rgba(vec4(mod((float(i) + 1.0 - nu) * 3.0, 360.0), 1.0, 1.0, 1.0));
            return;
        }
    }
    
    gl_FragColor = vec4(0.0, 0.0, 0.0, 255.0);
} 