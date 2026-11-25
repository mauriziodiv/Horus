#include "scene.h"
#include <thread>
#include <atomic>
#include <fstream>
#include <sstream>
#include <opensubdiv/far/topologyDescriptor.h>
#include <opensubdiv/far/primvarRefiner.h>
#include <filesystem>
#include <opensubdiv/bfr/refinerSurfaceFactory.h>
#include <opensubdiv/bfr/surface.h>
#include <opensubdiv/bfr/tessellation.h>

struct Vertex
{
	float x, y, z;

	Vertex() : x(0.0f), y(0.0f), z(0.0f) {}

	void Clear()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	void AddWithWeight(const Vertex& v, float weight)
	{
		x += v.x * weight;
		y += v.y * weight;
		z += v.z * weight;
	}
};

struct FVarUV
{
	float u, v;

	FVarUV() : u(0.0f), v(0.0f) {}

	void Clear()
	{
		u = 0.0f;
		v = 0.0f;
	}

	void AddWithWeight(const FVarUV& uv, float weight)
	{
		u += uv.u * weight;
		v += uv.v * weight;
	}
};

bool parseFloat(std::string_view token, float& v)
{
	const char* first = token.data();
	const char* last = first + token.size();

	std::from_chars_result res = std::from_chars(first, last, v);

	return res.ec == std::errc{} && res.ptr == last;
}

std::unordered_map<std::string_view, RenderOutput> renderOutputMap = {
	{ "ppm", RenderOutput::PPM },
	{ "exr", RenderOutput::EXR }
};

std::unordered_map<std::string_view, GammaCorrection> gammaCorrectionMap = {
	{"gamma2", GammaCorrection::GAMMA2}
};

Scene::Scene() : sceneObjects(), camera(nullptr), geometries(), lights(), renderOutput(RenderOutput::PPM), output(), filePathWrite()
{

}

// Initializes the scene by checking for the presence of a camera, geometries, and lights. Returns true if the scene is valid, false otherwise.
bool Scene::getScene(std::vector<std::unique_ptr<SceneObject>>& scene)
{
	if (scene.empty())
	{
		return false;
	}

	//sceneObjects = scene;
	sceneObjects = std::move(scene);

	if (!cameraCheck())
	{
		return false;
	}

	geometriesCheck();
	lightCheck();

	return true;
}

// Checks for the presence of a camera.
bool Scene::cameraCheck()
{
	for (const auto& obj : sceneObjects)
	{
		if (obj->getType() == SceneObjectType::CAMERA)
		{
			camera = static_cast<CameraObject*>(obj.get());
			camera->setWindow(getWidth(), getHeight());

			break;
		}
	}

	if (camera == nullptr)
	{
		std::cout << "There is no camera in the scene!" << std::endl;
		return false;
	}

	return true;
}

// Checks for the presence of geometries and populates the geometries vector.
bool Scene::geometriesCheck()
{
	geometries.clear();

	for (const auto& geo : sceneObjects)
	{
		if (geo->getType() == SceneObjectType::GEOMETRY)
		{
			GeometryObject* geometryObject = static_cast<GeometryObject*>(geo.get());

			if (geometryObject->getGeometryType() == GeometryType::MESH)
			{
				if (geometryObject->hasDisplacement() && geometryObject->getMaxSubdivisions() > 0)
				{
					if (Mesh* mesh = dynamic_cast<Mesh*>(geometryObject))
					{
						std::cout << "Displacement: " << mesh->getObjPath() << " -> subdivide up to level " << mesh->getMaxSubdivisions() << ", bound " << mesh->getDisplacementBound() << std::endl;

						if (!displacementGeo(*mesh))
						{
							std::cout << "Displacement failed!" << std::endl;
						}
					
					}
				}

				std::vector<GeometryObject*> triangles = geometryObject->getTriangles();
				geometries.insert(geometries.end(), triangles.begin(), triangles.end());
			}
			else
			{
				geometries.push_back(geometryObject);
			}
		}
	}

	if (geometries.size() == 0)
	{
		std::cout << "Warning: There are no geometries in the scene!" << std::endl;
	}

	return true;
}

// Checks for the presence of lights and populates the lights vector.
bool Scene::lightCheck()
{
	lights.clear();
	areaLights.clear();
	meshLights.clear();
	infiniteLights.clear();

	for (const auto& lgts : sceneObjects)
	{
		if (lgts->getType() == SceneObjectType::LIGHT)
		{
			lights.push_back(static_cast<LightObject*>(lgts.get()));

			if (InfiniteLight* infiniteLight = dynamic_cast<InfiniteLight*>(lgts.get()))
			{
				infiniteLights.push_back(infiniteLight);
			}
		}
		else if (AreaLight* areaLight = dynamic_cast<AreaLight*>(lgts.get()))
		{
			areaLights.push_back(areaLight);
		}
		else if (MeshLight* meshLight = dynamic_cast<MeshLight*>(lgts.get()))
		{
			meshLight->buildCDF();
			meshLights.push_back(meshLight);
			//meshLights.buildCDF();
		}
	}

	if (lights.size() == 0 && areaLights.size() == 0 && meshLights.size() == 0 && infiniteLights.size() == 0)
	{
		std::cout << "Warning: There are no lights in the scene!" << std::endl;
	}

	return true;
}

