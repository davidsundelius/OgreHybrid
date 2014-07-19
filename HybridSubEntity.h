#pragma once
#include "HybridPluginPrerequisites.h"
#include "HybridMaterial.h"
#include <Ogre.h>

namespace Hybrid {
	class _HybridExport SubEntity : public Ogre::SubEntity {
		public:
			SubEntity(Ogre::Entity* parent, Ogre::SubMesh* subMeshBasis);
			~SubEntity();
			void setRaytraced(bool isRaytraced);
			bool getRaytraced();
			void _initHybridMaterial();
			void setMaterialName(const Ogre::String& name, const Ogre::String& groupName = Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);
			void setMaterial(const Ogre::MaterialPtr& material);
			Material* getHybridMaterial();
			void setHybridMaterial(Material* material);
		private:
			bool isRaytraced;
	};
}