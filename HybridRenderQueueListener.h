#pragma once
#include <Ogre.h>

namespace Hybrid {
	class HybridRenderQueueListener : public Ogre::RenderQueueListener {
		public:
			HybridRenderQueueListener(int vbo, int texture);
			~HybridRenderQueueListener();
			void renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation);
			void renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation);
		private:
			int mVbo;
			int mTexture;
	};
}