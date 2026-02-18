#pragma once
#include "hrs.h"
#include "output.h"

class Scene {

	public:
		Scene();
		bool getScene(const std::vector<SceneObject*>& scene);

		CameraObject* getCamera() { return camera; }
		std::vector<GeometryObject*> getGeometries() { return geometries; }
		std::vector<LightObject*> getLights() { return lights; }
		bool setRenderOutput(const std::string_view& ro);
		RenderOutput getRenderOutput() { return renderOutput; }
		void render();
		bool setFilePathWrite(const std::string_view& path);
		const std::string_view& getFilePathWrite() { return filePathWrite; }

	private:
		std::vector<SceneObject*> sceneObjects;
		CameraObject* camera;
		std::vector<GeometryObject*> geometries;
		std::vector<LightObject*> lights;

		bool cameraCheck();
		bool geometriesCheck();
		bool lightCheck();

		RenderOutput renderOutput = RenderOutput::PPM;
		Output output;
		std::string_view filePathWrite;
};