#pragma once

#include "ComplexNumber.h"

ComplexNumber GetNextComplexNumber(ComplexNumber z, ComplexNumber c);

ComplexNumber GetFractalComplexNumber(ComplexNumber c, int iterations);

double GetComplexNumberDistance(ComplexNumber z);