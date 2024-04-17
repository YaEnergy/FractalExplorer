#pragma once

#include "ComplexNumber.h"

ComplexNumber GetMandelbrotSetComplexNumber(ComplexNumber c, int iterations);

int GetMandelbrotSetComplexNumberMaxIterations(ComplexNumber c, int maxIterations);

void DrawMandelbrotFractal(int width, int height, double positionX, double positionY, double zoom, int maxIterations);