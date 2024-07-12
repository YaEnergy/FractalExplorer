#version 330 core

#define PI 3.1415926535897932384626433

in vec2 fragTexCoord;
in vec4 fragColor;

uniform float widthStretch = 1.0;

uniform vec2 c = vec2(0.5, 0.2);
uniform float power = 2.0;
uniform int maxIterations = 20;

uniform vec2 position = vec2(0.0, 0.0);
uniform vec2 offset = vec2(0.0, 0.0);

uniform float zoom = 1.0;

out vec4 finalColor;

//2-argument arctangent, used to (for example:) get the angle of a complex number
float atan2(float y, float x)
{
    return x > 0 ? atan(y / x) : atan(y / x) + PI;
}

vec2 AddComplexNumbers(vec2 a, vec2 b)
{
    return vec2(a.x + b.x, a.y + b.y);
}

vec2 MultiplyComplexNumbers(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

vec2 PowComplexNumber(vec2 z, float power)
{
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
    //next z = z * z + c
    //until magtinude z >= 2 or max iterations is reached

    int complexIterations = 0;
    vec2 z = ((vec2((fragTexCoord.x + offset.x) / widthStretch, fragTexCoord.y + offset.y)) / zoom) + position;

    while (z.x * z.x + z.y * z.y <= 2.0 * 2.0 && complexIterations <= maxIterations)
    {
        z = AddComplexNumbers(PowComplexNumber(z, power), c);
        
        complexIterations++;
    }

    finalColor = hsva2rgba(vec4(mod(complexIterations * 15.0, 360), 1.0, 1.0, 1.0));
} 