bool Scene::computeVertexNormals(const std::vector<Vector3D<float>>& positions, const std::vector<int>& cornerCounts, const std::vector<int>& positionIndices, std::vector<Vector3D<float>>& outNormals)
{
	outNormals.clear();
	outNormals.resize(positions.size());

	int offset = 0;
	int count = 0;

	for (size_t i = 0; i < cornerCounts.size(); ++i)
	{
		count = cornerCounts[i];

		if (count != 4 && count != 3)
		{
			offset += count;
			continue;
		}

		Vector3D<float> v0, v1, v2, v3;

		const int* index = &positionIndices[offset];

		v0 = positions[index[0]];
		v1 = positions[index[1]];
		v2 = positions[index[2]];

		if (count == 4) { v3 = positions[index[3]]; }

		Vector3D<float> faceNormal;

		if (count == 4)
		{
			faceNormal = (v2 - v0) | (v3 - v1);
		}
		else if (count == 3)
		{
			faceNormal = (v1 - v0) | (v2 - v1);
		}

		for (int j = 0; j < count; ++j)
		{
			outNormals[index[j]] += faceNormal;
		}

		offset += count;
	}

	for (size_t i = 0; i < outNormals.size(); ++i)
	{
		float length = outNormals[i].getLength();

		if (length > 0)
		{
			outNormals[i] = outNormals[i] / length;
		}
	}
	
	return true;
}

bool Scene::getCorners(const std::string& objPath, std::vector<int>& cornersCounts, std::vector<int>& positions, std::vector<int>& uvs, int& numberOfVerticesTotal, int& numberOfUvTotal, const std::string& ext)
{
	if (ext == ".obj")
	{
		std::ifstream objFile(objPath);

		if (!objFile.is_open())
		{
			std::cout << "Failed to open OBJ file: " << objPath << std::endl;

			return false;
		}

		std::string line;
		int numberOfVertices = 0;
		numberOfVerticesTotal = 0;
		numberOfUvTotal = 0;

		while (std::getline(objFile, line))
		{
			if (line.find("v ") == 0)
			{
				numberOfVerticesTotal++;
			}

			if (line.find("vt ") == 0)
			{
				numberOfUvTotal++;
			}


			std::istringstream lineStream(line);

			char type = 0;
			lineStream >> type;

			if (type == 'f')
			{
				std::string vertex;

				while (lineStream >> vertex)
				{
					std::istringstream elements(vertex);
					std::string position, uv;

					std::getline(elements, position, '/');
					std::getline(elements, uv, '/');

					if (position.empty())
					{
						std::cout << "Error: Invalid OBJ file format. Position index is missing." << std::endl;

						return false;
					}

					if (uv.empty())
					{
						std::cout << "Error: Invalid OBJ file format. UV index is missing." << std::endl;

						return false;
					}

					int positionIndex = std::stoi(position);
					int uvIndex = std::stoi(uv);

					numberOfVertices++;

					positions.push_back(positionIndex - 1);
					uvs.push_back(uvIndex - 1);
				}

				cornersCounts.push_back(numberOfVertices);
				numberOfVertices = 0;
			}
		}

		return true;
	}
	
	return false;
}

OpenSubdiv::Far::TopologyRefiner* Scene::sendOpenSubdiv(int numberOfVertices, const std::vector<int>& cornerCounts, const std::vector<int>& positions, int numberOfUvTotal, const std::vector<int>& uvs, int level)
{
	OpenSubdiv::Far::TopologyDescriptor desc;

	desc.numVertices = numberOfVertices;
	desc.numFaces = (int)cornerCounts.size();
	desc.numVertsPerFace = cornerCounts.data();
	desc.vertIndicesPerFace = positions.data();

	OpenSubdiv::Far::TopologyDescriptor::FVarChannel uvChannel;
	uvChannel.numValues = numberOfUvTotal;
	uvChannel.valueIndices = uvs.data();

	desc.numFVarChannels = 1;
	desc.fvarChannels = &uvChannel;

	OpenSubdiv::Sdc::Options sdcOptions;
	sdcOptions.SetVtxBoundaryInterpolation(OpenSubdiv::Sdc::Options::VTX_BOUNDARY_EDGE_AND_CORNER);
	sdcOptions.SetFVarLinearInterpolation(OpenSubdiv::Sdc::Options::FVAR_LINEAR_CORNERS_ONLY);

	typedef OpenSubdiv::Far::TopologyRefinerFactory<OpenSubdiv::Far::TopologyDescriptor> RefinerFactory;

	RefinerFactory::Options options(OpenSubdiv::Sdc::SCHEME_CATMARK, sdcOptions);

	OpenSubdiv::Far::TopologyRefiner* refiner = RefinerFactory::Create(desc, options);

	if (refiner == nullptr)
	{
		std::cout << "OpenSubdiv: could not build the refiner" << std::endl;

		return nullptr;
	}

	if (level > 0)
	{
		refiner->RefineUniform(OpenSubdiv::Far::TopologyRefiner::UniformOptions(level));
	}

	return refiner;
}

