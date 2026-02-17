#include "output.h"

void Output::writePPM()
{
	std::ofstream file(filePathWrite.data());

	//buffer.clear();

	file << "P3\n" << (int)width << " " << (int)height << "\n255\n";

	//for (int i = (int)height - 1; i >= 0; --i)
	//{
	//	for (int j = 0; j < (int)width; ++j)
	//	{
	//		
	//	}
	//}

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
			std::cout << "Buffer size: " << buffer.size() << " expected: " << (int)(width * height) << std::endl;
			writePPM();
			break;
			
		default:
			break;
	}
}