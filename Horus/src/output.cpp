#include "output.h"

void Output::writePPM()
{
	std::ofstream file(filePathWrite.data());

	file << "P3\n" << (int)width << " " << (int)height << "\n255\n";

	for (size_t i = 0; i < buffer.size(); ++i)
	{
		file << (int)(buffer[i].x * 255) << " " << (int)(buffer[i].y * 255) << " " << (int)(buffer[i].z * 255) << "\n";
	}

	buffer.clear();
}

void Output::write()
{
	switch (renderOutput)
	{
		case RenderOutput::PPM:
			writePPM();
			break;
			
		default:
			break;
	}
}