bool Scene::linkOpenSubdiv(OpenSubdiv::Far::TopologyRefiner* refiner, const std::vector<Vector3D<float>>& vertex, const std::vector<Point<float>>& uvs, std::vector<Vector3D<float>>& outPositions, std::vector<Point<float>>& outUvs, std::vector<int>& outCornerCounts, std::vector<int>& outPositionIndices, std::vector<int>& outUvIndices)
{
	std::vector<Vertex> vertexData(refiner->GetNumVerticesTotal());
	std::vector<FVarUV> uvData(refiner->GetNumFVarValuesTotal(0));
	
	//check that level 0 has the same number of vertices as the original mesh
	std::cout << "Level 0 vertices: " << refiner->GetLevel(0).GetNumVertices() << ", original mesh vertices: " << vertex.size() << std::endl;
	std::cout << "Level 0 UVs: " << refiner->GetLevel(0).GetNumFVarValues(0) << ", original mesh UVs: " << uvs.size() << std::endl;

	for (size_t i = 0; i < vertex.size(); ++i)
	{
		vertexData[i].x = vertex[i].x;
		vertexData[i].y = vertex[i].y;
		vertexData[i].z = vertex[i].z;
	}

	for (size_t i = 0; i < uvs.size(); ++i)
	{
		uvData[i].u = uvs[i].x;
		uvData[i].v = uvs[i].y;
	}

	Vertex* src = &vertexData[0];
	Vertex* dst = src + refiner->GetLevel(0).GetNumVertices();

	FVarUV* uvSrc = &uvData[0];
	FVarUV* uvDst = uvSrc + refiner->GetLevel(0).GetNumFVarValues(0);

	OpenSubdiv::Far::PrimvarRefiner primvarRefiner(*refiner);

	for (int level = 1; level <= refiner->GetMaxLevel(); ++level)
	{
		primvarRefiner.Interpolate(level, src, dst);
		primvarRefiner.InterpolateFaceVarying(level, uvSrc, uvDst, 0);

		src = dst;
		dst += refiner->GetLevel(level).GetNumVertices();

		uvSrc = uvDst;
		uvDst += refiner->GetLevel(level).GetNumFVarValues(0);
	}

	int numVertices = refiner->GetLevel(refiner->GetMaxLevel()).GetNumVertices();
	int numUvs = refiner->GetLevel(refiner->GetMaxLevel()).GetNumFVarValues(0);

	outPositions.clear();
	outPositions.reserve(numVertices);

	outUvs.clear();
	outUvs.reserve(numUvs);

	for (int i = 0; i < numVertices; ++i)
	{
		Vector3D<float> pos(src[i].x, src[i].y, src[i].z);
		outPositions.push_back(pos);
	}

	for (int i = 0; i < numUvs; ++i)
	{
		Point<float> uv(uvSrc[i].u, uvSrc[i].v);
		outUvs.push_back(uv);
	}

	const OpenSubdiv::Far::TopologyLevel& maxLevel = refiner->GetLevel(refiner->GetMaxLevel());
	int numFaces = maxLevel.GetNumFaces();

	outCornerCounts.clear();
	outPositionIndices.clear();
	outUvIndices.clear();

	for (int i = 0; i < numFaces; ++i)
	{
		OpenSubdiv::Far::ConstIndexArray faceVerts = maxLevel.GetFaceVertices(i);
		OpenSubdiv::Far::ConstIndexArray faceUvs = maxLevel.GetFaceFVarValues(i, 0);

		outCornerCounts.push_back(faceVerts.size());

		for (int j = 0; j < faceVerts.size(); ++j)
		{
			outPositionIndices.push_back(faceVerts[j]);
		}

		for (int j = 0; j < faceUvs.size(); ++j)
		{
			outUvIndices.push_back(faceUvs[j]);
		}
	}

	return true;
}

bool Scene::displace(Surface& surface, std::vector<Vector3D<float>>& displacedVertices, const std::vector<Point<float>>& uvs, const std::vector<int>& cornerCounts, const std::vector<int>& positionIndices, const std::vector<int>& uvIndices, const std::vector<Vector3D<float>>& normals)
{
	std::vector<bool> visited(displacedVertices.size(), false);

	for (size_t i = 0; i < positionIndices.size(); ++i)
	{
		int v = positionIndices[i];

		if (visited[v]) { continue; }

		Vector3D<float> src = displacedVertices[v];
		Vector3D<float> dst(0.0f, 0.0f, 0.0f);
		
		float dispMap = 0;

		if (!surface.getDisplacementSample(uvs[uvIndices[i]], dispMap)) { return false; }
		
		dst = Vector3D<float>(src.x + (normals[v].x * dispMap), src.y + (normals[v].y * dispMap), src.z + (normals[v].z * dispMap));

		displacedVertices[v] = dst;

		visited[v] = true;
	}

	return true;
}

