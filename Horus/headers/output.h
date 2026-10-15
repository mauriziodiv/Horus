#pragma once
#include "vec_math.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
//#include <ImfRgbaFile.h>
//#include <ImfArray.h>

struct Layer
{
	std::string name;
	std::vector<Vector3D<float>> buffer;
};

enum class RenderOutput {
	PPM,
	EXR
};

class Output
{
	public:
		Output() : renderOutput(RenderOutput::PPM) {};
		Output(RenderOutput ro) : renderOutput(ro) {};

		void setRenderOutput(RenderOutput ro) { renderOutput = ro; }
		RenderOutput getRenderOutput() { return renderOutput; }

		void setFilePathWrite(const std::string_view& path) { filePathWrite = path; }
		std::string_view getFilePathWrite() { return filePathWrite; }

		void setWidth(float w) { width = w; }
		void setHeight(float h) { height = h; }

		float getWidth() { return width; }
		float getHeight() { return height; }

		void allocateBuffer() {
			buffer.assign((size_t)width * (size_t)height, Vector3D<float>(0.0f, 0.0f, 0.0f));
			alpha.assign((size_t)width * (size_t)height, 0.0f);
		};
		void writeBuffer(size_t index, Vector3D<float> value) { buffer[index] = value; };
		void writeAlpha(size_t index, float value) { alpha[index] = value; };

		size_t addLayer(const std::string& name)
		{
			layers.push_back(Layer{ name, std::vector<Vector3D<float>>((size_t)width * (size_t)height, Vector3D<float>(0.0f, 0.0f, 0.0f)) });

			return layers.size() - 1;
		}

		void writeLayer(size_t layer, size_t index, Vector3D<float> value)
		{
			layers[layer].buffer[index] = value;
		}

		void write();

	private:
		RenderOutput renderOutput;
		std::string_view filePathWrite;

		float width = 0.0f;
		float height = 0.0f;

		std::vector<Vector3D<float>> buffer;

		std::vector<float> alpha;

		std::vector<Layer> layers;

		void writePPM();
		void writeExr();
};