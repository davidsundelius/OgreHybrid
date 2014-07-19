#include <RenderSystems/GL/OgreGLTexture.h>
#include <RenderSystems/GL/OgreGLHardwarePixelBuffer.h>
#include <RenderSystems/GL/OgreGLHardwareVertexBuffer.h>
#include <Windows.h>
#include <cstdlib>
#include <stdio.h>
#include <time.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Optix/optix_math.h>
#include <OGRE/Ogre.h>

#include "HybridRaytracer.h"
#include "HybridRenderQueueListener.h"

namespace Hybrid {
	using namespace optix;

	//Public
	Raytracer::Raytracer(int width, int height, float fovx, float fovy, float focal, Ogre::SceneManager* sceneMgr) {
		try {
			this->width=width;
			this->height=height;
			this->fovx=fovx;
			this->fovy=fovy;
			this->focal=focal;
			this->sceneMgr=sceneMgr;
			hybridMode=true;
			transferMode=true;
			initialize();
			buildScene(sceneMgr);
			context->validate();
			context->compile();
			context->launch(0, width, height);
			sceneMgr->addRenderQueueListener(new HybridRenderQueueListener(vboId,target));
		} catch(optix::Exception& e) {
			MessageBox(0, e.getErrorString().c_str(), "Raytracer initialization error", MB_OK | MB_ICONEXCLAMATION);
		}
	}

	Raytracer::~Raytracer() {
		context->destroy();
	}

	void Raytracer::render(float* camPos, float* u, float* v, float* w) {
		try {
			context["camPos"]->set3fv(camPos);
			glm::vec3 gu = glm::vec3(u[0],u[1],u[2]);
			glm::vec3 gv = glm::vec3(v[0],v[1],v[2]);
			glm::vec3 gw = glm::vec3(w[0],w[1],w[2]);
			float ulen = focal * tanf(fovx); 
			float vlen = focal * tanf(fovy);
			gu=ulen*gu;
			gv=vlen*gv;
			gw=focal*gw;
			context["u"]->set3fv(glm::value_ptr(gu));
			context["v"]->set3fv(glm::value_ptr(gv));
			context["w"]->set3fv(glm::value_ptr(gw));

			context->launch(0, width, height);

			if(transferMode) {
				Ogre::HardwarePixelBufferSharedPtr pbo = (Ogre::HardwarePixelBufferSharedPtr)targetPtr->getBuffer();
				pbo->lock(Ogre::HardwareBuffer::HBL_DISCARD);
				float* pixeldata = static_cast<float*>(vbo->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
				memcpy(pbo->getCurrentLock().data, pixeldata, targetPtr->getWidth()*targetPtr->getHeight()*4*sizeof(float));
				vbo->unlock();
				pbo->unlock();
			}

			/*Ogre::HardwarePixelBufferSharedPtr pb = (Ogre::HardwarePixelBufferSharedPtr)targetPtr->getBuffer(); 
			pb->lock(Ogre::HardwarePixelBuffer::HBL_DISCARD);
			memcpy(pb->getCurrentLock().data, resultBuffer->map(), targetPtr->getWidth()*targetPtr->getHeight()*4*sizeof(float));
			pb->unlock();
			resultBuffer->unmap();*/
		} catch(optix::Exception& e) {
			MessageBox(0, e.getErrorString().c_str(), "Raytracer runtime error", MB_OK | MB_ICONEXCLAMATION);
			printf("Raytracer error: %s", e.getErrorString().c_str());
			exit(1);
		}
	}

	void Raytracer::resize(int width, int height) {
		resultBuffer->destroy();
		resultBuffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width, height);
		context["outBuffer"]->setBuffer(resultBuffer);
		this->width=width;
		this->height=height;
	}

	void Raytracer::setHybridMode(bool hybridMode) {
		this->hybridMode=hybridMode;
		if(hybridMode) {
			context["skipMask"]->setInt(false);
		} else {
			context["skipMask"]->setInt(true);
		}
	}

	void Raytracer::setSuperSampling(unsigned int numSamples) {
		this->numSamples=numSamples;
		context["numSamples"]->setUint(numSamples);
	}

	void Raytracer::setTransferMode(bool tm) {
		this->transferMode=tm;
	}

	void Raytracer::updateLights() {
		scene->reparseLights();
	}

	optix::Context Raytracer::getOptixContext() {
		return context;
	}

