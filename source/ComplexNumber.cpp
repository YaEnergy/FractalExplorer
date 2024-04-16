#include "ComplexNumber.h"
#include <cmath>

double ComplexNumber::GetSquaredDistanceFromOrigin() const
{
	return real * real + imaginary * imaginary;
}

double ComplexNumber::GetDistanceFromOrigin() const
{
	return sqrt(real * real + imaginary * imaginary);
}
