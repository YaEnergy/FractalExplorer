#pragma once

#include <string>

#include "ComplexFloat.h"

namespace Explorer
{
	namespace ComplexPolynomial
	{
		//Provides some functions for getting polynomial equation strings from roots
		//The explorer itself doesn't need anything more, than just the strings

		std::string GetDegreeTwoFromRoots(ComplexFloat a, ComplexFloat b);
		std::string GetDegreeThreeFromRoots(ComplexFloat a, ComplexFloat b, ComplexFloat c);
		std::string GetDegreeFourFromRoots(ComplexFloat a, ComplexFloat b, ComplexFloat c, ComplexFloat d);
		std::string GetDegreeFiveFromRoots(ComplexFloat a, ComplexFloat b, ComplexFloat c, ComplexFloat d, ComplexFloat e);
	}
}
