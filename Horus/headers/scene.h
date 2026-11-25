#pragma once
#include "hrs.h"
#include "output.h"
#include "render.h"
#include "accelerator.h"
#include <charconv>
#include <opensubdiv/far/topologyRefiner.h>

enum class GammaCorrection
{
	GAMMA2
};

class Scene {

	public:
		Scene();
		bool getScene(std::vector<std::unique_ptr<SceneObject>>& scene);

		CameraObject* getCamera() { return camera; }
		std::vector<GeometryObject*> getGeometries() { return geometries; }
		std::vector<LightObject*> getLights() { return lights; }
		bool setGammaCorrection(const std::string_view gc);
		bool setWindow(const std::string_view wSize);
		bool setSamples(const std::string_view s);
		bool setBounces(const std::string_view b);
		bool setDiffuseAOV(const std::string_view d);
		bool setSpecularAOV(const std::string_view s);
		bool setEmissionAOV(const std::string_view e);
		bool setRefractionAOV(const std::string_view r);
		bool setSubsurfaceScatteringAOV(const std::string_view sss);
		bool setNormalAOV(const std::string_view n);
		bool setDirectLightAOV(const std::string_view d);
		bool setDepthAOV(const std::string_view d);
		bool setPositionAOV(const std::string_view p);
		bool setIndirectAOV(const std::string_view i);

		float getWidth() { return width; }
		float getHeight() { return height; }

		bool getGammaCorrectionSet() { return gammaCorrectionSet; }
		RenderOutput getRenderOutput() { return renderOutput; }
		void render();
		bool setFilePathWrite(const std::string_view& path);
		const std::string_view& getFilePathWrite() { return filePathWrite; }

		bool displacementGeo(Mesh& mesh);

	private:
		std::vector<std::unique_ptr<SceneObject>> sceneObjects;
		CameraObject* camera;
		std::vector<GeometryObject*> geometries;
		std::vector<LightObject*> lights;
		std::vector<AreaLight*> areaLights;
		std::vector<MeshLight*> meshLights;
		std::vector<InfiniteLight*> infiniteLights;

		bool cameraCheck();
		bool geometriesCheck();
		bool lightCheck();

		bool computeVertexNormals(const std::vector<Vector3D<float>>& positions, const std::vector<int>& cornerCounts, const std::vector<int>& positionIndices, std::vector<Vector3D<float>>& outNormals);
		bool getCorners(const std::string& objPath, std::vector<int>& cornersCounts, std::vector<int>& positions, std::vector<int>& uvs, int& numberOfVerticesTotal, int& numberOfUvTotal, const std::string& ext);
		OpenSubdiv::Far::TopologyRefiner* sendOpenSubdiv(int numberOfVertices, const std::vector<int>& cornerCounts, const std::vector<int>& positions, int numberOfUvTotal, const std::vector<int>& uvs, int level);
		bool linkOpenSubdiv(OpenSubdiv::Far::TopologyRefiner* refiner, const std::vector<Vector3D<float>>& vertex, const std::vector<Point<float>>& uvs, std::vector<Vector3D<float>>& outPositions, std::vector<Point<float>>& outUvs, std::vector<int>& outCornerCounts, std::vector<int>& outPositionIndices, std::vector<int>& outUvIndices);
		bool displace(Surface& surface, std::vector<Vector3D<float>>& displacedVertices, const std::vector<Point<float>>& uvs, const std::vector<int>& cornerCounts, const std::vector<int>& positionIndices, const std::vector<int>& uvIndices, const std::vector<Vector3D<float>>& normals);
		bool computeEdgeRate(OpenSubdiv::Far::TopologyRefiner* refiner, const std::vector<Vector3D<float>>& vertexList, const float& targetPixelSize, const int& maxSubdivision, std::vector<int>& outEdgeRate);
		bool tasselateBfr(const OpenSubdiv::Far::TopologyRefiner* refiner, const std::vector<Vector3D<float>>& vertexList, const std::vector<Point<float>>& uvList, const std::vector<int>& edgeRates, std::vector<Vector3D<float>>& positions, std::vector<Point<float>>& UVs, std::vector<int>& cornerCounts, std::vector<int>& positionIndices, std::vector<int>& UVIndices, std::vector<Vector3D<float>>& normals);

		RenderOutput renderOutput = RenderOutput::PPM;
		Output output;
		size_t diffuseLayer = 0;
		size_t specularLayer = 0;
		size_t emissionLayer = 0;
		size_t refractionLayer = 0;
		size_t subsurfaceScatteringLayer = 0;
		size_t normalLayer = 0;
		size_t directLightLayer = 0;
		size_t depthLayer = 0;
		size_t positionLayer = 0;
		size_t indirectLayer = 0;

		std::string_view filePathWrite;

		bool gammaCorrectionSet = false;
		GammaCorrection gammaCorrection = GammaCorrection::GAMMA2;
		bool gammaCorrectionEnabled = true;

		int32_t numberOfSamples = 10; //100
		int bounces = 5;
		float width = 400.0f;
		float height = 400.0f;

		bool diffuseAOVEnabled = false;
		bool specularAOVEnabled = false;
		bool emissionAOVEnabled = false;
		bool refractionAOVEnabled = false;
		bool subsurfaceScatteringAOVEnabled = false;
		bool normalAOVEnabled = false;
		bool directLightAOVEnabled = false;
		bool depthAOVEnabled = false;
		bool positionAOVEnabled = false;
		bool indirectAOVEnabled = false;
};