#include "HybridManager.h"

namespace Hybrid {
	//Public
	HybridManager* HybridManager::getSingleton() {
		return instance;
	}

	void HybridManager::initialize(Ogre::RenderWindow* window, Ogre::SceneManager* scene, Ogre::Camera* camera) {
		coreWindow = window;
		coreScene = scene;
		coreCamera = camera;

		coreWindow->getViewport(0)->setVisibilityMask(0xFFFFF0F);

		//Initialize Render-to-texture mask
		Ogre::TexturePtr rtt_texture = Ogre::TextureManager::getSingleton().createManual("RttTex", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, coreWindow->getWidth(), coreWindow->getHeight(), 0, Ogre::PF_R8, Ogre::TU_RENDERTARGET, NULL, false, 0, "");
		renderTexture = rtt_texture->getBuffer()->getRenderTarget();
		renderTexture->addViewport(coreCamera);
		renderTexture->getViewport(0)->setClearEveryFrame(true);
		renderTexture->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Black);
		renderTexture->getViewport(0)->setOverlaysEnabled(false);
		renderTexture->getViewport(0)->setVisibilityMask(0xFFFFFF0);
		renderTexture->getViewport(0)->setMaterialScheme("Mask");
		renderTexture->setAutoUpdated(true);
		
		//Create materials
		Ogre::ResourceGroupManager::getSingleton().createResourceGroup("Hybrid");
		Ogre::MaterialPtr rttMaterial = Ogre::MaterialManager::getSingleton().create("Hybrid/Mask", "Hybrid");
		Ogre::Pass* p = rttMaterial->getTechnique(0)->getPass(0);
		p->setLightingEnabled(false);
		Ogre::TextureUnitState* tex = p->createTextureUnitState();
		tex->setTextureName("RttTex");
		
		Ogre::MaterialPtr hybridMaterial = Ogre::MaterialManager::getSingleton().create("Hybrid/Raytracer", "Hybrid");
		p =  hybridMaterial->getTechnique(0)->getPass(0);
		p->setSceneBlending(Ogre::SceneBlendType::SBT_TRANSPARENT_ALPHA);
		p->setDepthWriteEnabled(false);
		p->setLightingEnabled(false);
		tex = p->createTextureUnitState();
		tex->setTextureName("RTtarget");

		//Initialize raytracer
		float fovy = coreCamera->getFOVy().valueRadians()*0.5f;
		float fovx = atanf(coreCamera->getAspectRatio() * tanf(fovy));
		float focal = coreCamera->getFocalLength();
		rt = new Raytracer(coreWindow->getWidth(), coreWindow->getHeight(), fovx, fovy, focal, coreScene);

		//Create the hybrid renderer
		hybridScreen = new Ogre::Rectangle2D(true);
		hybridScreen->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);
		hybridScreen->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
		hybridScreen->setMaterial("Hybrid/Raytracer");
		hybridScreen->setVisibilityFlags(0x0F);
		hybridScreenNode = coreScene->getRootSceneNode()->createChildSceneNode("hybridScreenNode");
		hybridScreenNode->attachObject(hybridScreen);

		//Disable skybox in raytracing mode
		if(coreScene->isSkyBoxEnabled()) {
			Ogre::SceneNode::ObjectIterator it = coreScene->getSkyBoxNode()->getAttachedObjectIterator();
			while (it.hasMoreElements()) {
			   it.getNext()->setVisibilityFlags(0x0F);
			}
		}

		//Debugging
		//MaskScreen
		maskScreen = new Ogre::Rectangle2D(true);
		maskScreen->setCorners(0.5f, -0.5f, 1.0f, -1.0f);
		maskScreen->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
		maskScreen->setVisibilityFlags(0x0F);
		Ogre::SceneNode* maskScreenNode = coreScene->getRootSceneNode()->createChildSceneNode("MaskScreenNode");
		maskScreenNode->attachObject(maskScreen);
		maskScreen->setMaterial("Hybrid/Mask");
		maskScreen->setVisible(false);
		//RTScreen
		rtScreen = new Ogre::Rectangle2D(true);
		rtScreen->setCorners(0.0f, -0.5f, 0.5f, -1.0f);
		rtScreen->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
		rtScreen->setMaterial("Hybrid/Raytracer");
		rtScreen->setVisibilityFlags(0x0F);
		Ogre::SceneNode* rtScreenNode = coreScene->getRootSceneNode()->createChildSceneNode("RTScreenNode");
		rtScreenNode->attachObject(rtScreen);
		rtScreen->setVisible(false);

		renderTexture->addListener(this);

		//Ogre::CompositorManager::getSingleton().addCompositor(coreWindow->getViewport(0), "Hybrid");
		//Ogre::CompositorManager::getSingleton().setCompositorEnabled(coreWindow->getViewport(0), "Hybrid", true);

		pluginEnabled = true;
	}

	void HybridManager::setRenderMode(RenderMode renderMode) {
		this->renderMode=renderMode;
		switch(renderMode) {
			case NORMAL:
				rt->setHybridMode(false);
				hybridScreen->setVisible(false);
				coreWindow->getViewport(0)->setVisibilityMask(0xFFFFFFF);
				break;
			case HYBRID:
				rt->setHybridMode(true);
				hybridScreen->setVisible(true);
				coreWindow->getViewport(0)->setVisibilityMask(0xFFFFF0F);
				renderTexture->getViewport(0)->setVisibilityMask(0xFFFFFF0);
				break;
			case RAYTRACE:
				rt->setHybridMode(false);
				hybridScreen->setVisible(true);
				renderTexture->getViewport(0)->setVisibilityMask(0xFFFFFFF);
				break;
		}
	}

	HybridManager::RenderMode HybridManager::getRenderMode() {
		return renderMode;
	}

	void HybridManager::setSuperSampling(SuperSampling ss) {
		if(ss==SS_4X) {
			rt->setSuperSampling(4);
		} else {
			rt->setSuperSampling(1);
		}
	}

	void HybridManager::setTransferMode(TransferMode tm) {
		rt->setTransferMode(tm == TM_PBO);
	}

	void HybridManager::setDebug(bool debugMode) {
		this->debugMode=debugMode;
		maskScreen->setVisible(debugMode);
		rtScreen->setVisible(debugMode);
	}

	bool HybridManager::getDebug() {
		return debugMode;
	}

	bool HybridManager::isPluginEnabled() {
		return pluginEnabled;
	}

	void HybridManager::updateLights() {
		rt->updateLights();
	}

	void HybridManager::updateMaterials() {
		for(int i=0;i<materials.size();++i) {
			materials[i]->update();
		}
		materials[3]->setReflexive(0.0f);
	}
	
	void HybridManager::addMaterial(Material* material) {
		materials.push_back(material);
	}

	optix::Context HybridManager::getOptixContext() {
		return rt->getOptixContext();
	}

	//Protected
	void HybridManager::preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt) {
	}

	void HybridManager::postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt) {
		if(renderMode!=NORMAL) {
			rt->render((float*)coreCamera->getDerivedPosition().ptr(), coreCamera->getDerivedRight().ptr(), coreCamera->getDerivedUp().ptr(), (coreCamera->getDerivedDirection()).ptr());
		}
	}

	//Private
	HybridManager* HybridManager::instance = new HybridManager();

	HybridManager::HybridManager() {
		pluginEnabled=false;
	}

	HybridManager::~HybridManager() {
	}
}