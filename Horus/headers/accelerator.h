#pragma once
#include "hrs.h"
#include <memory_resource>

using Allocator = std::pmr::polymorphic_allocator<std::byte>;

class BVHNode
{
public:
	BVHNode() : left(nullptr), right(nullptr) { }

	void assignBoundingBox(const BoundingBox& bb) { boundingBox.assignBoundingBox(bb); }
	void addBoundingBox(const BoundingBox& bb) { boundingBox += bb; };

	BoundingBox getBoundingBox() const { return boundingBox; }

	void addLeft(BVHNode* l) { left = l; };
	void addRight(BVHNode* r) { right = r; };

	BVHNode* getLeft() { return left; }
	BVHNode* getRight() { return right; }

	void setRoot(bool r) { root = r; }
	bool isRoot() { return root; }

private:
	BoundingBox boundingBox;
	BVHNode* left;
	BVHNode* right;

	bool root = false;
};

class BVH
{
public:
	BVH() : resource(256 * 1024, std::pmr::new_delete_resource()), allocator(&resource) {}

private:
	void* BVHNode_memory = resource.allocate(sizeof(BVHNode), alignof(BVHNode));

	std::pmr::monotonic_buffer_resource resource;
	Allocator allocator;

	BVHNode* root = nullptr;

	std::vector<BoundingBox> boundingBoxes;

	bool collectBoundingBoxes(std::vector<GeometryObject*>& objects);
	bool mergeBoundingBoxes(std::vector<GeometryObject*>& objects, BVHNode* node,  int32_t begin, int32_t end);

	bool buildRoot(std::vector<GeometryObject*>& objects);
};