bool Scene::computeEdgeRate(OpenSubdiv::Far::TopologyRefiner* refiner, const std::vector<Vector3D<float>>& vertexList, const float& targetPixelSize, const int& maxSubdivision, std::vector<int>& outEdgeRate)
{
	if (refiner == nullptr || camera == nullptr) { return false; }

	const OpenSubdiv::Far::TopologyLevel& originalTopology = refiner->GetLevel(0);
	const int numberEdges = originalTopology.GetNumEdges();

	outEdgeRate.resize(numberEdges, 1);

	const int cap = 1 << maxSubdivision;
	
	const float offScreenFactor = 0.25f;

	for (int i = 0; i < numberEdges; i++)
	{
		OpenSubdiv::Far::ConstIndexArray vertices = originalTopology.GetEdgeVertices(i);

		const Vector3D<float>& v1 = vertexList[vertices[0]];
		const Vector3D<float>& v2 = vertexList[vertices[1]];

		float x1, x2, y1, y2;

		bool v1Check = getCamera()->pointToPixel(v1, x1, y1);
		bool v2Check = getCamera()->pointToPixel(v2, x2, y2);

		if (!v1Check && !v2Check)
		{
			continue;
		}
		else if (v1Check && v2Check)
		{
			float pixelDistance = std::sqrt(((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)));
			float edgeSubdiv = pixelDistance / targetPixelSize;

			bool offScreen = (x1 < 0.0f && x2 < 0.0f) || (x1 > width - 1 && x2 > width - 1) || (y1 < 0.0f && y2 < 0.0f) || (y1 > height - 1 && y2 > height - 1);

			bool backFacing = true;
			OpenSubdiv::Far::ConstIndexArray edgeFaces = originalTopology.GetEdgeFaces(i);

			for (int j = 0; j < edgeFaces.size(); j++)
			{
				OpenSubdiv::Far::ConstIndexArray faceVertices =  originalTopology.GetFaceVertices(edgeFaces[j]);

				const Vector3D<float>& c0 = vertexList[faceVertices[0]];
				const Vector3D<float>& c1 = vertexList[faceVertices[1]];
				const Vector3D<float>& c2 = vertexList[faceVertices[2]];
				const Vector3D<float>& cLast = vertexList[faceVertices[faceVertices.size() - 1]];

				Vector3D<float> n = (c2 - c0) | (cLast - c1);
				Vector3D<float> vFromCamera = c0 - getCamera()->getPosition();

				if (n * vFromCamera <= 0.0f)
				{
					backFacing = false;
					break;
				}
			}

			if (offScreen || backFacing)
			{
				edgeSubdiv *= offScreenFactor;
			}

			edgeSubdiv = std::ceil(edgeSubdiv);
			edgeSubdiv = std::clamp(edgeSubdiv, 1.0f, static_cast<float>(cap));

			outEdgeRate[i] = static_cast<int>(edgeSubdiv);
		}
		else
		{
			outEdgeRate[i] = cap;

			continue;
		}
	}

	return true;
}

