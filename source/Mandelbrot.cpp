#include "Mandelbrot.h"
#include <cmath>

//Ref: https://nl.wikipedia.org/wiki/Mandelbrotverzameling , Wiskundige Beschrijving
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