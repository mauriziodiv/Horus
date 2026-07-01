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
	
	triangleMesh.nTriangles = static_cast<int>(triangleMesh.vertexIndices.size() / 3);

	return true;
}

void Mesh::buildTriangles()
{
	triangles.clear();
	triangles.reserve(triangleMesh.nTriangles);

	for (int i = 0; i < triangleMesh.nTriangles; i++)
	{
		triangles.emplace_back(&triangleMesh, i);
		triangles.back().setBoundingBox();
	}
}