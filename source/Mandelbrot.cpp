#include "Mandelbrot.h"
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