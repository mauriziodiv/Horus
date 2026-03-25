#include "accelerator.h"

// Sorts the Morton primitives using a radix sort algorithm.
void BVH::sortMortonPrimitive(std::vector<MortonPrimitive>& mortonPrimitives)
{
	std::vector<MortonPrimitive> temp(mortonPrimitives.size());

	const int32_t bitsPerPass = 6;
	const int32_t bits = 30;
	const int32_t passes = bits / bitsPerPass;

	for (int32_t i = 0; i < passes; ++i)
	{
		int32_t firstBit = i * bitsPerPass;

		std::vector<MortonPrimitive>& begin = (i & 1) ? temp : mortonPrimitives;
		std::vector<MortonPrimitive>& end = (i & 1) ? mortonPrimitives : temp;

		const int32_t nBucket = 1 << bitsPerPass;
		const int32_t bitMask = nBucket - 1;
		int32_t bucketCount[nBucket] = { 0 };

		for (const MortonPrimitive& mp : begin)
		{
			int32_t buket = (mp.getMorton().code >> firstBit) & bitMask;
			++bucketCount[buket];
		}

		int outIndex[nBucket] = { 0 };

		for (int32_t i = 1; i < nBucket; ++ i)
		{
			outIndex[i] = outIndex[i - 1] + bucketCount[i - 1];
		}

		for (const MortonPrimitive& mp : begin)
		{
			int32_t buket = (mp.getMorton().code >> firstBit) & bitMask;
			end[outIndex[buket]++] = mp;
		}
	}

	if (passes & 1)
	{
		std::swap(mortonPrimitives, temp);
	}
}

// Remaps the coordinates of the geometry objects to fit within a 10-bit range for Morton code computation.
void BVH::remapCoordinatesForMorton(std::vector<GeometryObject*>& objects, Vector3D<float> min, Vector3D<float> max)
{
	Vector3D<float> centroid;

	uint32_t x;
	uint32_t y;
	uint32_t z;

	for (GeometryObject* obj : objects)
	{
		centroid = obj->getBoundingBox().getCentroid();

		x = static_cast<uint32_t>(((centroid.x - min.x) / (max.x - min.x) * 1023));
		y = static_cast<uint32_t>(((centroid.y - min.y) / (max.y - min.y) * 1023));
		z = static_cast<uint32_t>(((centroid.z - min.z) / (max.z - min.z) * 1023));

		obj->getBoundingBox().setCentroid(Vector3D<float>(x, y, z));
	}
}

// Collects the bounding boxes of the geometry objects and stores them in the 'boundingBoxes' vector. Returns true if successful, false otherwise.
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

// Creates a bounding box that encompasses the centroids of the geometry objects in the specified range. Returns true if successful, false otherwise.
bool BVH::createBoundingBoxFromCentroids(std::vector<GeometryObject*>& objects, int32_t begin, int32_t end)
{
	if (!objects.empty())
	{
		Vector3D<float> min = objects[begin]->getBoundingBox().getCentroid();
		Vector3D<float> max = min;

		for (int32_t i = begin + 1; i < end; ++i)
		{
			Vector3D<float> centroid = objects[i]->getBoundingBox().getCentroid();

			if (centroid.x < min.x) { min.x = centroid.x; }
			if (centroid.y < min.y) { min.y = centroid.y; }
			if (centroid.z < min.z) { min.z = centroid.z; }

			if (centroid.x > max.x) { max.x = centroid.x; }
			if (centroid.y > max.y) { max.y = centroid.y; }
			if (centroid.z > max.z) { max.z = centroid.z; }
		}

		//node->assignBoundingBox(BoundingBox(min, max));

		remapCoordinatesForMorton(objects, min, max);

		return true;
	}

	return false;
}

// Merges the bounding boxes of the geometry objects in the specified range and assigns the merged bounding box to the given BVH node. Returns true if successful, false otherwise.
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

// Builds the BVH tree by collecting bounding boxes, creating the root node, merging bounding boxes, and creating a bounding box from centroids. Returns true if successful, false otherwise.
bool BVH::buildRoot(std::vector<GeometryObject*>& objects)
{
	if (collectBoundingBoxes(objects))
	{
		BVHNode_memory = resource.allocate(sizeof(BVHNode), alignof(BVHNode));

		root = new (BVHNode_memory) BVHNode();

		root->setRoot(true);

		if (mergeBoundingBoxes(objects, root, 0, static_cast<int32_t>(boundingBoxes.size())))
		{
			if (createBoundingBoxFromCentroids(objects, 0, boundingBoxes.size()))
			{
				return true;
			}
		}

		return false;
	}

	return false;
}