bool Scene::tasselateBfr(const OpenSubdiv::Far::TopologyRefiner* refiner, const std::vector<Vector3D<float>>& vertexList, const std::vector<Point<float>>& uvList, const std::vector<int>& edgeRates, std::vector<Vector3D<float>>& positions, std::vector<Point<float>>& UVs, std::vector<int>& cornerCounts, std::vector<int>& positionIndices, std::vector<int>& UVIndices, std::vector<Vector3D<float>>& normals)
{
	if (refiner == nullptr) { return false; }

	const OpenSubdiv::Far::TopologyLevel& originalTopology = refiner->GetLevel(0);

	if (originalTopology.GetNumEdges() != static_cast<int>(edgeRates.size())) { return false; }

	OpenSubdiv::Bfr::RefinerSurfaceFactory<>::Options factoryOption;
	factoryOption.SetDefaultFVarID(0);

	OpenSubdiv::Bfr::RefinerSurfaceFactory<> factory(*refiner, factoryOption);

	OpenSubdiv::Bfr::Surface<float> positionsSurface;
	OpenSubdiv::Bfr::Surface<float> uvSurface;
	std::vector<float> positionPatchPoints;
	std::vector<float> uvPatchPoints;
	std::vector<int> faceRate;
	std::vector<int> facets;

	std::vector<int> cornerTable(originalTopology.GetNumVertices(), -1);
	std::vector<int> edgeStartTable(originalTopology.GetNumEdges(), -1);
	std::vector<int> edgeDirTable(originalTopology.GetNumEdges(), -1);

	std::vector<int> borderIndices;

	OpenSubdiv::Bfr::Tessellation::Options triangleShapeOption;
	triangleShapeOption.SetFacetSize(3);

	std::vector<float> coordinates;

	const OpenSubdiv::Bfr::Surface<float>::PointDescriptor surfacePointGroup = 3;
	const OpenSubdiv::Bfr::Surface<float>::PointDescriptor uvsPointGroup = 2;

	for (int i = 0; i < originalTopology.GetNumFaces(); i++)
	{
		if (!factory.FaceHasLimitSurface(i)) { continue; }

		if (!factory.InitVertexSurface(i, &positionsSurface)) { return false; }
		if (!factory.InitFaceVaryingSurface(i, &uvSurface)) { return false; }

		positionPatchPoints.resize(positionsSurface.GetNumPatchPoints() * 3);
		uvPatchPoints.resize(uvSurface.GetNumPatchPoints() * 2);

		positionsSurface.PreparePatchPoints(&vertexList[0].x, surfacePointGroup, positionPatchPoints.data(), surfacePointGroup);
		uvSurface.PreparePatchPoints(&uvList[0].x, uvsPointGroup, uvPatchPoints.data(), uvsPointGroup);

		OpenSubdiv::Far::ConstIndexArray faceEdges = originalTopology.GetFaceEdges(i);
		faceRate.resize(faceEdges.size());

		for (int j = 0; j < faceEdges.size(); j++)
		{
			faceRate[j] = edgeRates[faceEdges[j]];
		}

		OpenSubdiv::Bfr::Tessellation tassellation(positionsSurface.GetParameterization(), static_cast<int>(faceRate.size()), faceRate.data(), triangleShapeOption);

		if (!tassellation.IsValid()) { return false; }

		coordinates.resize(tassellation.GetNumCoords() * 2);
		tassellation.GetCoords(coordinates.data());

		int firstUv = static_cast<int>(UVs.size());

		for (int z = 0; z < tassellation.GetNumCoords(); z++)
		{
			
			float evUvs[2];

			uvSurface.Evaluate(&coordinates[2 * z], uvPatchPoints.data(), uvsPointGroup, evUvs);
		
			Point<float> uvs(evUvs[0], evUvs[1]);

			UVs.push_back(uvs);
		}

		borderIndices.resize(tassellation.GetNumBoundaryCoords());
		OpenSubdiv::Far::ConstIndexArray faceVertices = originalTopology.GetFaceVertices(i);

		int b = 0;

		for (int k = 0; k < faceEdges.size(); k++)
		{
			if (cornerTable[faceVertices[k]] == -1)
			{
				int newIndex = static_cast<int>(positions.size());

				float boundaryPos[3];
				float dU[3];
				float dV[3];

				positionsSurface.Evaluate(&coordinates[2 * b], positionPatchPoints.data(), surfacePointGroup, boundaryPos, dU, dV);

				Vector3D<float> r(boundaryPos[0], boundaryPos[1], boundaryPos[2]);
				Vector3D<float> dUvec(dU[0], dU[1], dU[2]);
				Vector3D<float> dVvec(dV[0], dV[1], dV[2]);

				Vector3D<float> normal = dUvec | dVvec;
				float normalLength = normal.getLength();

				if (normalLength > 0.0f)
				{
					normal = normal / normalLength;
				}

				positions.push_back(r);
				normals.push_back(normal);
				
				cornerTable[faceVertices[k]] = newIndex;
			}

			borderIndices[b] = cornerTable[faceVertices[k]];
			b++;

			int numEdgePoints = faceRate[k] - 1;

			int edge = faceEdges[k];
			int startVertex = faceVertices[k];

			if (edgeStartTable[faceEdges[k]] == -1)
			{
				int newIndex = static_cast<int>(positions.size());

				edgeStartTable[edge] = newIndex;
				edgeDirTable[edge] = startVertex;

				for (int j = 0; j < numEdgePoints; j++)
				{
					float edgePos[3];
					float edgeDu[3];
					float edgeDv[3];

					positionsSurface.Evaluate(&coordinates[2 * (b + j)], positionPatchPoints.data(), surfacePointGroup, edgePos, edgeDu, edgeDv);

					Vector3D<float> r(edgePos[0], edgePos[1], edgePos[2]);
					Vector3D<float> edgeDuVec(edgeDu[0], edgeDu[1], edgeDu[2]);
					Vector3D<float> edgeDvVec(edgeDv[0], edgeDv[1], edgeDv[2]);

					Vector3D<float> edgeNormal = edgeDuVec | edgeDvVec;
					float edgeNormalLength = edgeNormal.getLength();

					if (edgeNormalLength > 0.0f)
					{
						edgeNormal = edgeNormal / edgeNormalLength;
					}

					positions.push_back(r);
					normals.push_back(edgeNormal);
				}
			}

			for (int j = 0; j < numEdgePoints; j++)
			{
				if (startVertex == edgeDirTable[edge])
				{
					borderIndices[b + j] = edgeStartTable[edge] + j;
				}
				else
				{
					borderIndices[b + j] = edgeStartTable[edge] + (numEdgePoints - 1 - j);
				}
			}

			b += numEdgePoints;
		}

		const int numBorderPoints = tassellation.GetNumBoundaryCoords();
		const int interiorStart = static_cast<int>(positions.size());

		for (int z = numBorderPoints; z < tassellation.GetNumCoords(); z++)
		{
			float interiorPos[3];
			float interiorDu[3];
			float interiorDv[3];

			positionsSurface.Evaluate(&coordinates[2 * z], positionPatchPoints.data(), surfacePointGroup, interiorPos, interiorDu, interiorDv);

			Vector3D<float> r(interiorPos[0], interiorPos[1], interiorPos[2]);
			Vector3D<float> interiorDuVec(interiorDu[0], interiorDu[1], interiorDu[2]);
			Vector3D<float> interiorDvVec(interiorDv[0], interiorDv[1], interiorDv[2]);

			Vector3D<float> interiorNormal = interiorDuVec | interiorDvVec;
			float interiorNormalLength = interiorNormal.getLength();

			if (interiorNormalLength > 0.0f)
			{
				interiorNormal = interiorNormal / interiorNormalLength;
			}

			positions.push_back(r);
			normals.push_back(interiorNormal);
		}

		facets.resize(tassellation.GetNumFacets() * 3);
		tassellation.GetFacets(facets.data());

		for (int f = 0; f < tassellation.GetNumFacets(); f++)
		{
			cornerCounts.push_back(3);

			for (int c = 0; c < 3; c++)
			{
				int m = facets[3 * f + c];

				int positionIndex = (m < numBorderPoints) ? borderIndices[m] : interiorStart + (m - numBorderPoints);

				positionIndices.push_back(positionIndex);
				UVIndices.push_back(firstUv + m);
			}
		}
	}

	return true;
}

