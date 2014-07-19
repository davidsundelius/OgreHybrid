#include <GL\glew.h>
#include "HybridRenderQueueListener.h"

namespace Hybrid {
	//Public
	HybridRenderQueueListener::HybridRenderQueueListener(int vbo, int texture) : mVbo(vbo), mTexture(texture) {
	}

	HybridRenderQueueListener::~HybridRenderQueueListener() {
	}
	
	void HybridRenderQueueListener::renderQueueStarted(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& skipThisInvocation) {
	}
	
	void HybridRenderQueueListener::renderQueueEnded(Ogre::uint8 queueGroupId, const Ogre::String& invocation, bool& repeatThisInvocation) {
		if (queueGroupId != Ogre::RENDER_QUEUE_MAIN) 
			return;

		/*glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glPopMatrix();*/
		//GLuint test;
		//glGenBuffers(1, &test);

		//MessageBox(0, ("Binder textur nummer: " + Ogre::StringConverter::toString(test)).c_str(), "Raytracer runtime error", MB_OK | MB_ICONEXCLAMATION);
		//glBindTexture(GL_TEXTURE_2D, mTexture);
		//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mVbo);
		//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		
		//glBindBuffer(GL_PIXEL_PACK_BUFFER, mVbo);
		/*glBindTexture(GL_TEXTURE_RECTANGLE_ARB, mTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 768, 0, GL_RGBA, GL_FLOAT, 0);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);*/
		//glGetTexImage(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, GL_FLOAT, 0);
		//glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
}