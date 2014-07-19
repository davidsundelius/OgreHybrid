#pragma once
#include <Ogre.h>

namespace Hybrid {
	class EntityFactory : public Ogre::EntityFactory {
		public:
			EntityFactory();
			~EntityFactory();
			virtual Ogre::MovableObject* createInstance (const Ogre::String &name, Ogre::SceneManager *manager, const Ogre::NameValuePairList *params);
			virtual Ogre::MovableObject* EntityFactory::createInstanceImpl(const Ogre::String& name, const Ogre::NameValuePairList* params);
	};
}