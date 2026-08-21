#pragma once
#include <vector>
#include "vec_math.h"
#include "ray.h"
#include "hrs.h"

class MeshLight;

class TriangleMesh
{
	public:
		TriangleMesh() : nVertices(0), nTriangles(0) {}
		
		int nTriangles;
		int nVertices;

		std::vector<int> vertexIndices;
		std::vector<int> normalIndices;
		std::vector<int> textureIndices;

		std::vector<Vector3D<float>> vertex;
		std::vector<Vector3D<float>> vertexNormal;
		std::vector<Point<float>> vertexUV;

		bool reverseOrientation, transformSwapsHandedness;

	private:
};

class Triangle : public GeometryObject
{
	public:
		Triangle();
		Triangle(const TriangleMesh* mesh, int tIndex);

		//void findVertices(int mIndex, int tIndex);

		virtual bool rayIntersection(Ray &ray, float tMin, float tMax) override;

		int meshIndex;
		int triangleIndex;

		static std::vector<const TriangleMesh*> *allMeshes;

		Vector3D<float> vertices[3];
		Vector3D<float> vertexNormals[3];
		Point<float> vertexUV[3];

		bool hasVertexNormals = false;
		bool hasVertexUV = false;

		virtual void setBoundingBox() override;
		virtual void computeNormal() override;
		virtual void computeTexture() override;
		virtual Vector3D<float> getNormal() override;

		std::string_view getObjectName() override { return name; }

		virtual MeshLight* getMeshLight() { return meshLightPtr; }

		MeshLight* meshLightPtr = nullptr;

	private:
		float epsilon = 0.001f;
		
		Vector3D<float> normal;
		Point<float> uv;

		static constexpr const char name[] = "Triangle";
};