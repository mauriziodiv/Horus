#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "vec_math.h"
#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <ImathBox.h>

class Texture
{
	public:
		Texture(): loaded(false), width(0), height(0) {};
		bool load(const std::string& filePath);
		Vector3D<float> sample(Point<float> uv);
		bool isLoaded() { return loaded; }

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