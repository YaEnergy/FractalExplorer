#include "Mandelbrot.h"
#include "raylib.h"

#include <vector>
#include <cmath>

//Ref: https://nl.wikipedia.org/wiki/Mandelbrotverzameling , Wiskundige Beschrijving
//Can go out of bounds of mandelbrot set!
ComplexDouble GetMandelbrotSetComplexDouble(ComplexDouble c, int iterations)
{
	ComplexDouble z = { 0.0, 0.0 };
	
	for (int i = 0; i < iterations; i++)
	{
		//see ref
		z = z * z + c;
	}

	return z;
}

//ref: https://en.wikipedia.org/wiki/Mandelbrot_set , computer drawings
int GetMandelbrotSetComplexDoubleMaxIterations(ComplexDouble c, int maxIterations)
{
	ComplexDouble z = { 0.0, 0.0 };
	int iterations = 0;

	while (z.GetMagnitudeSquared() <= 2.0 * 2.0 && iterations < maxIterations)
	{
		//see ref
		z = z * z + c;

		iterations++;
	}

	return iterations;
}

void DrawMandelbrotFractal(int width, int height, double positionX, double positionY, double zoom, int maxIterations)
{
	//generate fractal iteration palette
	std::vector<Color> paletteColors = std::vector<Color>(maxIterations);

	for (int i = 0; i < maxIterations; i++)
	{
		paletteColors[i] = ColorFromHSV((float)((i * 15) % 360), 1.0f, 1.0f);
	}

	//draw fractal
	for (int y = 0; y <= height; y++)
	{
		for (int x = 0; x <= width; x++)
		{
			int iterations = GetMandelbrotSetComplexDoubleMaxIterations({ ((double)x - (double)width / 2.0) / zoom + positionX, ((double)y - (double)height / 2.0) / zoom + positionY }, maxIterations);

			DrawPixel(x, y, paletteColors[(iterations - 1) % maxIterations]);
		}
	}
}