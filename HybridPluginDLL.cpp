#include "HybridPlugin.h"
#include <OgreRoot.h>

HybridPlugin* plugin;

extern "C" void _HybridExport dllStartPlugin(void)
{
  plugin = new HybridPlugin();
  Ogre::Root::getSingleton().installPlugin(plugin);
}
extern "C" void _HybridExport dllStopPlugin(void)
{
  Ogre::Root::getSingleton().uninstallPlugin(plugin);
  delete plugin;
}