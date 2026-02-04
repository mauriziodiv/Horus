#pragma once
#include "hrs.h"

enum class RenderOutput {
	PPM
};

class Scene {

	public:
		Scene();
		bool getScene(std::vector<SceneObject*>);

		CameraObject* getCamera() { return camera; }
		std::vector<GeometryObject*> getGeometries() { return geometries; }
		std::vector<LightObject*> getLights() { return lights; }
		void setRenderOutput(std::string);
		RenderOutput getRenderOutput() { return renderOutput; }
		void render();

	private:
		std::vector<SceneObject*> sceneObjects;
		CameraObject* camera;
		std::vector<GeometryObject*> geometries;
		std::vector<LightObject*> lights;

		bool cameraCheck();
		bool geometriesCheck();
		bool lightCheck();

		RenderOutput renderOutput = RenderOutput::PPM;
};