// Computes the Morton codes for the geometry objects and stores them in the 'mortonPrimitives' vector. Returns true if successful, false otherwise.
bool BVH::computeMorton(std::vector<GeometryObject*>& objects)
{
	if (!objects.empty())
	{
		Morton morton;

		for (GeometryObject* obj : objects)
		{
			obj->createMorton();
			morton = obj->getMorton();
			mortonPrimitives.push_back(MortonPrimitive(morton, obj));
		}

		return true;
	}

	return false;
}

// Partitions the Morton primitives into treelets based on their Morton codes. Returns true if successful, false otherwise.
bool BVH::treeletSearch(std::vector<MortonPrimitive>& mortonPrimitives)
{
	if (!mortonPrimitives.empty())
	{
		uint32_t mask = 0b00111111111111000000000000000000;

		uint32_t old_codeCheck = 0;

		for (MortonPrimitive& mp : mortonPrimitives)
		{
			uint32_t codeCheck = mp.getMorton().code & mask;

			if (codeCheck != old_codeCheck || treelets.empty())
			{
				treelets.push_back(Treelet(mp));
			}
			else
			{
				treelets.back().addPrimitive(mp);
			}

			old_codeCheck = codeCheck;
		}

		return true;
	}

	return false;
}

// Performs a binary search on the Morton primitives to find the split point based on the specified mask. Returns the index of the split point.
uint32_t BVH::binarySearch(std::vector<MortonPrimitive*> mortonPrimitives, int32_t begin, int32_t end, uint32_t mask)
{
	int32_t mid = static_cast<int>(begin + (end - begin) / 2);

	if (end == begin + 1)
	{
		return end;
	}

	if (mask & mortonPrimitives[mid]->getMorton().code)
	{
		return binarySearch(mortonPrimitives, begin, mid, mask);
	}
	else
	{
		return binarySearch(mortonPrimitives, mid, end, mask);
	}
}


BVHNode* BVH::createLBVH(std::vector<MortonPrimitive*> mortonPrimitives, int32_t begin, int32_t end, uint32_t mask, BVHNode* nodes, int32_t& nodeIndex)
{
	if (mask == 0 || end - begin == 1)
	{
		//create leaf node
		BVHNode* node = new (&nodes[nodeIndex++]) BVHNode();
		node->assignBoundingBox(mortonPrimitives[begin]->getObject()->getBoundingBox());

		node->addPrimitive(mortonPrimitives[begin]->getObject());

		for (int32_t i = begin + 1; i< end; ++i)
		{
			node->addBoundingBox(mortonPrimitives[i]->getObject()->getBoundingBox());

			node->addPrimitive(mortonPrimitives[i]->getObject());
		}
		
		++totalNodes;

		return node;
	}

	if ((mask & mortonPrimitives[begin]->getMorton().code) == (mask & mortonPrimitives[end - 1]->getMorton().code))
	{
		// all prims have the same 17th bit, so we can skip to the next bit
		BVHNode* node = createLBVH(mortonPrimitives, begin, end, mask >> 1, nodes, nodeIndex);

		return node;
	}
	else
	{
		int32_t splitPoint = binarySearch(mortonPrimitives, begin, end, mask);

		BVHNode* left = createLBVH(mortonPrimitives, begin, splitPoint, mask >> 1, nodes, nodeIndex);
		BVHNode* right = createLBVH(mortonPrimitives, splitPoint, end, mask >> 1, nodes, nodeIndex);

		BVHNode* node = new (&nodes[nodeIndex++]) BVHNode();
		node->addLeft(left);
		node->addRight(right);
		node->assignBoundingBox(left->getBoundingBox() + right->getBoundingBox());

		++totalNodes;

		return node;
	}
}

