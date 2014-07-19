#include "HybridPlugin.h"
#include "HybridManager.h"
#include "HybridEntityFactory.h"

#ifdef _DEBUG
	const Ogre::String pluginName = "Plugin_Hybrid_d";
#else
	const Ogre::String pluginName = "Plugin_Hybrid";
#endif

HybridPlugin::HybridPlugin() : coreScene(0), coreWindow(0) {
}

const Ogre::String& HybridPlugin::getName() const {
  return pluginName;
}

void HybridPlugin::install() {
	Ogre::Root::getSingleton().addMovableObjectFactory(new Hybrid::EntityFactory(),true);
	Ogre::LogManager::getSingleton().logMessage("HybridPlugin: Installed");
}

void HybridPlugin::initialise() {
	Ogre::Root::getSingleton().addFrameListener(this);
	Ogre::LogManager::getSingleton().logMessage("HybridPlugin: Initialized");
}

void HybridPlugin::shutdown() {
	Ogre::LogManager::getSingleton().logMessage("HybridPlugin: Has been shutdown");
}

void HybridPlugin::uninstall() {
	Ogre::LogManager::getSingleton().logMessage("HybridPlugin: Uninstalled");
}

bool HybridPlugin::frameStarted(const Ogre::FrameEvent& evt) {
	if(coreWindow==0) { //Initialize
		coreWindow = Ogre::Root::getSingleton().getAutoCreatedWindow();
		Ogre::SceneManagerEnumerator::SceneManagerIterator smit = Ogre::Root::getSingleton().getSceneManagerIterator();
		coreScene = smit.getNext();
		Ogre::SceneManager::CameraIterator cit = coreScene->getCameraIterator();
		coreCamera = cit.getNext();
		Hybrid::HybridManager::getSingleton()->initialize(coreWindow, coreScene, coreCamera);
		Ogre::Root::getSingleton().removeFrameListener(this);
	}
	return true;
}