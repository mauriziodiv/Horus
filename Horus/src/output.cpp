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

void Output::writeExr()
{
	Imf::Array2D<Imf::Rgba> pixels((int)height, (int)width);
	
	for (int y = 0; y< (int)height; y++)
	{
		for (int x = 0; x < (int)width; x++)
		{
			const Vector3D<float>& pixelData = buffer[(size_t)y * (size_t)width + x];

			pixels[y][x].r = pixelData.x;
			pixels[y][x].g = pixelData.y;
			pixels[y][x].b = pixelData.z;
			pixels[y][x].a = 1.0f;
		}
	}

	try
	{
		Imf::RgbaOutputFile file(filePathWrite.data(), (int)width, (int)height, Imf::WRITE_RGBA);

		file.setFrameBuffer(&pixels[0][0], 1, (size_t)width);
		file.writePixels((int)height);
	}
	catch (const std::exception& e)
	{
		std::cout << "writeExr: " << e.what() << std::endl;
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

		case RenderOutput::EXR:
			writeExr();
			break;

		default:
			break;
	}
}