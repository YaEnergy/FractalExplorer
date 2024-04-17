#include "Mandelbrot.h"
#include "raylib.h"

#include <vector>
#include <cmath>

//Ref: https://nl.wikipedia.org/wiki/Mandelbrotverzameling , Wiskundige Beschrijving
//Can go out of bounds of mandelbrot set!
ComplexNumber GetMandelbrotSetComplexNumber(ComplexNumber c, int iterations)
{
	ComplexNumber z = { 0.0, 0.0 };
	
	for (int i = 0; i < iterations; i++)
	{
		//see ref
		//c = a + bi
		z = { z.real * z.real - z.imaginary * z.imaginary + c.real,  2 * z.real * z.imaginary + c.imaginary };
	}

	return z;
}

//ref: https://en.wikipedia.org/wiki/Mandelbrot_set , computer drawings
int GetMandelbrotSetComplexNumberMaxIterations(ComplexNumber c, int maxIterations)
{
	ComplexNumber z = { 0.0, 0.0 };
	int iterations = 0;

	while (z.real * z.real + z.imaginary * z.imaginary <= 2.0 * 2.0 && iterations < maxIterations)
	{
		//see ref
		//c = a + bi
		z = { z.real * z.real - z.imaginary * z.imaginary + c.real,  2 * z.real * z.imaginary + c.imaginary };
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
		paletteColors[i] = ColorFromHSV((i * 15) % 360, 1.0f, 1.0f);
	}

	//draw fractal
	for (int y = 0; y <= height; y++)
	{
		for (int x = 0; x <= width; x++)
		{
			int iterations = GetMandelbrotSetComplexNumberMaxIterations({ ((double)x - (double)width / 2.0) / zoom + positionX, ((double)y - (double)height / 2.0) / zoom + positionY }, maxIterations);

			DrawPixel(x, y, paletteColors[(iterations - 1) % maxIterations]);
		}
	}
}