#pragma once
#include <OgrePrerequisites.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#       define _HybridExport __declspec(dllexport) 
#else 
#   define _HybridExport
#endif