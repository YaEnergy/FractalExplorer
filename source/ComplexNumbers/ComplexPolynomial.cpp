#include "ComplexNumbers/ComplexPolynomial.h"

#include <string>

#include "ComplexNumbers/ComplexFloat.h"
#include "raylib.h"

namespace Explorer
{
	namespace ComplexPolynomial
	{
		std::string GetDegreeTwoFromRoots(ComplexFloat a, ComplexFloat b)
		{
			ComplexFloat firstDegreeFactor = -b - a;
			ComplexFloat constant = a * b;

			return std::string(TextFormat("P(z) ~= z^2 + (%.02g%+.02gi)z + (%.02g%+.02gi)", firstDegreeFactor.real, firstDegreeFactor.imaginary, constant.real, constant.imaginary));
		}

		std::string GetDegreeThreeFromRoots(ComplexFloat a, ComplexFloat b, ComplexFloat c)
		{
			ComplexFloat secondDegreeFactor = -a - b - c;
			ComplexFloat firstDegreeFactor = (a * b) + (b * c) + (a * c);
			ComplexFloat constant = -(a * b * c);

			return std::string(TextFormat("P(z) ~= z^3 + (%.02g%+.02gi)z^2 + (%.02g%+.02gi)z + (%.02g%+.02gi)", secondDegreeFactor.real, secondDegreeFactor.imaginary, firstDegreeFactor.real, firstDegreeFactor.imaginary, constant.real, constant.imaginary));
		}

		std::string GetDegreeFourFromRoots(ComplexFloat a, ComplexFloat b, ComplexFloat c, ComplexFloat d)
		{
			ComplexFloat thirdDegreeFactor = -a - b - c - d;
			ComplexFloat secondDegreeFactor = a * (b + c + d) + b * (c + d) + c * d;
			ComplexFloat firstDegreeFactor = -c * d * (a + b) - a * b * (c + d);
			ComplexFloat constant = a * b * c * d;

			return std::string(TextFormat("P(z) ~= z^4 + (%.02g%+.02gi)z^3 + (%.02g%+.02gi)z^2 + (%.02g%+.02gi)z + (%.02g%+.02gi)", thirdDegreeFactor.real, thirdDegreeFactor.imaginary, secondDegreeFactor.real, secondDegreeFactor.imaginary, firstDegreeFactor.real, firstDegreeFactor.imaginary, constant.real, constant.imaginary));
		}

		std::string GetDegreeFiveFromRoots(ComplexFloat a, ComplexFloat b, ComplexFloat c, ComplexFloat d, ComplexFloat e)
		{
			ComplexFloat fourthDegreeFactor = -a - b - c - d - e;
			ComplexFloat thirdDegreeFactor = a * (b + c + d + e) + b * (c + d + e) + c * (d + e) + d * e;
			ComplexFloat secondDegreeFactor = -c * d * (b + a + e) - a * b * (c + d + e) - b * e * (c + d) - a * e * (c + d);
			ComplexFloat firstDegreeFactor = c * d * e * (a + b) + a * b * (d * e + c * (d + e));
			ComplexFloat constant = -a * b * c * d * e;

			return std::string(TextFormat("P(z) ~= z^5 + (%.02g%+.02gi)z^4 + (%.02g%+.02gi)z^3 + (%.02g%+.02gi)z^2 + (%.02g%+.02gi)z + (%.02g%+.02gi)", fourthDegreeFactor.real, fourthDegreeFactor.imaginary, thirdDegreeFactor.real, thirdDegreeFactor.imaginary, secondDegreeFactor.real, secondDegreeFactor.imaginary, firstDegreeFactor.real, firstDegreeFactor.imaginary, constant.real, constant.imaginary));
		}
	}
}

