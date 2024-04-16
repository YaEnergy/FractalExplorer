#include "ComplexNumber.h"
#include <cmath>

double ComplexNumber::GetDistanceFromOrigin() const
{
	return sqrt(real * real + imaginary * imaginary);
}
