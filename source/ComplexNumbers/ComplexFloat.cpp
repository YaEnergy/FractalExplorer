#include <cmath>

#include "raylib.h"
#include "raymath.h"

#include "ComplexNumbers/ComplexFloat.h"

float ComplexFloat::GetMagnitudeSquared() const
{
	return real * real + imaginary * imaginary;
}

float ComplexFloat::GetMagnitude() const
{
	return sqrt(real * real + imaginary * imaginary);
}

float ComplexFloat::GetAngleRadians() const
{
	return real > 0 ? atan(imaginary / real) : atan(imaginary / real) + 180.0f * DEG2RAD;
}

ComplexFloat ComplexFloat::FromPolarForm(float magnitude, float angleRadians)
{
	return ComplexFloat{ magnitude * cos(angleRadians), magnitude * sin(angleRadians) };
}

Vector2 ComplexFloat::ToVector2() const
{
	return Vector2{ real, imaginary };
}