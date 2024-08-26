#pragma once

#include <string>

#include "raylib.h"

namespace Explorer
{
	struct Notification
	{
		std::string message = "";
		float timeSeconds = 0.0f;
		Color color = WHITE;

		Notification()
		{

		}

		Notification(std::string message, float timeSeconds, Color color)
		{
			this->message = message;
			this->timeSeconds = timeSeconds;
			this->color = color;
		}

		Notification(std::exception& ex)
		{
			message = "An exception has occured. Message: " + std::string(ex.what());
			timeSeconds = 15.0f;
			color = MAROON;
		}
	};

}