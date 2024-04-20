#include "ComplexDouble.h"
#include <cmath>

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
	return atan(imaginary / real);
}

ComplexDouble ComplexDouble::FromPolarForm(double magnitude, double angleRadians)
{
	return { magnitude * cos(angleRadians), magnitude * sin(angleRadians) };
}