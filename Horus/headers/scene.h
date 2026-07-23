#pragma once
#include "hrs.h"
#include "output.h"
#include "render.h"
#include "accelerator.h"
#include <charconv>

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
		bool setRenderOutput(const std::string_view& ro);
		bool setGammaCorrection(const std::string_view gc);
		bool setWindow(const std::string_view wSize);
		bool setFocalLength(const std::string_view fl);

		float getWidth() { return width; }
		float getHeight() { return height; }

		bool getGammaCorrectionSet() { return gammaCorrectionSet; }
		RenderOutput getRenderOutput() { return renderOutput; }
		void render();
		bool setFilePathWrite(const std::string_view& path);
		const std::string_view& getFilePathWrite() { return filePathWrite; }

	private:
		std::vector<std::unique_ptr<SceneObject>> sceneObjects;
		CameraObject* camera;
		std::vector<GeometryObject*> geometries;
		std::vector<LightObject*> lights;
		std::vector<AreaLight*> areaLights;
		std::vector<MeshLight*> meshLights;

		bool cameraCheck();
		bool geometriesCheck();
		bool lightCheck();

		RenderOutput renderOutput = RenderOutput::PPM;
		Output output;
		std::string_view filePathWrite;

		UnitRandom unitRandom;

		bool gammaCorrectionSet = false;
		GammaCorrection gammaCorrection = GammaCorrection::GAMMA2;

		int32_t numberOfSamples = 10; //100
		float width = 400.0f;
		float height = 400.0f;
		float focal_length = 35.0f;
};