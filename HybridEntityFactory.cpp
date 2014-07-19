#include "HybridEntityFactory.h"
#include "HybridEntity.h"
#include <OgreException.h>

namespace Hybrid {
	EntityFactory::EntityFactory() : Ogre::EntityFactory() {
	}

	EntityFactory::~EntityFactory() {
	}

	Ogre::MovableObject* EntityFactory::createInstance (const Ogre::String &name, Ogre::SceneManager *manager, const Ogre::NameValuePairList *params=0) {
		Ogre::MovableObject* entity = createInstanceImpl(name, params);
		entity->_notifyCreator(this);
		entity->_notifyManager(manager);
		return entity;
	}

	Ogre::MovableObject* EntityFactory::createInstanceImpl(const Ogre::String& name, const Ogre::NameValuePairList* params) {
		Ogre::MeshPtr pMesh;
        if (params != 0) {
			Ogre::NameValuePairList::const_iterator ni = params->find("mesh");
			if (ni != params->end()) {
				pMesh = Ogre::MeshManager::getSingleton().load(ni->second, Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );
            }
		}
        if (pMesh.isNull()) {
			OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "'mesh' parameter required when constructing an HybridEntity.", "HybridEntityFactory::createInstance");
        }
		Entity* entity = OGRE_NEW Entity(name, pMesh);
		
		return entity;
	}
}