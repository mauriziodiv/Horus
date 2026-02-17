#pragma once
#include "vec_math.h"
#include <vector>
#include <iostream>
#include <fstream>

enum class RenderOutput {
	PPM
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

		void writeBuffer(Vector3D<float> value) { buffer.push_back(value); };

		void write();

	private:
		RenderOutput renderOutput;
		std::string_view filePathWrite;

		float width = 0.0f;
		float height = 0.0f;

		std::vector<Vector3D<float>> buffer;

		void writePPM();
};