	//Private
	void Raytracer::initialize() {
		//Initialize target texture
		targetPtr = Ogre::TextureManager::getSingleton().createManual("RTtarget", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_FLOAT32_RGBA, Ogre::TU_RENDERTARGET);
		target = ((Ogre::GLTexturePtr)targetPtr)->getGLID();

		//Initialize OptiX context
		context = Context::create();
		context->setRayTypeCount(2);
		context->setEntryPointCount(1);
		context->setStackSize(2048);
		context["rayTypeRadience"]->setUint(0u);
		context["rayTypeShadow"]->setUint(1u);
		
		//Ogre::HardwarePixelBuffer* pbo = (Ogre::HardwarePixelBuffer*)targetPtr->getBuffer().get(); 
		Ogre::HardwareVertexBufferSharedPtr vertexBuffer = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(1, 21, Ogre::HardwareBuffer::HBU_STATIC);
		vbo = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(float)*4, width*height, Ogre::HardwareBuffer::HBU_DYNAMIC);
		vboId = static_cast<Ogre::GLHardwareVertexBuffer*>(vbo.get())->getGLBufferId();
		float* test = static_cast<float*>(vbo->lock(Ogre::HardwareBuffer::HBL_DISCARD));
		vbo->unlock();

		resultBuffer = context->createBufferFromGLBO(RT_BUFFER_INPUT_OUTPUT, vboId);
		resultBuffer->setFormat(RT_FORMAT_FLOAT4);
		resultBuffer->setSize(width,height);
		
		//optix::Buffer resultBuffer2;
		//resultBuffer2 = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT4, width, height);
		context["outBuffer"]->setBuffer(resultBuffer);
		context["numSamples"]->setUint(1u);
	
		//Initialize masktexture
		Ogre::TexturePtr ptr = Ogre::TextureManager::getSingleton().getByName("RttTex");
		GLuint maskid = ((Ogre::GLTexturePtr)ptr)->getGLID();
		mask = context->createTextureSamplerFromGLImage(maskid, RT_TARGET_GL_TEXTURE_2D);
		mask->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
		mask->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);
		mask->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
		context["mask"]->setTextureSampler(mask);
		context["skipMask"]->setInt(false);
		mask->unregisterGLTexture();

		//Ray generation program
		float fov[] = {tanf(fovx), tanf(fovy)};
		generatorProgram = context->createProgramFromPTXFile("Kernel/generator.cu.ptx","generate");
		context->setRayGenerationProgram(0, generatorProgram);
		context["fov"]->set2fv(fov);
		context["focal"]->setFloat(focal);

		//Exception program
	#ifdef _DEBUG
		exceptionProgram = context->createProgramFromPTXFile("Kernel/generator.cu.ptx", "exception");
		context->setExceptionProgram(0, exceptionProgram);
		context->setExceptionEnabled(RT_EXCEPTION_ALL, true);
		context->setPrintEnabled(true);
		context->setPrintBufferSize(4096);
	#else
		context->setExceptionEnabled(RT_EXCEPTION_ALL, false);
		context->setPrintEnabled(false);
		context->setPrintBufferSize(0);
	#endif
		//Miss program
		float up[] = {0.0f, 1.0f, 0.0f};
		Ogre::TexturePtr texPtr = Ogre::TextureManager::getSingleton().load("Env.jpg", "General");
		if(texPtr.isNull()) exit(1);

		Ogre::Image img;
		texPtr->convertToImage(img);
		Ogre::TexturePtr tex = Ogre::TextureManager::getSingleton().createManual("spheremap", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, texPtr->getWidth(), texPtr->getHeight(), 0, Ogre::PF_FLOAT32_RGBA, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
		tex->loadImage(img);
		GLuint texture = ((Ogre::GLTexturePtr)tex)->getGLID();

		TextureSampler sampler;
		sampler = context->createTextureSamplerFromGLImage(texture, RT_TARGET_GL_TEXTURE_2D);
		sampler->setWrapMode(0, RT_WRAP_REPEAT);
		sampler->setWrapMode(1, RT_WRAP_REPEAT);
		sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
		sampler->setReadMode(RT_TEXTURE_READ_NORMALIZED_FLOAT);
		sampler->setMaxAnisotropy(1.0f);
		sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
		context["tex"]->setTextureSampler(sampler);

		missProgram = context->createProgramFromPTXFile("Kernel/miss.cu.ptx", "miss");
		context->setMissProgram(0, missProgram);
		context["up"]->set3fv(up);
	}

	void Raytracer::buildModels() {
		//sphere1 = new Model(context, PT_SPHERE, glm::vec3(0.7f, 0.7f, 0.7f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),10.0f,0.0f,1.0f,0.0f);
		//plane = new Model(context, PT_PLANE, glm::vec3(0.0f, 0.0f, 0.2f), glm::vec3(0.2f, 0.2f, 1.0f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.0f),10.0f,0.6f,1.0f,0.0f);
	}

	void Raytracer::buildScene(Ogre::SceneManager* sceneMgr) {
		scene = new Scene(context);
		scene->parseScene(sceneMgr);
		scene->initialize();
		context["scene"]->set(scene->getScene());
	}
}