#include "HybridManager.h"
#include "HybridSubEntity.h"

namespace Hybrid {
	//Public
	SubEntity::SubEntity(Ogre::Entity* parent, Ogre::SubMesh* subMeshBasis) : Ogre::SubEntity(parent, subMeshBasis) {
		isRaytraced=false;
	}

	SubEntity::~SubEntity() {
	}

	void SubEntity::setRaytraced(bool isRaytraced) {
		//if(HybridManager::getSingleton()->isPluginEnabled()) {
			if(isRaytraced) {
				Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(getMaterialName());
				mat->getTechnique("MaskTechnique")->getPass(0)->getTextureUnitState(0)->setColourOperationEx(
						Ogre::LayerBlendOperationEx::LBX_SOURCE1, 
						Ogre::LayerBlendSource::LBS_MANUAL,
						Ogre::LayerBlendSource::LBS_CURRENT,
						Ogre::ColourValue::White);
			} else {
				Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(getMaterialName());
				mat->getTechnique("MaskTechnique")->getPass(0)->getTextureUnitState(0)->setColourOperationEx(
						Ogre::LayerBlendOperationEx::LBX_SOURCE1, 
						Ogre::LayerBlendSource::LBS_MANUAL,
						Ogre::LayerBlendSource::LBS_CURRENT,
						Ogre::ColourValue::Black);
			}
			this->isRaytraced=isRaytraced;
		//} else {
			//Ogre::LogManager::getSingleton().logMessage("HybridPlugin: Plugin not active, cannot enable raytracing of a subentity.");
		//}
	}

	bool SubEntity::getRaytraced() {
		return isRaytraced;
	}

	void SubEntity::_initHybridMaterial() {
		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(getMaterialName());
		//Add masking technique
		Ogre::Technique* t = mat->createTechnique();
		t->setName("MaskTechnique");
		t->setSchemeName("Mask");
		Ogre::Pass* p = t->createPass();
		p->setLightingEnabled(false);
		p->createTextureUnitState();
		setRaytraced(isRaytraced);
	}

	void SubEntity::setMaterialName(const Ogre::String& name, const Ogre::String& groupName) {
		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(getMaterialName());
		Ogre::Any hybridMaterial = mat->getTechnique(0)->getUserObjectBindings().getUserAny("HybridMaterial");
		Ogre::SubEntity::setMaterialName(name, groupName);
		mat->getTechnique(0)->getUserObjectBindings().setUserAny("HybridMaterial", hybridMaterial);
		_initHybridMaterial();
    }

	void SubEntity::setMaterial(const Ogre::MaterialPtr& material) {
		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(getMaterialName());
		Ogre::Any hybridMaterial = mat->getTechnique(0)->getUserObjectBindings().getUserAny("HybridMaterial");
		Ogre::SubEntity::setMaterial(material);
		material->getTechnique(0)->getUserObjectBindings().setUserAny("HybridMaterial", hybridMaterial);
		_initHybridMaterial();
	}

	Material* SubEntity::getHybridMaterial() {
		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(getMaterialName());
		Ogre::Any maybeMaterial = (mat->getTechnique(0)->getUserObjectBindings().getUserAny("HybridMaterial"));
		Material* hmat;
		if(maybeMaterial.isEmpty()) {
			hmat = new Material(HybridManager::getSingleton()->getOptixContext(), mat);
			HybridManager::getSingleton()->addMaterial(hmat);
			mat->getTechnique(0)->getUserObjectBindings().setUserAny("HybridMaterial", Ogre::Any(hmat));
		} else {
			hmat = Ogre::any_cast<Material*>(maybeMaterial);
		}
		return hmat;
	}

	void SubEntity::setHybridMaterial(Material* material){
		Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName(getMaterialName());
		mat->getTechnique(0)->getUserObjectBindings().setUserAny(Ogre::Any(material));
	}
}