#include "mesh.h"

Mesh::Mesh() : GeometryObject(GeometryType::MESH)
{

}

bool Mesh::importObj(const char* filename)
{
	if (!loadObj(filename, objMesh)) { return false; }

	return true;
}



Vector3D<float> Mesh::VecToVector(Vec3D v)
{
	Vector3D<float> vector3D;

	vector3D.x = v.getX();
	vector3D.y = v.getY();
	vector3D.z = v.getZ();

	return vector3D;
}

bool Mesh::getData()
{
	triangleMesh.vertexIndices.reserve(objMesh.getVertexIndices().size());
	triangleMesh.vertex.reserve(objMesh.getVertices().size());

	for (const unsigned int v : objMesh.getVertexIndices())
	{
		triangleMesh.vertexIndices.push_back(v - 1);
	}
	
	for (const Vec3D& v : objMesh.getVertices())
	{
		triangleMesh.vertex.push_back(VecToVector(v));
		triangleMesh.nVertices++;
	}

	if (!objMesh.getNormals().empty())
	{
		triangleMesh.vertexIndices.reserve(objMesh.getNormals().size());
		triangleMesh.vertexNormal.reserve(objMesh.getNormals().size());

		for (const unsigned int n : objMesh.getNormalIndices())
		{
			triangleMesh.normalIndices.push_back(n - 1);
		}

		for (const Vec3D& n : objMesh.getNormals())
		{
			triangleMesh.vertexNormal.push_back(VecToVector(n));
		}
	}

	if (!objMesh.getTextures().empty())
	{
		triangleMesh.vertexUV.reserve(objMesh.getTextures().size());

		for (const unsigned int t : objMesh.getTextureIndices())
		{
			triangleMesh.textureIndices.push_back(t - 1);
		}

		for (const Vec2D& t : objMesh.getTextures())
		{
			triangleMesh.vertexUV.emplace_back(t.getX(), t.getY());
		}
	}
	
	triangleMesh.nTriangles = static_cast<int>(triangleMesh.vertexIndices.size() / 3);

	return true;
}

void Mesh::buildTriangles()
{
	triangles.clear();
	triangles.reserve(triangleMesh.nTriangles);

	MeshLight* mLight = dynamic_cast<MeshLight*>(this);

	for (int i = 0; i < triangleMesh.nTriangles; i++)
	{
		triangles.emplace_back(&triangleMesh, i);
		triangles.back().setBoundingBox();
		triangles.back().meshLightPtr = mLight;
	}
}

bool Mesh::loadGeometry(const char* filename)
{
	std::string ext = std::filesystem::path(filename).extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

	if (ext == ".obj")
	{
		if (!importObj(filename)) { return false; };
		if (!getData()) { return false; }
		//add tranformation
		buildTriangles();

		return true;
	}

	return false;
}

std::vector<GeometryObject*> Mesh::getTriangles()
{
	std::vector<GeometryObject*> RTriangles;
	RTriangles.reserve(triangles.size());

	for (Triangle& t : triangles)
	{
		t.getShader() = getShader();
		RTriangles.push_back(&t);
	}

	return RTriangles;
}

void MeshLight::buildCDF()
{
	const std::vector<Triangle>& trianglesList = getTrianglesList();

	cdf.clear();
	cdf.reserve(trianglesList.size());

	for (const Triangle& t : trianglesList)
	{
		Vector3D<float> e1 = t.vertices[1] - t.vertices[0];
		Vector3D<float> e2 = t.vertices[2] - t.vertices[0];

		float area = 0.5f * (e1 | e2).getLength();

		totalArea += area;
		cdf.push_back(totalArea);
	}
}

LightSample MeshLight::sampleLight(float rndTriangle, float rU, float rV) const
{
	const std::vector<Triangle>& trList = getTrianglesList();

	float target = rndTriangle * totalArea;
	size_t index = std::upper_bound(cdf.begin(), cdf.end(), target) - cdf.begin();

	if (index >= trList.size())
	{
		index = trList.size() - 1;
	}

	float s = std::sqrt(rU);

	float b0 = 1.0f - s;
	float b1 = s * (1.0f - rV);
	float b2 = s * rV;

	const Triangle& triangle = trList[index];

	LightSample lightSample;
	
	Vector3D<float> pos = (triangle.vertices[0] * b0) + (triangle.vertices[1] * b1) + (triangle.vertices[2] * b2);
	lightSample.position = pos;

	Vector3D<float> norm = (triangle.vertexNormals[0] * b0) + (triangle.vertexNormals[1] * b1) + (triangle.vertexNormals[2] * b2);
	norm.normalize();
	lightSample.normal = norm;
	
	lightSample.pdf = 1.0f / totalArea;

	return lightSample;
}