bool Scene::setGammaCorrection(const std::string_view gc)
{
	std::string_view par = (gc.front() == '-') ? gc.substr(1) : gc;

	if (gammaCorrectionMap.find(par) == gammaCorrectionMap.end() || !gammaCorrectionEnabled)
	{
		return false;
	}

	gammaCorrection = gammaCorrectionMap[par];
	gammaCorrectionSet = true;

	return true;
}

bool Scene::setWindow(const std::string_view wSize)
{
	float w = 0.0f;
	float h = 0.0f;

	std::string_view token = (wSize.front() == '-') ? wSize.substr(1) : wSize;

	size_t sep = token.find(",");

	if (sep == std::string_view::npos) { return false; }

	std::string_view w_token = token.substr(0, sep);
	std::string_view h_token = token.substr(sep + 1);

	if (!parseFloat(w_token, w) || !parseFloat(h_token, h)) { return false; }

	width = w;
	height = h;

	return true;
}

bool Scene::setSamples(const std::string_view s)
{
	float samples = 0.0f;
	std::string_view samples_token = (s.front() == '-') ? s.substr(1) : s;

	if (!parseFloat(samples_token, samples)) { return false; };

	if (samples < 1.0f) { return false; }

	numberOfSamples = static_cast<int>(samples);

	return true;
}

bool Scene::setBounces(const std::string_view b)
{
	float bncs = 0.0f;
	std::string_view bounces_token = (b.front() == '-') ? b.substr(1) : b;

	if (!parseFloat(bounces_token, bncs)) { return false; };

	bounces = static_cast<int>(bncs);

	return true;
}

bool Scene::setDiffuseAOV(const std::string_view d)
{
	if (d.empty())
	{
		return false;
	}

	std::string_view diffuse_token = (d.front() == '-') ? d.substr(1) : d;

	if (diffuse_token == "d")
	{
		std::cout << "Diffuse AOV is enabled!" << std::endl;
		diffuseAOVEnabled = true;
	}

	return false;
}

bool Scene::setSpecularAOV(const std::string_view s)
{
	if (s.empty())
	{
		return false;
	}

	std::string_view specular_token = (s.front() == '-') ? s.substr(1) : s;

	if (specular_token == "s")
	{
		std::cout << "Specular AOV is enabled!" << std::endl;
		specularAOVEnabled = true;
	}

	return false;
}

bool Scene::setEmissionAOV(const std::string_view e)
{
	if (e.empty())
	{
		return false;
	}

	std::string_view emission_token = (e.front() == '-') ? e.substr(1) : e;

	if (emission_token == "e")
	{
		std::cout << "Emission AOV is enabled!" << std::endl;
		emissionAOVEnabled = true;
	}

	return false;
}

bool Scene::setRefractionAOV(const std::string_view r)
{
	if (r.empty())
	{
		return false;
	}

	std::string_view refraction_token = (r.front() == '-') ? r.substr(1) : r;

	if (refraction_token == "r")
	{
		std::cout << "Refraction AOV is enabled!" << std::endl;
		refractionAOVEnabled = true;
	}

	return false;
}

bool Scene::setSubsurfaceScatteringAOV(const std::string_view sb)
{
	if (sb.empty())
	{
		return false;
	}

	std::string_view sss_token = (sb.front() == '-') ? sb.substr(1) : sb;

	if (sss_token == "sb")
	{
		std::cout << "Subsurface Scattering AOV is enabled!" << std::endl;
		subsurfaceScatteringAOVEnabled = true;
	}

	return false;
}

bool Scene::setNormalAOV(const std::string_view n)
{
	if (n.empty())
	{
		return false;
	}

	std::string_view normal_token = (n.front() == '-') ? n.substr(1) : n;

	if (normal_token == "n")
	{
		std::cout << "Normal AOV is enabled!" << std::endl;
		normalAOVEnabled = true;
	}

	return false;
}

bool Scene::setDirectLightAOV(const std::string_view d)
{
	if (d.empty())
	{
		return false;
	}

	std::string_view directLight_token = (d.front() == '-') ? d.substr(1) : d;

	if (directLight_token == "dl")
	{
		std::cout << "Direct Light AOV is enabled!" << std::endl;
		directLightAOVEnabled = true;
	}
	return false;
}

bool Scene::setDepthAOV(const std::string_view d)
{
	if (d.empty())
	{
		return false;
	}

	std::string_view depth_token = (d.front() == '-') ? d.substr(1) : d;

	if (depth_token == "dp")
	{
		std::cout << "Depth AOV is enabled!" << std::endl;
		depthAOVEnabled = true;
	}
	return false;
}

bool Scene::setPositionAOV(const std::string_view p)
{
	if (p.empty())
	{
		return false;
	}

	std::string_view position_token = (p.front() == '-') ? p.substr(1) : p;

	if (position_token == "pos")
	{
		std::cout << "Position AOV is enabled!" << std::endl;
		positionAOVEnabled = true;
	}
	
	return false;
}

