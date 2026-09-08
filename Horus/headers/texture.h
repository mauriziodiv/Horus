#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "vec_math.h"
#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <ImathBox.h>
#include <filesystem>

class Texture
{
	public:
		Texture(): loaded(false), width(0), height(0) {};
		bool load(const std::string& filePath);
		Vector3D<float> sample(Point<float> uv);
		bool isLoaded() { return loaded; }
		int getWidth() { return width; }
		int getHeight() { return height; }
		const Vector3D<float>& getPixel(int x, int y) { return pixels[y * width + x]; }
		float getLuminance(int x, int y) { return (0.2126 * getPixel(x, y).x + 0.7152 * getPixel(x, y).y + 0.0722 * getPixel(x, y).z); };

	private:
		bool loaded;
		int width;
		int height;
		std::vector<Vector3D<float>> pixels;
};

class TextureSet
{
	public:
		bool load(const std::string& filePath);
		Vector3D<float> sample(Point<float> uv);
		bool isLoaded() { return !tiles.empty(); };

	private:
		std::unordered_map<int, Texture> tiles;
		bool loadTile(int tile, const std::string& filePath);
};