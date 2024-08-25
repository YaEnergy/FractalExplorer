#include <cmath>

#include "raymath.h"

#include "ComplexNumbers/ComplexDouble.h"

double ComplexDouble::GetMagnitudeSquared() const
{
	return real * real + imaginary * imaginary;
}

double ComplexDouble::GetMagnitude() const
{
	return sqrt(real * real + imaginary * imaginary);
}

double ComplexDouble::GetAngleRadians() const
{
	return real > 0 ? atan(imaginary / real) : atan(imaginary / real) + 180.0 * (double)DEG2RAD;
}

ComplexDouble ComplexDouble::FromPolarForm(double magnitude, double angleRadians)
{
	return ComplexDouble{ magnitude * cos(angleRadians), magnitude * sin(angleRadians) };
}