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
		bool setGammaCorrection(const std::string_view gc);
		bool setWindow(const std::string_view wSize);
		bool setSamples(const std::string_view s);
		bool setBounces(const std::string_view b);
		bool setDiffuseAOV(const std::string_view d);
		bool setSpecularAOV(const std::string_view s);
		bool setEmissionAOV(const std::string_view e);
		bool setRefractionAOV(const std::string_view r);
		bool setSubsurfaceScatteringAOV(const std::string_view sss);

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
		size_t diffuseLayer = 0;
		size_t specularLayer = 0;
		size_t emissionLayer = 0;
		size_t refractionLayer = 0;
		size_t subsurfaceScatteringLayer = 0;
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
};