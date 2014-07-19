#pragma once
#ifndef NOMINMAX
	#define NOMINMAX 0
#endif
#include <Optix\optix_math.h>
#include "HybridNode.h"
#include "common.h"
#include <Ogre.h>

namespace Hybrid {
	class Scene {
		public:
			Scene(optix::Context context);
			~Scene();

			void initialize();
			Node* getNode(int i);
			optix::Group getScene();

			void parseScene(Ogre::SceneManager* sceneMgr);
			void reparseLights();

			int addNode(Node* n);
			int addLight(glm::vec3 l);
		private:
			static Ogre::TextureUnitState* whiteMask;
			static Ogre::TextureUnitState* blackMask;
			bool isInit;
			optix::Context context;
			optix::Buffer light_buffer;
			std::vector<Node*> nodes;
			std::vector<Light> lights;
			optix::Group top;
			optix::Acceleration acc;
			Ogre::SceneManager* sceneMgr;

			void parseNode(Ogre::SceneNode* node);
			void parseLight(Ogre::Light* light);
	};
}