bool Scene::setIndirectAOV(const std::string_view i)
{
	if (i.empty())
	{
		return false;
	}

	std::string_view indirect_token = (i.front() == '-') ? i.substr(1) : i;

	if (indirect_token == "il")
	{
		std::cout << "Indirect AOV is enabled!" << std::endl;
		indirectAOVEnabled = true;
	}

	return false;
}

// Sets the file path for writing the rendered output.
bool Scene::setFilePathWrite(const std::string_view& path)
{
	std::string_view par = (path.front() == '-') ? path.substr(1) : path;

	if (par.empty())
	{
		std::cout << "Invalid file path for writing!" << std::endl;
		return false;
	}

	size_t dot = par.rfind('.');

	if (dot == std::string_view::npos)
	{
		std::cout << "No extension found in file path!" << std::endl;
		return false;
	}

	std::string_view extension = par.substr(dot + 1);

	auto it = renderOutputMap.find(extension);

	if (it == renderOutputMap.end())
	{
		std::cout << "Unsupported file extension!" << std::endl;
		return false;
	}

	renderOutput = it->second;

	if (renderOutput == RenderOutput::EXR)
	{
		gammaCorrectionEnabled = false;
	}

	filePathWrite = par;

	return true;
}

bool Scene::displacementGeo(Mesh& mesh)
{
	std::vector<int> cornersCounts;
	std::vector<int> positions;
	std::vector<int> uvs;

	std::vector<Vector3D<float>> outPositions;
	std::vector<Point<float>> outUvs;
	std::vector<int> outCornerCounts;
	std::vector<int> outPositionIndices;
	std::vector<int> outUvIndices;

	std::vector<int> edgeRates;

	std::vector<Vector3D<float>> normals;
	std::vector<Vector3D<float>> smoothNormals;

	int numberOfVerticesTotal = 0;
	int numberOfUvTotal = 0;
	std::string ext = std::filesystem::path(mesh.getObjPath()).extension().string();
	int level = mesh.getMaxSubdivisions();

	Surface* surface = std::get_if<Surface>(&mesh.getShader());

	if (!surface) { return false; }

	OpenSubdiv::Far::TopologyRefiner* refiner;

	if (!getCorners(mesh.getObjPath(), cornersCounts, positions, uvs, numberOfVerticesTotal, numberOfUvTotal, ext)) { return false; }

	refiner = sendOpenSubdiv(numberOfVerticesTotal, cornersCounts, positions, numberOfUvTotal, uvs, 0);
	if (!refiner) { return false; }

	if (!computeEdgeRate(refiner, mesh.getVertexList(), mesh.getTargetPixelSize(), level, edgeRates))
	{
		delete refiner;
		return false;
	}

	if (!tasselateBfr(refiner, mesh.getVertexList(), mesh.getUvList(), edgeRates, outPositions, outUvs, outCornerCounts, outPositionIndices, outUvIndices, smoothNormals))
	{
		delete refiner;
		return false; 
	}

	delete refiner;

	if (!displace(*surface, outPositions, outUvs, outCornerCounts, outPositionIndices, outUvIndices, smoothNormals)) { return false; }

	if (!computeVertexNormals(outPositions, outCornerCounts, outPositionIndices, normals)) { return false; }

	if (!mesh.rebuildTriangles(outPositions, outUvs, normals, outCornerCounts, outPositionIndices, outUvIndices)) { return false; }

	return true;
}

