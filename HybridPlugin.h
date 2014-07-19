#pragma once
#include "HybridPluginPrerequisites.h"
#include <Ogre.h>
#include <OgrePlugin.h>

class HybridPlugin : public Ogre::Plugin, public Ogre::FrameListener{
  public:
    HybridPlugin();
    const Ogre::String& getName() const;
    void install();
    void initialise();
    void shutdown();
    void uninstall();
	bool frameStarted(const Ogre::FrameEvent& evt);
  private:
	Ogre::SceneManager* coreScene;
	Ogre::RenderWindow* coreWindow;
	Ogre::Camera* coreCamera;
};