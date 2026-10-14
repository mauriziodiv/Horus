#include "output.h"
#include <ImfOutputFile.h>
#include <ImfHeader.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>

void Output::writePPM()
{
	if (!layers.empty())
	{
		std::cout << "PPM format does not support layers. Only the main buffer will be written." << std::endl;
	}

	std::ofstream file(filePathWrite.data());

	file << "P3\n" << (int)width << " " << (int)height << "\n255\n";

	for (size_t i = 0; i < buffer.size(); ++i)
	{
		file << (int)(buffer[i].x * 255) << " " << (int)(buffer[i].y * 255) << " " << (int)(buffer[i].z * 255) << "\n";
	}

	buffer.clear();
	layers.clear();
}

void Output::writeExr()
{
	Imf::Header header((int)width, (int)height);

	Imf::FrameBuffer frameBuffer;

	const size_t xStride = sizeof(Vector3D<float>);
	const size_t yStride = sizeof(Vector3D<float>) * (size_t)width;

	auto addRgb = [&](const std::string& name, const std::vector<Vector3D<float>>& rgbBuffer)
	{
		header.channels().insert(name + "R", Imf::Channel(Imf::FLOAT));
		header.channels().insert(name + "G", Imf::Channel(Imf::FLOAT));
		header.channels().insert(name + "B", Imf::Channel(Imf::FLOAT));

		frameBuffer.insert(name + "R", Imf::Slice(Imf::FLOAT, (char*)&rgbBuffer[0].x, xStride, yStride));
		frameBuffer.insert(name + "G", Imf::Slice(Imf::FLOAT, (char*)&rgbBuffer[0].y, xStride, yStride));
		frameBuffer.insert(name + "B", Imf::Slice(Imf::FLOAT, (char*)&rgbBuffer[0].z, xStride, yStride));
	};

	addRgb("", buffer);

	for (const Layer& layer : layers)
	{
		addRgb(layer.name + ".", layer.buffer);
	}

	try
	{
		Imf::OutputFile file(filePathWrite.data(), header);

		file.setFrameBuffer(frameBuffer);
		file.writePixels((int)height);
	}
	catch (const std::exception& e)
	{
		std::cout << "writeExr: " << e.what() << std::endl;
	}

	buffer.clear();
	layers.clear();
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