// Renders the scene.
void Scene::render()
{
	output.setRenderOutput(getRenderOutput());
	output.setFilePathWrite(getFilePathWrite());
	output.setWidth(getWidth());
	output.setHeight(getHeight());
	output.allocateBuffer();

	if (diffuseAOVEnabled)
	{
		diffuseLayer = output.addLayer("diffuse");
	}

	if (specularAOVEnabled)
	{
		specularLayer = output.addLayer("specular");
	}

	if (emissionAOVEnabled)
	{
		emissionLayer = output.addLayer("emission");
	}

	if (refractionAOVEnabled)
	{
		refractionLayer = output.addLayer("refraction");
	}

	if (subsurfaceScatteringAOVEnabled)
	{
		subsurfaceScatteringLayer = output.addLayer("subsurface_scattering");
	}

	if (normalAOVEnabled)
	{
		normalLayer = output.addLayer("normal");
	}

	if (directLightAOVEnabled)
	{
		directLightLayer = output.addLayer("direct_light");
	}

	if (depthAOVEnabled)
	{
		depthLayer = output.addLayer("Z");
	}

	if (positionAOVEnabled)
	{
		positionLayer = output.addLayer("P");
	}

	if (indirectAOVEnabled)
	{
		indirectLayer = output.addLayer("indirect");
	}

	camera->setWindow(getWidth(), getHeight());
	
	std::cout << "camera position : " << camera->getPosition().x << " " << camera->getPosition().y << " " << camera->getPosition().z << std::endl;

	std::cout << camera->getWidth() << " " << camera->getHeight() << std::endl;

	float width = camera->getWidth();
	float height = camera->getHeight();

	BVH bvh;
	bvh.buildBVH(geometries);

	auto renderRow = [&](int i, Integrator& integrator)
		{
			for (int j = 0; j < width; ++j)
			{
				integrator.seed(static_cast<uint32_t>(i * (int)width + j));

				float u = (float)j / (width - 1);
				float v = (float)i / (height - 1);

				Vector3D<float> color(0.0f, 0.0f, 0.0f);
				Vector3D<float> diffuseAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> specularAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> emissionAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> refractionAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> subsurfaceScatteringAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> normalAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> directLightAOV(0.0f, 0.0f, 0.0f);
				Vector3D<float> depthAOV(1.0e30f, 0.0f, 0.0f);
				Vector3D<float> positionAOV(1.0e30f, 1.0e30f, 1.0e30f);
				Vector3D<float> indirectAOV(0.0f, 0.0f, 0.0f);
				float alpha = 0.0f;

				Ray ray(camera->genRay(u, v));

				Ray originalRay = ray;

				for (size_t k = 0; k < numberOfSamples; ++k)
				{
					float x = (originalRay.getDirection().x + (integrator.getUnitRandom().Generate() - 0.5f) / width);
					float y = (originalRay.getDirection().y + (integrator.getUnitRandom().Generate() - 0.5f) / height);
					float z = (originalRay.getDirection().z + (integrator.getUnitRandom().Generate() - 0.5f) / width);

					ray.setDirection(Vector3D<float>(x, y, z));

					color += integrator.rayPath(ray, bvh, bounces, true, true, 0.0f, &diffuseAOV, &specularAOV, &emissionAOV, &refractionAOV, &subsurfaceScatteringAOV, &alpha, &normalAOV, &directLightAOV, &depthAOV, &positionAOV, &indirectAOV);
				}

				color /= (float)numberOfSamples;
				diffuseAOV /= (float)numberOfSamples;
				specularAOV /= (float)numberOfSamples;
				emissionAOV /= (float)numberOfSamples;
				refractionAOV /= (float)numberOfSamples;
				subsurfaceScatteringAOV /= (float)numberOfSamples;
				normalAOV /= (float)numberOfSamples;
				directLightAOV /= (float)numberOfSamples;
				indirectAOV /= (float)numberOfSamples;
				alpha /= (float)numberOfSamples;

				if (gammaCorrectionSet)
				{
					switch (gammaCorrection)
					{
					case GammaCorrection::GAMMA2:
						color.x = std::sqrt(color.x);
						color.y = std::sqrt(color.y);
						color.z = std::sqrt(color.z);
						break;

					default:
						break;
					}
				}

				output.writeBuffer(static_cast<size_t>((height - 1 - i) * width + j), color);
				output.writeAlpha(static_cast<size_t>((height - 1 - i) * width + j), alpha);

				if (diffuseAOVEnabled)
				{
					output.writeLayer(diffuseLayer, static_cast<size_t>((height - 1 - i) * width + j), diffuseAOV);
				}
				
				if (specularAOVEnabled)
				{
					output.writeLayer(specularLayer, static_cast<size_t>((height - 1 - i) * width + j), specularAOV);
				}

				if (emissionAOVEnabled)
				{
					output.writeLayer(emissionLayer, static_cast<size_t>((height - 1 - i) * width + j), emissionAOV);
				}

				if (refractionAOVEnabled)
				{
					output.writeLayer(refractionLayer, static_cast<size_t>((height - 1 - i) * width + j), refractionAOV);
				}

				if (subsurfaceScatteringAOVEnabled)
				{
					output.writeLayer(subsurfaceScatteringLayer, static_cast<size_t>((height - 1 - i) * width + j), subsurfaceScatteringAOV);
				}

				if (normalAOVEnabled)
				{
					output.writeLayer(normalLayer, static_cast<size_t>((height - 1 - i) * width + j), normalAOV);
				}

				if (directLightAOVEnabled)
				{
					output.writeLayer(directLightLayer, static_cast<size_t>((height - 1 - i) * width + j), directLightAOV);
				}

				if (depthAOVEnabled)
				{
					output.writeLayer(depthLayer, static_cast<size_t>((height - 1 - i) * width + j), depthAOV);
				}

				if (positionAOVEnabled)
				{
					output.writeLayer(positionLayer, static_cast<size_t>((height - 1 - i) * width + j), positionAOV);
				}

				if (indirectAOVEnabled)
				{
					output.writeLayer(indirectLayer, static_cast<size_t>((height - 1 - i) * width + j), indirectAOV);
				}
			}
		};

	unsigned int threadCount = std::thread::hardware_concurrency();

	if (threadCount == 0)
	{
		threadCount = 4;
	}

	std::atomic<int> nextRow{ 0 };

	std::vector<std::thread> threads;

	for (unsigned int t = 0; t < threadCount; ++t)
	{
		threads.emplace_back([&]()
			{
				Integrator integrator(lights);
				integrator.addAreaLights(areaLights);
				integrator.addMeshLights(meshLights);
				integrator.addInfiniteLights(infiniteLights);
				integrator.setCameraForward(camera->getForward());

				int i;

				while ((i = nextRow++) < (int)height)
				{
					renderRow(i, integrator);
				}
			});
	}

	for (std::thread& t : threads)
	{
		t.join();
	}

	output.write();
}