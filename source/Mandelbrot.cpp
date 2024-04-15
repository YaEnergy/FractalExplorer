#include "Mandelbrot.h"
#include "raymath.h"

//Ref: https://nl.wikipedia.org/wiki/Mandelbrotverzameling , Wiskundige Beschrijving
ComplexNumber GetNextComplexNumber(ComplexNumber z, ComplexNumber c)
{
	//c = a + bi
	return ComplexNumber(z.real * z.real - z.imaginary * z.imaginary + c.real,  2 * z.real * z.imaginary + c.imaginary);
}

//Ref: https://nl.wikipedia.org/wiki/Mandelbrotverzameling , Wiskundige Beschrijving
ComplexNumber GetFractalComplexNumber(ComplexNumber c, int iterations)
{
	ComplexNumber complexNumber = { 0.0, 0.0 };
	
	for (int i = 0; i < iterations; i++)
	{
		complexNumber = GetNextComplexNumber(complexNumber, c);
	}

	return complexNumber;
}

double GetComplexNumberDistance(ComplexNumber z)
{
	return sqrt(z.real * z.real + z.imaginary * z.imaginary);
}
