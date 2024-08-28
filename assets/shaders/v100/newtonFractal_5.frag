#version 100
precision highp float;

#define PI 3.1415926535897932384626433

#define NUM_ROOTS 5

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

uniform vec2 a;

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

//Fifth-degree polynomial function for complex numbers (az^5 + bz^4 + cz^3 + dz^2 + ez + f)
vec2 FifthDegreePolynomial(vec2 z, vec2 a, vec2 b, vec2 c, vec2 d, vec2 e, vec2 f)
{
    //P(z) = az^5 + bz^4 + cz^3 + dz^2 + ez + f (fourth-degree polynomial, due to having 5 roots)

    //az^5
    vec2 fifthDegree = ComplexMultiply(a, ComplexMultiply(ComplexMultiply(ComplexMultiply(ComplexMultiply(z, z), z), z), z));

    //bz^4
    vec2 fourthDegree = ComplexMultiply(b, ComplexMultiply(ComplexMultiply(ComplexMultiply(z, z), z), z));

    //cz^3
    vec2 thirdDegree = ComplexMultiply(c, ComplexMultiply(ComplexMultiply(z, z), z));

    //dz^2
    vec2 secondDegree = ComplexMultiply(d, ComplexMultiply(z, z));

    //ez
    vec2 firstDegree = ComplexMultiply(e, z);

    //add together with constant f
    return fifthDegree + fourthDegree + thirdDegree + secondDegree + firstDegree + f;
}

//Derivative of a fifth-degree polynomial function (5az^4 + 4bz^3 + 3cz^2 + 2dz + e)
vec2 FifthDegreePolynomialDerivative(vec2 z, vec2 a, vec2 b, vec2 c, vec2 d, vec2 e)
{
    //P(z) = az^5 + bz^4 + cz^3 + dz^2 + ez + f (fifth-degree polynomial, due to having 5 roots)
    //Power rule => P'(z) = 5az^4 + 4bz^3 + 3cz^2 + 2dz + e

    //5az^4
    vec2 fourthDegree = ComplexMultiply(5.0 * a, ComplexMultiply(ComplexMultiply(ComplexMultiply(z, z), z), z));

    //4bz^3
    vec2 thirdDegree = ComplexMultiply(4.0 * b, ComplexMultiply(ComplexMultiply(z, z), z));

    //3cz^2
    vec2 secondDegree = ComplexMultiply(3.0 * c, ComplexMultiply(z, z));

    //2dz
    vec2 firstDegree = ComplexMultiply(2.0 * d, z);

    //add together with constant e
    return fourthDegree + thirdDegree + secondDegree + firstDegree + e;
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

    //P(z) = az^5 + bz^4 + cz^3 + dz^2 + ez + f (fifth-degree polynomial, due to having 5 roots)
    //Power rule => P'(z) = 5az^4 + 4bz^3 + 3cz^2 + 2dz + e

    //worked this out on paper
    //tried to avoid as many complex number multiplications as I could, as they're more expensive
    //and this is quite the expensive shader

    //P(z) = x^5 - (a + b + c + d + e)x^4 + (a(b+c+d+e) + b(c+d+e) + c(d+e) + de)x^3 - (c*d*(b+a+e) + a*b*(c+d+e) + b*e*(c+d) + a*e*(c + d))x^2 + (cde(a+b) + ab(de + c(d+e)))x - abcde

    //https://en.wikipedia.org/wiki/Polynomial#Calculus thanks Wikipedia
    //https://en.wikipedia.org/wiki/Power_rule 

    //due to floating imprecision, we might not perfectly land at a root
    float tolerance = 0.35;
    //it seems this I suck at this

    //fifth degree factor: 1
    vec2 fifthDegreeFactor = vec2(1.0, 0.0);

    //fourth degree factor: - (a + b + c + d + e) = -a - b - c - d - e
    vec2 fourthDegreeFactor = -roots[0] - roots[1] - roots[2] - roots[3] - roots[4];

    //third degree factor: (a(b+c+d+e) + b(c+d+e) + c(d+e) + de)
    vec2 thirdDegreeFactor = ComplexMultiply(roots[0], roots[1] + roots[2] + roots[3] + roots[4]) + ComplexMultiply(roots[1], roots[2] + roots[3] + roots[4]) + ComplexMultiply(roots[2], roots[3] + roots[4]) + ComplexMultiply(roots[3], roots[4]);

    //second degree factor: -c*d*(b+a+e) - a*b*(c+d+e) - b*e*(c+d) - a*e*(c + d)
    vec2 secondDegreeFactor = -ComplexMultiply(ComplexMultiply(roots[2], roots[3]), roots[0] + roots[1] + roots[4]) - ComplexMultiply(ComplexMultiply(roots[0], roots[1]), roots[2] + roots[3] + roots[4]) - ComplexMultiply(ComplexMultiply(roots[1], roots[4]), roots[2] + roots[3]) - ComplexMultiply(ComplexMultiply(roots[0], roots[4]), roots[2] + roots[3]);

    //first degree factor: cde(a+b) + ab(de + c(d+e))
    vec2 firstDegreeFactor = ComplexMultiply(ComplexMultiply(ComplexMultiply(roots[2], roots[3]), roots[4]), roots[0] + roots[1]) + ComplexMultiply(ComplexMultiply(roots[0], roots[1]), ComplexMultiply(roots[3], roots[4]) + ComplexMultiply(roots[2], roots[3] + roots[4]));

    //constant: -abcde
    vec2 constant = -ComplexMultiply(ComplexMultiply(ComplexMultiply(ComplexMultiply(roots[0], roots[1]), roots[2]), roots[3]), roots[4]);

    //https://en.wikipedia.org/wiki/Newton_fractal

    vec2 z = ((vec2((fragTexCoord.x + offset.x) / widthStretch, fragTexCoord.y + offset.y)) / zoom) + position;

    for (int i = 0; i < LIMIT_ITERATIONS; i++)
    {
        if (i >= maxIterations)
            break;

         vec2 rz = ComplexMultiply(a, ComplexDivide(
            FifthDegreePolynomial(z, fifthDegreeFactor, fourthDegreeFactor, thirdDegreeFactor, secondDegreeFactor, firstDegreeFactor, constant), 
            FifthDegreePolynomialDerivative(z, fifthDegreeFactor, fourthDegreeFactor, thirdDegreeFactor, secondDegreeFactor, firstDegreeFactor)
            ));

        z -= rz;

        //check if we are near any roots, we can check this by checking if we the distance that we'll be moving is smaller than our tolerance
        //if so, set the finalColor to a root's color thats within tolerance range and return if we find it
        if (abs(rz.x) <= tolerance && abs(rz.y) <= tolerance)
        {
            for (int i = 0; i < NUM_ROOTS; i++)
            {
                vec2 dif = roots[i] - z;
                if (abs(dif.x) <= tolerance && abs(dif.y) <= tolerance)
                {
                    gl_FragColor = hsva2rgba(vec4(float(i) * (360.0 / float(NUM_ROOTS)), 1.0, 1.0, 1.0));
                    return;
                }
            }
        }
    }
    
    //no root found, set finalcolor to black
    gl_FragColor = vec4(0.0, 0.0, 0.0, 255.0);
} 