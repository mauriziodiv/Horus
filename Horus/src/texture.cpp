#include "texture.h"

static bool readRgba(Imf::RgbaInputFile& file, Imf::Array2D<Imf::Rgba>& out, int& width, int& height)
{
	Imath::Box2i dw = file.dataWindow();

	width = dw.max.x - dw.min.x + 1;
	height = dw.max.y - dw.min.y + 1;

	if (width <= 0 || height <= 0)
	{
		return false;
	}

	out.resizeErase(height, width);
	
	file.setFrameBuffer(&out[0][0] - dw.min.x - dw.min.y * width, 1, width);
	file.readPixels(dw.min.y, dw.max.y);

	return true;
}

bool Texture::load(const std::string& filePath)
{
	try
	{

		Imf::RgbaInputFile texture(filePath.c_str());

		Imf::Array2D<Imf::Rgba> halfPixel;

		if (!readRgba(texture, halfPixel, width, height))
		{
			return false;
		}

		pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height));

		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				const Imf::Rgba& p = halfPixel[i][j];
				pixels[(i * width) + j] = Vector3D<float>(p.r, p.g, p.b);
			}
		}
		
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

	return pixels[(y * width) + x];
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
			//FROM HERE
			loadTile(udim, path);
		}
	}

	return true;
}

Vector3D<float> TextureSet::sample(Point<float> uv)
{
	return Vector3D<float>();
}