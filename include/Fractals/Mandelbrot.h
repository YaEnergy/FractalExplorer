#pragma once

#include "ComplexNumbers/ComplexDouble.h"

//CPU mandelbrot drawing functions

ComplexDouble GetMandelbrotSetComplexDouble(ComplexDouble c, int iterations);

int GetMandelbrotSetComplexDoubleMaxIterations(ComplexDouble c, int maxIterations);

void DrawMandelbrotFractal(int width, int height, double positionX, double positionY, double zoom, int maxIterations);

int GetNumLinesDrawnThisFractal();