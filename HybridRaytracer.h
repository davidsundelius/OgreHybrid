#pragma once
#include "HybridScene.h"
#include "HybridNode.h"
#include <Ogre.h>

namespace Hybrid {
	class Raytracer {
		public:
			Raytracer(int width, int height, float fovx, float fovy, float focal, Ogre::SceneManager* sceneMgr);
			~Raytracer();

			void render(float* camPos, float* u, float* v, float* w);
			void resize(int width, int height);
			void setHybridMode(bool hybridMode);
			void setSuperSampling(unsigned int numSamples);
			void setTransferMode(bool tm);
			void updateLights();
			optix::Context getOptixContext();

		private:
			Ogre::HardwareVertexBufferSharedPtr vbo;
			GLuint vboId;

			GLuint target;
			optix::TextureSampler mask;
			Ogre::TexturePtr targetPtr;

			int width;
			int height;
			float fovx;
			float fovy;
			float focal;

			bool hybridMode;
			int numSamples;
			bool transferMode;

			optix::Context context;
			optix::Program generatorProgram;
			optix::Program exceptionProgram;
			optix::Program missProgram;

			optix::Buffer resultBuffer;

			Ogre::SceneManager* sceneMgr;
			Scene* scene;
			Model* sphere1;
			Model* sphere2;
			Model* sphere3;
			Model* plane;
			Model* dragon;
	
			void initialize();
			void buildModels();
			void buildScene(Ogre::SceneManager* sceneMgr);
	};
}