// Connects the treelets into a single BVH tree. Returns the root node of the connected tree.
BVHNode* BVH::connectNodes(std::vector<Treelet>& treelets, BVHNode* nodes, int32_t& nodeIndex)
{
	if (!treelets.empty())
	{
		if (treelets.size() == 1)
		{
			return treelets[0].getRoot();
		}

		float xRange = 0.0f;
		float yRange = 0.0f;
		float zRange = 0.0f;

		float xMin;
		float yMin;
		float zMin;

		float xMax;
		float yMax;
		float zMax;

		treelets[0].getRoot()->getBoundingBox().computeCentroid();
		Vector3D<float> firstCentroid = treelets[0].getRoot()->getBoundingBox().getCentroid();

		xMin = xMax = firstCentroid.x;
		yMin = yMax = firstCentroid.y;
		zMin = zMax = firstCentroid.z;

		for (int32_t i = 1; i < treelets.size(); ++i)
		{
			BVHNode* root = treelets[i].getRoot();
			//BoundingBox treeletBB = root->getBoundingBox();
			root->getBoundingBox().computeCentroid();
			Vector3D<float> centroid = root->getBoundingBox().getCentroid();

			float x = centroid.x;
			float y = centroid.y;
			float z = centroid.z;

			if (x > xMax) { xMax = x; }
			if (y > yMax) { yMax = y; }
			if (z > zMax) { zMax = z; }

			if (x < xMin) { xMin = x; }
			if (y < yMin) { yMin = y; }
			if (z < zMin) { zMin = z; }
		}

		xRange = xMax - xMin;
		yRange = yMax - yMin;
		zRange = zMax - zMin;

		uint16_t axis = 0;

		if (yRange > xRange) { axis = 1; }
		if (zRange > yRange && zRange > xRange) { axis = 2; }

		const int32_t nBukets = 12;

		Bucket buckets[nBukets];

		float axisMin = (axis == 0) ? xMin : (axis == 1) ? yMin : zMin;
		float axisRange = (axis == 0) ? xRange : (axis == 1) ? yRange : zRange;

		std::vector<Treelet> leftTreelets;
		std::vector<Treelet> rightTreelets;

		if (axisRange == 0.0f)
		{
			int32_t mid = static_cast<int>(treelets.size() / 2);
			leftTreelets.assign(treelets.begin(), treelets.begin() + mid);
			rightTreelets.assign(treelets.begin() + mid, treelets.end());
		}
		else
		{
			for (int32_t i = 0; i < treelets.size(); ++i)
			{
				Vector3D<float> centroid = treelets[i].getRoot()->getBoundingBox().getCentroid();

				int32_t bucketIndex = nBukets * ((centroid[axis] - axisMin) / axisRange);

				if (bucketIndex == nBukets)
				{
					bucketIndex = nBukets - 1;
				}

				if (buckets[bucketIndex].getCount() == 0)
				{
					buckets[bucketIndex].setBoundingBox(treelets[i].getRoot()->getBoundingBox());

					buckets[bucketIndex].incCount();
				}
				else
				{
					buckets[bucketIndex].incCount();

					buckets[bucketIndex].getBoundingBox() += treelets[i].getRoot()->getBoundingBox();
				}
			}

			float cost[nBukets - 1];

			int32_t leftCount[nBukets - 1];
			int32_t rightCount[nBukets - 1];

			BoundingBox leftBoundingBox[nBukets - 1];
			BoundingBox rightBoundingBox[nBukets - 1];

			for (int32_t i = 0; i < nBukets - 1; ++i)
			{
				if (i == 0)
				{
					leftCount[0] = buckets[0].getCount();
					leftBoundingBox[0] = buckets[0].getBoundingBox();
				}
				else
				{
					leftCount[i] = leftCount[i - 1] + buckets[i].getCount();
					leftBoundingBox[i] = leftBoundingBox[i - 1] + buckets[i].getBoundingBox();
				}
			}

			for (int32_t i = nBukets - 2; i >= 0; --i)
			{
				if (i == nBukets - 2)
				{
					rightCount[i] = buckets[nBukets - 1].getCount();
					rightBoundingBox[i] = buckets[nBukets - 1].getBoundingBox();
				}
				else
				{
					rightCount[i] = rightCount[i + 1] + buckets[i + 1].getCount();
					rightBoundingBox[i] = rightBoundingBox[i + 1] + buckets[i + 1].getBoundingBox();
				}
			}

			float minCost;
			int32_t minCostSplit = 0;

			for (int32_t i = 0; i < nBukets - 1; ++i)
			{
				{
					cost[i] = (leftCount[i] * (leftBoundingBox[i].getSurfaceArea()) + (rightCount[i] * rightBoundingBox[i].getSurfaceArea()));

					if (i == 0)
					{
						minCost = cost[i];
					}
					else
					{
						if (cost[i] < minCost)
						{
							minCost = cost[i];
							minCostSplit = i;
						}
					}
				}
			}

			//std::vector<Treelet> leftTreelets;
			//std::vector<Treelet> rightTreelets;

			for (int32_t i = 0; i < treelets.size(); ++i)
			{
				int32_t bucketIndex = nBukets * (treelets[i].getRoot()->getBoundingBox().getCentroid()[axis] - axisMin) / axisRange;

				if (bucketIndex == nBukets) { bucketIndex = nBukets - 1; }

				if (bucketIndex <= minCostSplit)
				{
					leftTreelets.push_back(treelets[i]);
				}
				else
				{
					rightTreelets.push_back(treelets[i]);
				}
			}

			if (leftTreelets.empty() || rightTreelets.empty())
			{
				int32_t mid = static_cast<int>(treelets.size() / 2);
				leftTreelets.assign(treelets.begin(), treelets.begin() + mid);
				rightTreelets.assign(treelets.begin() + mid, treelets.end());
			}
		}

		BVHNode* leftNode = connectNodes(leftTreelets, nodes, nodeIndex);
		BVHNode* rightNode = connectNodes(rightTreelets, nodes, nodeIndex);
		
		BVHNode* node = new (&nodes[nodeIndex++]) BVHNode();

		node->addLeft(leftNode);
		node->addRight(rightNode);
		node->assignBoundingBox(leftNode->getBoundingBox() + rightNode->getBoundingBox());

		++totalNodes;

		return node;
	}

	return nullptr;
}

