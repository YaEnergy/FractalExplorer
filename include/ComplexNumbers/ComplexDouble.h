#pragma once

//A complex number using doubles: real + imaginary * i, with i^2 = -1
struct ComplexDouble
{
	double real;
	double imaginary;

	ComplexDouble(double real, double imaginary)
	{
		this->real = real;
		this->imaginary = imaginary;
	}

	public:
		double GetMagnitude() const;
		double GetMagnitudeSquared() const;
		double GetAngleRadians() const;

		static ComplexDouble FromPolarForm(double magnitude, double angleRadians);

		ComplexDouble operator+(ComplexDouble const& obj)
		{
			return { real + obj.real, imaginary + obj.imaginary };
		}

		ComplexDouble operator-(ComplexDouble const& obj)
		{
			return { real - obj.real, imaginary - obj.imaginary };
		}

		ComplexDouble operator*(ComplexDouble const& obj)
		{
			//z1 * z2
			//(a + bi) * (c + di)
			//(a*c) + (a * di) + (bi * c) + (bi * di)
			//=> (a*c) + (a*d)i + (b*c)i - (b*d) (i * i = -1)
			
			//real: a*c - b*d
			//imaginary: (a*d + b*c)i

			//womp womp

			return { real * obj.real - imaginary * obj.imaginary, real * obj.imaginary + imaginary * obj.real };
		}

		ComplexDouble operator/(ComplexDouble const& obj)
		{
			//oh dear lord...
			
			//ref: Delta 4B, p. 133
			//Sorry, I didn't feel like working this out on my own
			
			//z1 / z2
			//(a + bi) / (c + di)
			//(a + bi) / (c + di) * ((c - di) / (c - di))
			//(ac - adi + bco - bdi*i)/(c*c-d*d*i*i)
			//(ac + bd - adi + bci)/(c*c+d*d)
			//((ac + bd) + (bc - ad)i) / (c*c+d*d)
			//(ac + bd) / (c*c+d*d) + (bc - ad) / (c*c + d * d) i

			//real: (ac + bd) / (c*c+d*d)
			//imaginary: (bc - ad) / (c*c + d * d) i

			return { (real *  obj.real - imaginary * obj.imaginary) / (obj.real * obj.real + obj.imaginary * obj.imaginary), (imaginary * obj.real - real * obj.imaginary) / (obj.real * obj.real + obj.imaginary * obj.imaginary) };
		}

		//Negate
		ComplexDouble operator-()
		{
			return ComplexDouble{ -real, -imaginary };
		}
};