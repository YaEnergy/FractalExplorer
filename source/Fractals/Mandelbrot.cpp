#include "Fractals/Mandelbrot.h"

#include <cmath>
#include <vector>

#include "raylib.h"

//Color helper function
bool AreColorsEqual(Color a, Color b);

int numLinesDrawnThisFractal = 0;

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
	numLinesDrawnThisFractal = 0;

	//generate fractal iteration palette
	std::vector<Color> paletteColors = std::vector<Color>(maxIterations);

	for (int i = 0; i < maxIterations; i++)
	{
		paletteColors[i] = ColorFromHSV((float)((i * 15) % 360), 1.0f, 1.0f);
	}

	//draw fractal
	//draw line every the color changes instead of every pixel, to minimize draw calls
	for (int y = 0; y <= height; y++)
	{
		int startColorLineX = 0;
		Color colorLine = BLANK;

		for (int x = 0; x <= width; x++)
		{
			int iterations = GetMandelbrotSetComplexDoubleMaxIterations({ ((double)x - (double)width / 2.0) / zoom + positionX, ((double)y - (double)height / 2.0) / zoom + positionY }, maxIterations);

			Color pixelColor = paletteColors[(iterations - 1) % maxIterations];

			//if line color changed, draw the line and start next line
			if (!AreColorsEqual(pixelColor, colorLine))
			{
				if (!AreColorsEqual(colorLine, BLANK))
				{
					//draw color line

					if (x - 1 == startColorLineX)
						DrawPixel(startColorLineX, y, colorLine);
					else
						DrawLine(startColorLineX, y, x, y, colorLine);

					numLinesDrawnThisFractal++;
				}

				colorLine = pixelColor;
				startColorLineX = x;
			}

			//DrawPixel(x, y, pixelColor);
		}

		//Draw remaining color line
		DrawLine(startColorLineX, y, width, y, colorLine);
		numLinesDrawnThisFractal++;
	}
}

//Color helper function
bool AreColorsEqual(Color a, Color b)
{
	//Two colors are equal if all their components are equal
	return a.r == b.r 
		&& a.g == b.g 
		&& a.b == b.b 
		&& a.a == b.a;
}

int GetNumLinesDrawnThisFractal()
{
	return numLinesDrawnThisFractal;
}