#include "texture.h"

static bool readFloat(const std::string& filePath, std::vector<Vector3D<float>>& out, int& width, int& height)
{
	Imf::InputFile file(filePath.c_str());

	Imath::Box2i dw = file.header().dataWindow();

	width = dw.max.x - dw.min.x + 1;
	height = dw.max.y - dw.min.y + 1;

	if (width <= 0 || height <= 0)
	{
		return false;
	}

	// colour maps have R,G,B - single-channel maps (roughness...) only have Y
	bool isRgb = file.header().channels().findChannel("R") != nullptr;

	size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);

	std::vector<float> r(count), g(count), b(count);

	size_t xStride = sizeof(float);
	size_t yStride = sizeof(float) * width;

	Imf::FrameBuffer frameBuffer;

	if (isRgb)
	{
		frameBuffer.insert("R", Imf::Slice(Imf::FLOAT, (char*)(r.data() - dw.min.x - dw.min.y * width), xStride, yStride));
		frameBuffer.insert("G", Imf::Slice(Imf::FLOAT, (char*)(g.data() - dw.min.x - dw.min.y * width), xStride, yStride));
		frameBuffer.insert("B", Imf::Slice(Imf::FLOAT, (char*)(b.data() - dw.min.x - dw.min.y * width), xStride, yStride));
	}
	else
	{
		frameBuffer.insert("Y", Imf::Slice(Imf::FLOAT, (char*)(r.data() - dw.min.x - dw.min.y * width), xStride, yStride));
	}

	file.setFrameBuffer(frameBuffer);
	file.readPixels(dw.min.y, dw.max.y);

	out.resize(count);

	for (size_t i = 0; i < count; i++)
	{
		out[i] = isRgb ? Vector3D<float>(r[i], g[i], b[i]) : Vector3D<float>(r[i], r[i], r[i]);
	}

	return true;
}

bool Texture::load(const std::string& filePath)
{
	try
	{
		std::vector<Vector3D<float>> data;

		if (!readFloat(filePath, data, width, height))
		{
			return false;
		}

		pixels = std::make_shared<const std::vector<Vector3D<float>>>(std::move(data));

		loaded = true;

		return true;
	}
	catch (const std::exception& e)
	{
		std::cout << "Texture load failed!" << std::endl;

		return false;
	}

	return false;
}

Vector3D<float> Texture::sample(Point<float> uv)
{
	if (!loaded)
	{
		return Vector3D<float>(1.0f, 1.0f, 1.0f);
	}

	float u = uv.x - std::floor(uv.x);
	float v = uv.y - std::floor(uv.y);

	int x = static_cast<int>(u * width);
	int y = static_cast<int>((1.0f - v) * height);

	if (x < 0) { x = 0; };
	if (x > width - 1) { x = width - 1; };
	if (y < 0) { y = 0; };
	if (y > height - 1) { y = height - 1; };

	return (*pixels)[(y * width) + x];
}

bool TextureSet::loadTile(int tile, const std::string& filePath)
{
	Texture texture;

	if (!texture.load(filePath))
	{
		return false;
	}
	
	tiles[tile] = texture;
	
	return true;
}

bool TextureSet::load(const std::string& filePath)
{
	tiles.clear();

	size_t token = filePath.find("<UDIM>");

	if (token == std::string::npos)
	{
		if (!loadTile(1001, filePath))
		{
			return false;
		}
	}
	else
	{
		for (int udim = 1001; udim <1100; udim++)
		{
			std::string path = filePath;
			path.replace(token, 6, std::to_string(udim));

			if (!std::filesystem::exists(path))
			{
				continue;
			}
			
			loadTile(udim, path);
		}
	}

	return true;
}

Vector3D<float> TextureSet::sample(Point<float> uv)
{
	int udim = 1001 + int(std::floor(uv.x)) + 10 * int(std::floor(uv.y));

	auto u = tiles.find(udim);

	if (u == tiles.end())
	{
		return Vector3D<float>(1.0f, 1.0f, 1.0f);
	}

	return u->second.sample(uv);
}