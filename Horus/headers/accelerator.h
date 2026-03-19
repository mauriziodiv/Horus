#pragma once
#include "hrs.h"
#include <memory_resource>

using Allocator = std::pmr::polymorphic_allocator<std::byte>;

struct MortonPrimitive
{
public:
	MortonPrimitive() : object(nullptr) {}
	MortonPrimitive(Morton v, GeometryObject* obj) : morton(v), object(obj) {}

	Morton getMorton() const { return morton; }
	GeometryObject* getObject() const { return object; }

private:
	Morton morton;
	GeometryObject* object;
};

struct Bucket
{
	public:
		Bucket() : count(0), boundingBox() {}

		void incCount() { ++count; }
		void setBoundingBox(const BoundingBox& bb) { boundingBox.assignBoundingBox(bb); }

		int32_t getCount() const { return count; }
		BoundingBox& getBoundingBox() { return boundingBox; }

	private:
		int32_t count;
		BoundingBox boundingBox;
};

struct linearBVH
{
	public:
		linearBVH() : secondChildOffset(0), primitiveOffset(0), nPrimitives(0) {}

		void setBoundingBox(const BoundingBox& bb) { boundingBox.assignBoundingBox(bb); }
		void setPrimitiveOffset(int32_t offset) { primitiveOffset = offset; }
		void setNPrimitives(int32_t n) { nPrimitives = n; }
		void setSecondChildOffset(int32_t offset) { secondChildOffset = offset; }

		BoundingBox& getBoundingBox() { return boundingBox; }
		int32_t getPrimitiveOffset() const { return primitiveOffset; }
		int32_t getNPrimitives() const { return nPrimitives; }
		int32_t getSecondChildOffset() const { return secondChildOffset; }

	private:
		BoundingBox boundingBox;
		int32_t primitiveOffset;
		int32_t nPrimitives;
		int32_t secondChildOffset;
};

class BVHNode
{
public:
	BVHNode() : left(nullptr), right(nullptr) { }

	void assignBoundingBox(const BoundingBox& bb) { boundingBox.assignBoundingBox(bb); }
	void assignBoundingBox(const Vector3D<float>& min, const Vector3D<float>& max) { boundingBox.assignBoundingBox(BoundingBox(min, max)); }
	void addBoundingBox(const BoundingBox& bb) { boundingBox += bb; };

	BoundingBox& getBoundingBox() { return boundingBox; }

	void addLeft(BVHNode* l) { left = l; };
	void addRight(BVHNode* r) { right = r; };

	void addPrimitive(GeometryObject* prim) { primitives.push_back(prim); }

	BVHNode* getLeft() { return left; }
	BVHNode* getRight() { return right; }

	void setRoot(bool r) { root = r; }
	bool isRoot() { return root; }

	std::vector<GeometryObject*>& getPrimitives() { return primitives; }

private:
	BoundingBox boundingBox;
	BVHNode* left;
	BVHNode* right;

	bool root = false;

	std::vector<GeometryObject*> primitives;
};

struct Treelet
{
public:
	Treelet(MortonPrimitive& p) { primitives.push_back(&p); root = nullptr; }
	void addPrimitive(MortonPrimitive& p) { primitives.push_back(&p); };
	std::vector<MortonPrimitive*>& getPrimitives() { return primitives; }
	void setRoot(BVHNode* rt) { root = rt; }
	BVHNode* getRoot() { return root; }

private:
	std::vector<MortonPrimitive*> primitives;
	BVHNode* root;
};

class BVH
{
public:
	BVH() : resource(256 * 1024, std::pmr::new_delete_resource()), allocator(&resource) {}

	void buildBVH(std::vector<GeometryObject*>& objects);

private:
	void* BVHNode_memory = nullptr;

	std::pmr::monotonic_buffer_resource resource;
	Allocator allocator;

	BVHNode* root = nullptr;

	std::vector<BoundingBox> boundingBoxes;

	std::vector<MortonPrimitive> mortonPrimitives;
	std::vector<Treelet> treelets;

	std::vector<GeometryObject*> orderedPrimitives;

	std::vector<linearBVH> linearNodes;

	int32_t totalNodes = 0;

	void sortMortonPrimitive(std::vector<MortonPrimitive>& mortonPrimitives);
	void remapCoordinatesForMorton(std::vector<GeometryObject*>& objects, Vector3D<float> min, Vector3D<float> max);

	bool collectBoundingBoxes(std::vector<GeometryObject*>& objects);
	bool createBoundingBoxFromCentroids(std::vector<GeometryObject*>& objects, int32_t begin, int32_t end);
	bool mergeBoundingBoxes(std::vector<GeometryObject*>& objects, BVHNode* node,  int32_t begin, int32_t end);

	bool buildRoot(std::vector<GeometryObject*>& objects);
	bool computeMorton(std::vector<GeometryObject*>& objects);

	bool treeletSearch(std::vector<MortonPrimitive>& mortonPrimitives);

	BVHNode* createLBVH(std::vector<MortonPrimitive*> mortonPrimitive, int32_t begin, int32_t end, uint32_t mask, BVHNode* nodes, int32_t& nodeIndex);

	uint32_t binarySearch(std::vector<MortonPrimitive*> mortonPrimitives, int32_t begin, int32_t end, uint32_t mask);

	bool createNodes(std::vector<Treelet>& treelets);

	BVHNode* connectNodes(std::vector<Treelet>& treelets, BVHNode* nodes, int32_t& nodeIndex);

	int32_t flattenBVH(BVHNode* node, int32_t& offset);
};