#include "HybridManager.h"
#include "HybridEntity.h"

namespace Hybrid {
	Entity::Entity(Ogre::String name, Ogre::MeshPtr pMesh) : Ogre::Entity(name, pMesh) {
		mSubEntityList.clear();
		buildSubEntityList(pMesh,&mSubEntityList);
	}

	Entity::~Entity() {
	}

	void Entity::setMaterialName(const Ogre::String& name, const Ogre::String& groupName) {
        for (SubEntityList::iterator i = mSubEntityList.begin(); i != mSubEntityList.end(); ++i) {
			((SubEntity*)(*i))->setMaterialName(name, groupName);
        }
    }


	void Entity::setMaterial(const Ogre::MaterialPtr& material) {
		for(SubEntityList::iterator i=mSubEntityList.begin();i!=mSubEntityList.end();++i) {
			((SubEntity*)(*i))->setMaterial(material);
		}
	}

	void Entity::setHybridMaterial(Material* material) {
		for(SubEntityList::iterator i=mSubEntityList.begin();i!=mSubEntityList.end();++i) {
			((SubEntity*)(*i))->setHybridMaterial(material);
		}
	}

	//Protected
	void Entity::buildSubEntityList(Ogre::MeshPtr& mesh, Ogre::Entity::SubEntityList* sublist) {
		unsigned short i, numSubMeshes;
		Ogre::SubMesh* subMesh;
		Ogre::SubEntity* subEnt;
	
		numSubMeshes = mesh->getNumSubMeshes();
		for(i=0; i<numSubMeshes; ++i) {
			subMesh = mesh->getSubMesh(i);
			subEnt = OGRE_NEW SubEntity(this, subMesh);
			if(subMesh->isMatInitialised()) {
				subEnt->setMaterialName(subMesh->getMaterialName());
				((SubEntity*)subEnt)->_initHybridMaterial();
			}
			sublist->push_back(subEnt);
		}
	}

	void Entity::setRaytraced(bool isRaytraced) {
		//if(HybridManager::getSingleton()->isPluginEnabled()) {
			if(isRaytraced) {
				for(int i=0;i<mSubEntityList.size();++i) {
					((SubEntity*)mSubEntityList[i])->setRaytraced(true);
				}
			} else {
				for(int i=0;i<mSubEntityList.size();++i) {
					((SubEntity*)mSubEntityList[i])->setRaytraced(false);
				}
			}
		//} else {
		//	Ogre::LogManager::getSingleton().logMessage("HybridPlugin: Plugin not active, cannot enable raytracing of an entity.");
		//}
	}
}