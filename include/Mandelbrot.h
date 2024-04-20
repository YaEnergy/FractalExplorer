#pragma once

#include "ComplexDouble.h"

ComplexDouble GetMandelbrotSetComplexDouble(ComplexDouble c, int iterations);

int GetMandelbrotSetComplexDoubleMaxIterations(ComplexDouble c, int maxIterations);

void DrawMandelbrotFractal(int width, int height, double positionX, double positionY, double zoom, int maxIterations);