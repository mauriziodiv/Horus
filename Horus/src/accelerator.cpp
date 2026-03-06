#include "accelerator.h"

bool BVH::collectBoundingBoxes(std::vector<GeometryObject*>& objects)
{
	if (!objects.empty())
	{
		for (GeometryObject* obj : objects)
		{
			boundingBoxes.push_back(obj->getBoundingBox());
		}

		return true;
	}

	return false;
}

bool BVH::mergeBoundingBoxes(std::vector<GeometryObject*>& objects, BVHNode* node, int32_t begin, int32_t end)
{
	if (!objects.empty())
	{
		node->assignBoundingBox(boundingBoxes[begin]);

		for (int32_t i = begin + 1; i < end; ++i)
		{
			node->addBoundingBox(boundingBoxes[i]);
		}

		return true;
	}

	return false;
}

bool BVH::buildRoot(std::vector<GeometryObject*>& objects)
{
	if (collectBoundingBoxes(objects))
	{
		//BVHNode_memory = resource.allocate(sizeof(BVHNode), alignof(BVHNode));

		root = new (BVHNode_memory) BVHNode();

		root->setRoot(true);

		if (mergeBoundingBoxes(objects, root, 0, static_cast<int32_t>(boundingBoxes.size())))
		{
			return true;
		}

		return false;
	}

	return false;
}