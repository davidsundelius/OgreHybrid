#pragma once
#include <cstdlib>
#include <string>
#include <Optix/optixu/optixpp_namespace.h>
#include <glm/glm.hpp>
#include <Ogre/OgreNode.h>
#include "HybridModel.h"

namespace Hybrid {
	class Node : public Ogre::Node::Listener {
		public:
			Node(optix::Context context, Model* model);
			~Node();

			void move(glm::vec3 pos);
			void scale(glm::vec3 sc);
			void rotate(Ogre::Matrix3 rot);
			optix::Transform getTransformation();

		protected:
			
		private:
			bool isPrimitive;
			bool isRaytraced;
			optix::Context m_context;
			optix::Transform transform;
			optix::GeometryGroup group;
			optix::Acceleration acc;
			optix::GeometryInstance instance;
		
			glm::vec3 pos;
			glm::vec3 sc;
			glm::mat4 rot;

			void doTransform();
			virtual void nodeUpdated(const Ogre::Node* node);
			virtual void nodeAttached(const Ogre::Node* node) {};
			virtual void nodeDestroyed(const Ogre::Node* node) {};
			virtual void nodeDetached(const Ogre::Node* node) {};
	};
}