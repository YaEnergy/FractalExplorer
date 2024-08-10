#version 330 core

#define PI 3.1415926535897932384626433

#define NUM_ROOTS 4

in vec2 fragTexCoord;
in vec4 fragColor;

uniform float widthStretch = 1.0;

uniform int maxIterations = 20;

uniform vec2 position = vec2(0.0, 0.0);
uniform vec2 offset = vec2(0.0, 0.0);

uniform float zoom = 1.0;

uniform int colorBanding = 0;

//Default roots are the roots to P(z) = z^4 - 1
uniform vec2[NUM_ROOTS] roots = vec2[NUM_ROOTS]
(
    vec2(1.0, 0.0), 
    vec2(-1.0, 0.0), 
    vec2(0.0, 1.0),
    vec2(0.0, -1.0)
);

const vec4[NUM_ROOTS] ROOT_COLORS = vec4[NUM_ROOTS]
(
    vec4(1.0, 0.2, 0.2, 1.0), // RED
    vec4(0.2, 1.0, 0.2, 1.0), // GREEN
    vec4(0.2, 0.2, 1.0, 1.0), // BLUE
    vec4(1.0, 1.0, 0.2, 1.0)  // YELLOW
);

uniform vec2 a = vec2(1.0, 0.0);

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

//z = a + b, add a and b together instead
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

//z^power
vec2 ComplexPow(vec2 z, float power)
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

//Fourth-degree polynomial function for complex numbers (az^4 + bz^3 + cz^2 + dz + e)
vec2 FourthDegreePolynomial(vec2 z, vec2 a, vec2 b, vec2 c, vec2 d, vec2 e)
{
    //P(z) = az^4 + bz^3 + cz^2 + dz + e (fourth-degree polynomial, due to having 4 roots)

    //az^4
    vec2 fourthDegree = ComplexMultiply(a, ComplexMultiply(ComplexMultiply(ComplexMultiply(z, z), z), z));

    //bz^3
    vec2 thirdDegree = ComplexMultiply(b, ComplexMultiply(ComplexMultiply(z, z), z));

    //cz^2
    vec2 secondDegree = ComplexMultiply(c, ComplexMultiply(z, z));

    //dz
    vec2 firstDegree = ComplexMultiply(d, z);

    //add together with constant e
    return fourthDegree + thirdDegree + secondDegree + firstDegree + e;
}

//Derivative of a fourth-degree polynomial function (4az^3 + 3bz^2 + 2cz + d)
vec2 FourthDegreePolynomialDerivative(vec2 z, vec2 a, vec2 b, vec2 c, vec2 d)
{
    //P(z) = az^4 + bz^3 + cz^2 + dz + e (fourth-degree polynomial, due to having 4 roots)
    //Power rule => P'(z) = 4az^3 + 3bz^2 + 2cz + d

    //4az^3
    vec2 thirdDegree = ComplexMultiply(4 * a, ComplexMultiply(ComplexMultiply(z, z), z));

    //3bz^2
    vec2 secondDegree = ComplexMultiply(3 * b, ComplexMultiply(z, z));

    //2cz
    vec2 firstDegree = ComplexMultiply(2 * c, z);

    //add together with constant d
    return thirdDegree + secondDegree + firstDegree + d;
}


void main()
{
    //Newton fractal:
    //next z = z - (P(z) / P'(z))
    //until root or max iterations is reached (no root)

    //P(z) = az^4 + bz^3 + cz^2 + dz + e (fourth-degree polynomial, due to having 4 roots)
    //Power rule => P'(z) = 4az^3 + 3bz^2 + 2cz + d

    //worked this out on paper
    //tried to avoid as many complex number multiplications as I could, as they're more expensive
    //and this is quite the expensive shader

    //P(z) = x^4 - (a + b + c + d)x^3 + (cb + ac + ab + ad + db + cd)x^2 - (dcb + acd + abd + abc)x + abcd
    //P(z) = x^4 - (a + b + c + d)x^3 + (a(b+c+d) + b(d+c) + cd)x^2 - (cd(a+b) + ab(c+d))x + abcd

    //https://en.wikipedia.org/wiki/Polynomial#Calculus thanks Wikipedia
    //https://en.wikipedia.org/wiki/Power_rule 

    //due to floating imprecision, we might not perfectly land at a root
    float tolerance = 0.35;//0.000001; //TODO: tweak 0.35
    //it seems this I suck at this

    //fourth degree factor: 1
    vec2 fourthDegreeFactor = vec2(1.0, 0.0);

    //third degree factor: - (a + b + c + d) = -a - b - c - d
    vec2 thirdDegreeFactor = -roots[0] - roots[1] - roots[2] - roots[3];

    //second degree factor: (cb + ac + ab + ad + db + cd) = (a(b+c+d) + b(d+c) + cd)
    vec2 secondDegreeFactor = ComplexMultiply(roots[0], roots[1] + roots[2] + roots[3]) + ComplexMultiply(roots[1], roots[2] + roots[3]) + ComplexMultiply(roots[2], roots[3]);

    //first degree factor: - (dcb + acd + abd + abc) = -cd(a+b) - ab(c+d)
    vec2 firstDegreeFactor = -ComplexMultiply(ComplexMultiply(roots[2], roots[3]), roots[0] + roots[1]) - ComplexMultiply(ComplexMultiply(roots[0], roots[1]), roots[2] + roots[3]);

    //constant: abcd
    vec2 constant = ComplexMultiply(ComplexMultiply(ComplexMultiply(roots[0], roots[1]), roots[2]), roots[3]);

    //https://en.wikipedia.org/wiki/Newton_fractal

    vec2 z = ((vec2((fragTexCoord.x + offset.x) / widthStretch, fragTexCoord.y + offset.y)) / zoom) + position;

    bool rootFound = false;

    for (int iteration = 0; iteration < maxIterations; iteration++)
    {
        z -= ComplexMultiply(a, ComplexDivide(
            FourthDegreePolynomial(z, fourthDegreeFactor, thirdDegreeFactor, secondDegreeFactor, firstDegreeFactor, constant), 
            FourthDegreePolynomialDerivative(z, fourthDegreeFactor, thirdDegreeFactor, secondDegreeFactor, firstDegreeFactor)
            ));

        //check if we are near any roots
        //if found, break and set the finalColor to that root's color
        for (int i = 0; i < NUM_ROOTS; i++)
        {
            vec2 dif = z - roots[i];

            //if root found within tolerance range
            if (abs(dif.x) < tolerance && abs(dif.y) < tolerance)
            {
                float ab = (float(iteration) / float(maxIterations)) * 0.6 + 0.4;
                finalColor = ROOT_COLORS[i];//vec4(vec3(ab), 1.0);//vec4(ROOT_COLORS[i].x * ab, ROOT_COLORS[i].y * ab, ROOT_COLORS[i].z * ab, ROOT_COLORS[i].w);
                rootFound = true;
                break;
            }
        }

        if (rootFound)
            break;
    }

    //no root found
    if (!rootFound)
    {
        //finalColor = vec4(0.0, 0.0, 0.0, 1.0);

        //get closest root rn
        float minDistanceSquared = 0.0;
        int rootIndex = -1;

        for (int i = 0; i < NUM_ROOTS; i++)
        {
            vec2 dif = z - roots[i];

            if (dif.x * dif.x + dif.y * dif.y <= minDistanceSquared || rootIndex == -1)
            {
                minDistanceSquared = dif.x * dif.x + dif.y * dif.y;
                rootIndex = i;
            }
        }

        finalColor = ROOT_COLORS[rootIndex] * 0.8;
    }
} 