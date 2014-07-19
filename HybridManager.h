#pragma once
#include "HybridPluginPrerequisites.h"
#include "HybridRaytracer.h"
#include "HybridNode.h"
#include "HybridEntity.h"
#include "HybridMaterial.h"
#include <Ogre.h>

namespace Hybrid {
	class _HybridExport HybridManager : public Ogre::RenderTargetListener  {
		public:
			enum RenderMode {
				NORMAL,
				HYBRID,
				RAYTRACE
			};
			enum SuperSampling {
				SS_NONE,
				SS_4X
			};
			enum TransferMode {
				TM_NONE,
				TM_PBO
			};

			void initialize(Ogre::RenderWindow* window, Ogre::SceneManager* scene, Ogre::Camera* camera);
			void setRenderMode(RenderMode renderMode);
			RenderMode getRenderMode();
			void setSuperSampling(SuperSampling ss);
			void setTransferMode(TransferMode tm);
			void setDebug(bool debugMode);
			bool getDebug();
			bool isPluginEnabled();
			void updateLights();
			void updateMaterials();
			void addMaterial(Material* material);
			optix::Context getOptixContext();

			static HybridManager* getSingleton();
		protected:
			void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
			void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
		private:
			RenderMode renderMode;

			static HybridManager* instance;

			Ogre::RenderWindow* coreWindow;
			Ogre::SceneManager* coreScene;
			Ogre::Camera* coreCamera;
			Raytracer* rt;
			Ogre::RenderTexture* renderTexture;
			Ogre::Rectangle2D* hybridScreen;
			Ogre::SceneNode* hybridScreenNode;
			bool debugMode;
			bool pluginEnabled;
			Ogre::Rectangle2D* maskScreen;
			Ogre::Rectangle2D* rtScreen;
			std::vector<Material*> materials;

			HybridManager();
			~HybridManager();
	};
}