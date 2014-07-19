#pragma once
#include "HybridPluginPrerequisites.h"
#include <Ogre.h>
#include "HybridSubEntity.h"

namespace Hybrid {
	class _HybridExport Entity : public Ogre::Entity {
		public:
			Entity(Ogre::String name, Ogre::MeshPtr pMesh);
			~Entity();

			void setRaytraced(bool isRaytraced);
			void Entity::setMaterialName(const Ogre::String& name, const Ogre::String& groupName = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
			void Entity::setMaterial(const Ogre::MaterialPtr& material);
			void Entity::setHybridMaterial(Material* material);
		protected:
			void buildSubEntityList(Ogre::MeshPtr& mesh, Ogre::Entity::SubEntityList* sublist);
	};
}