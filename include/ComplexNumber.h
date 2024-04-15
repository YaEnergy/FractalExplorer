#pragma once

//A complex number: x + y * i, with i^2 = -1
struct ComplexNumber
{
	double real;
	double imaginary;

	ComplexNumber(double real, double imaginary)
	{
		this->real = real;
		this->imaginary = imaginary;
	}
};