// Flattens the BVH tree into a linear array of nodes for efficient traversal. Returns the index of the current node in the linear array.
int32_t BVH::flattenBVH(BVHNode* node, int32_t& offset)
{
	int32_t index = offset;

	linearBVH& linNode = linearNodes[offset++];
	linNode.setBoundingBox(node->getBoundingBox());

	if (node->getLeft() == nullptr && node->getRight() == nullptr)
	{
		//leaf node
		int32_t primitiveOffset = orderedPrimitives.size();

		linNode.setPrimitiveOffset(primitiveOffset);
		linNode.setNPrimitives(node->getPrimitives().size());

		std::vector<GeometryObject*>& primitives = node->getPrimitives();

		for (GeometryObject* primitive : primitives)
		{
			orderedPrimitives.push_back(primitive);
		}

		return index;
	}
	else
	{
		int32_t leftOffset = flattenBVH(node->getLeft(), offset);
		int32_t rightOffset = flattenBVH(node->getRight(), offset);

		linNode.setSecondChildOffset(rightOffset);

		return index;
	}
}

// Creates the BVH nodes for each treelet. Returns true if successful, false otherwise.
bool BVH::createNodes(std::vector<Treelet>& treelets)
{
	if (!treelets.empty())
	{
		uint32_t mask = 1<< 17;

		//make sure the treelets have sorted items
		for (Treelet& treelet : treelets)
		{
			//createLBVH(treelet, mask);
			std::vector<MortonPrimitive*> mortonPrimitives = treelet.getPrimitives();

			int32_t end = mortonPrimitives.size();

			int32_t numberOfNodes = 2 * end - 1;
			BVHNode* nodes = static_cast<BVHNode*>(resource.allocate(sizeof(BVHNode) * numberOfNodes, alignof(BVHNode)));
			int32_t nodeIndex = 0;

			BVHNode* root = createLBVH(mortonPrimitives, 0, end, mask, nodes, nodeIndex);
			treelet.setRoot(root);
		}

		return true;
	}

	return false;
}

// Builds the BVH tree from the given geometry objects by performing several steps including building the root node, computing Morton codes, sorting Morton primitives, partitioning into treelets, creating nodes for each treelet, and connecting the nodes into a single BVH tree.
void BVH::buildBVH(std::vector<GeometryObject*>& objects)
{
	if (buildRoot(objects))
	{
		if (computeMorton(objects))
		{
			sortMortonPrimitive(mortonPrimitives);

			if (treeletSearch(mortonPrimitives))
			{
				if (createNodes(treelets))
				{
					BVHNode* nodes = static_cast<BVHNode*>(resource.allocate(sizeof(BVHNode) * (treelets.size() - 1), alignof(BVHNode)));

					int32_t nodeIndex = 0;

					root = connectNodes(treelets, nodes, nodeIndex);

					int32_t offset = 0;

					linearNodes.resize(totalNodes);

					flattenBVH(root, offset);
				}
			}
		}
	}
}

// Traverses the BVH tree to find the closest intersection of a ray with the geometry objects. Returns a pointer to the closest hit object, or nullptr if no intersection is found.
GeometryObject* BVH::traversal(Ray& ray, float tMin, float tMax)
{
	GeometryObject* closestHit = nullptr;
	float closestT = tMax;

	int32_t stack[64];
	int32_t stackIndex = 0;
	int32_t nodeIndex = 0;

	while (true)
	{
		linearBVH& node = linearNodes[nodeIndex];

		if (node.getBoundingBox().intersect(ray, tMin, closestT))
		{
			if (node.getNPrimitives() > 0)
			{
				for (int32_t i = 0; i < node.getNPrimitives(); ++i)
				{
					GeometryObject* obj = orderedPrimitives[node.getPrimitiveOffset() + i];

					if (obj->rayIntersection(ray, tMin, closestT))
					{
						closestT = obj->hitRecord.t;
						closestHit = obj;
					}
				}

				if (stackIndex == 0) { break; }
				nodeIndex = stack[--stackIndex];
			}
			else
			{
				stack[stackIndex++] = node.getSecondChildOffset();
				++nodeIndex;
			}
		}
		else
		{
			if (stackIndex == 0) { break; }
			nodeIndex = stack[--stackIndex];
		}
	}

	return closestHit;
}