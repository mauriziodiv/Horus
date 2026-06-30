#include "triangle.h"

std::vector<const TriangleMesh*>* Triangle::allMeshes = nullptr;

Triangle::Triangle() : GeometryObject(GeometryType::TRIANGLE)
{
	meshIndex = -1;
	triangleIndex = -1;
}

//Triangle::Triangle(int mIndex, int tIndex)
//{
//
//}

Triangle::Triangle(const TriangleMesh* mesh, int tIndex) : GeometryObject(GeometryType::TRIANGLE)
{
	//const TriangleMesh* mesh = (*allMeshes)[mIndex];

	int i01 = mesh->vertexIndices[tIndex * 3];
	int i02 = mesh->vertexIndices[(tIndex * 3) + 1];
	int i03 = mesh->vertexIndices[(tIndex * 3) + 2];

	vertices[0] = mesh->vertex[i01];
	vertices[1] = mesh->vertex[i02];
	vertices[2] = mesh->vertex[i03];
}

bool Triangle::rayIntersection(Ray& ray, float tMin, float tMax)
{
	// Move the ray to the origin
	Ray originRay;
	originRay.setOrigin(Vector3D<float>(0.0f, 0.0f, 0.0f));
	originRay.setDirection(ray.getDirection());

	// Permute ray direction to maximize z component
	float x = std::abs(originRay.getDirection().x);
	float y = std::abs(originRay.getDirection().y);
	float z = std::abs(originRay.getDirection().z);

	int kx, ky, kz;

	if (x > y && x > z) { kz = 0; }
	else if (y > x && y > z) { kz = 1; }
	else { kz = 2; };

	kx = kz + 1;
	if (kx == 3) { kx = 0; };

	ky = kx + 1;
	if (ky == 3) { ky = 0; };

	originRay.setDirection(Vector3D<float>(originRay.getDirection()[kx], originRay.getDirection()[ky], originRay.getDirection()[kz]));

	// apply shear transformation to vertices
	float Sx = -originRay.getDirection().x / originRay.getDirection().z;
	float Sy = -originRay.getDirection().y / originRay.getDirection().z;
	float Sz = 1.0f / originRay.getDirection().z;

	Vector3D<float> p0t = vertices[0] - ray.getOrigin();
	Vector3D<float> p1t = vertices[1] - ray.getOrigin();
	Vector3D<float> p2t = vertices[2] - ray.getOrigin();

	Vector3D<float> p0 = Vector3D<float>(p0t[kx], p0t[ky], p0t[kz]);
	Vector3D<float> p1 = Vector3D<float>(p1t[kx], p1t[ky], p1t[kz]);
	Vector3D<float> p2 = Vector3D<float>(p2t[kx], p2t[ky], p2t[kz]);

	float p0Sx = p0.x + (Sx * p0.z);
	float p0Sy = p0.y + (Sy * p0.z);
	
	float p1Sx = p1.x + (Sx * p1.z);
	float p1Sy = p1.y + (Sy * p1.z);

	float p2Sx = p2.x + (Sx * p2.z);
	float p2Sy = p2.y + (Sy * p2.z);

	// edge function
	float e0 = (p1Sx * p2Sy) - (p1Sy * p2Sx);
	float e1 = (p2Sx * p0Sy) - (p2Sy * p0Sx);
	float e2 = (p0Sx * p1Sy) - (p0Sy * p1Sx);

	if ((e0 > 0 || e1 > 0 || e2 > 0) && (e0 < 0 || e1 < 0 || e2 < 0))
	{
		// the ray misses the triangle
		return false;
	}

	float det = e0 + e1 + e2;

	if (det == 0)
	{
		return false;
	}

	float p0Sz = Sz * p0.z;
	float p1Sz = Sz * p1.z;
	float p2Sz = Sz * p2.z;

	float invDet = 1.0f / det;

	float num = (e0 * p0Sz) + (e1 * p1Sz) + (e2 * p2Sz);

	float b0 = e0 * invDet;
	float b1 = e1 * invDet;
	float b2 = e2 * invDet;

	float t = num * invDet;

	//if (t < ray.getTMin() || t > ray.getTMax())
	//{
	//	return false;
	//}

	if (t < tMin || t > tMax)
	{
		return false;
	}

	hitRecord.t = t;
	hitRecord.hitPoint = ray.getPointat(t);
	hitRecord.front = (det > 0);
	hitRecord.back = (det < 0);

	return true;
}

void Triangle::setBoundingBox()
{
	float x_min = std::min(std::min(vertices[0].x, vertices[1].x), vertices[2].x);
	float x_max = std::max(std::max(vertices[0].x, vertices[1].x), vertices[2].x);

	float y_min = std::min(std::min(vertices[0].y, vertices[1].y), vertices[2].y);
	float y_max = std::max(std::max(vertices[0].y, vertices[1].y), vertices[2].y);

	float z_min = std::min(std::min(vertices[0].z, vertices[1].z), vertices[2].z);
	float z_max = std::max(std::max(vertices[0].z, vertices[1].z), vertices[2].z);

	boundingBox.setMin(Vector3D<float>(x_min - epsilon, y_min - epsilon, z_min - epsilon));
	boundingBox.setMax(Vector3D<float>(x_max + epsilon, y_max + epsilon, z_max + epsilon));

	boundingBox.computeCentroid();
}

void Triangle::computeNormal()
{
	Vector3D<float> edge_01 = vertices[1] - vertices[0];
	Vector3D<float> edge_02 = vertices[2] - vertices[0];
	
	normal = edge_01 | edge_02;
	normal.normalize();
}

Vector3D<float> Triangle::getNormal()
{
	return normal;
}