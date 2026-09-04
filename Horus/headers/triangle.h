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
		virtual void computeUV() override;
		virtual void computeTangents() override;
		virtual Vector3D<float> getNormal() override;

		virtual bool getHasTangents() override { return hasTangent; }
		virtual Vector3D<float> getTangent() override { return tangent; }
		virtual Vector3D<float> getBitangent() override { return bitangent; }

		virtual bool getHasVertexUV() override { return hasVertexUV; }
		virtual Point<float> getUV() override { return uv; }

		std::string_view getObjectName() override { return name; }

		virtual MeshLight* getMeshLight() { return meshLightPtr; }

		MeshLight* meshLightPtr = nullptr;

	private:
		float epsilon = 0.001f;
		float uvEpsilon = 0.0001f;
		
		Vector3D<float> normal;
		Point<float> uv;
		Vector3D<float> tangent;
		Vector3D<float> bitangent;
		bool hasTangent = false;

		static constexpr const char name[] = "Triangle";
};