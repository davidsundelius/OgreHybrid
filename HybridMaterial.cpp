#include <RenderSystems/GL/OgreGLTexture.h>
#include "HybridManager.h"
#include "HybridMaterial.h"
#include <glm/gtc/type_ptr.hpp>


namespace Hybrid {
	Material::Material(optix::Context context, Ogre::MaterialPtr baseMaterial) {
		setBaseMaterial(baseMaterial);
		this->context = context;
		createMaterial();
		this->reflexive=0.0f;
		this->opacity=1.0f;
		this->refractiveIndex=1.0f;
		update();
	}

	Material::Material(optix::Context context, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 emissive, float shininess, float reflexive, float opacity, float refractiveIndex,  GLuint tex) {
		this->context = context;
		createMaterial();
		this->reflexive=reflexive;
		this->opacity=opacity;
		this->refractiveIndex=refractiveIndex;
		prepareMaterial(ambient, diffuse, specular, emissive, shininess, tex);
	}

	Material::~Material() {
	}

	void Material::setReflexive(float reflexive) {
		this->reflexive=reflexive;
		targetMaterial["reflexive"]->setFloat(reflexive);
	}

	void Material::setOpacity(float opacity) {
		this->opacity=opacity;
		targetMaterial["opacity"]->setFloat(opacity);
	}

	void Material::setRefractiveIndex(float refractiveIndex) {
		this->refractiveIndex=refractiveIndex;
		targetMaterial["refractiveIndex"]->setFloat(refractiveIndex);
	}

	float Material::getReflexive() {
		return reflexive;
	}

	float Material::getOpacity() {
		return opacity;
	}

	float Material::getRefractiveIndex() {
		return refractiveIndex;
	}

	void Material::update() {
		if(!baseMaterial.isNull()) {
			Ogre::ColourValue ambient = baseMaterial->getTechnique(0)->getPass(0)->getAmbient();
			Ogre::ColourValue diffuse = baseMaterial->getTechnique(0)->getPass(0)->getDiffuse();
			Ogre::ColourValue specular = baseMaterial->getTechnique(0)->getPass(0)->getSpecular();
			Ogre::ColourValue emissive = baseMaterial->getTechnique(0)->getPass(0)->getEmissive();
			float shininess = baseMaterial->getTechnique(0)->getPass(0)->getShininess();
			GLuint tex = 0;
			if(baseMaterial->getNumTechniques()!=0 && baseMaterial->getTechnique(0)->getNumPasses()!=0 && baseMaterial->getTechnique(0)->getPass(0)->getNumTextureUnitStates()!=0) {
				Ogre::String texName = baseMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName();
				if(Ogre::TextureManager::getSingleton().resourceExists(texName)) {
					Ogre::TexturePtr texPtr = Ogre::TextureManager::getSingleton().getByName(texName);
					Ogre::Image img;
					texPtr->convertToImage(img);
					Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(texPtr->getName()+"_optixformat", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, texPtr->getWidth(), texPtr->getHeight(), 0, Ogre::PF_FLOAT32_RGBA, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
					texture->loadImage(img);
					tex = ((Ogre::GLTexturePtr)texture)->getGLID();
				}
			}
			prepareMaterial(glm::vec3(ambient.r,ambient.g,ambient.b),
							glm::vec3(diffuse.r,diffuse.g,diffuse.b),
							glm::vec3(specular.r,specular.g,specular.b),
							glm::vec3(emissive.r,emissive.g,emissive.b),
							shininess,tex);
		}
	}

	void Material::setBaseMaterial(Ogre::MaterialPtr material) {
		baseMaterial=material;
		material->getTechnique(0)->getUserObjectBindings().setUserAny("HybridMaterial", Ogre::Any(this));
	}

	optix::Material Material::getTargetMaterial() {
		return targetMaterial;
	}

	//Private
	optix::Program Material::closestHitProgram = 0;
	optix::Program Material::anyHitProgram = 0;

	void Material::createMaterial() {
		if(closestHitProgram==0) closestHitProgram = context->createProgramFromPTXFile("Kernel/shader.cu.ptx", "closestHit");
		if(anyHitProgram==0) anyHitProgram = context->createProgramFromPTXFile("Kernel/shader.cu.ptx", "anyHitShadow");
		targetMaterial = context->createMaterial();
		targetMaterial->setClosestHitProgram(0, closestHitProgram);
		targetMaterial->setAnyHitProgram(1,anyHitProgram);
		HybridManager::getSingleton()->addMaterial(this);
	}

	void Material::prepareMaterial(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 emissive, float shininess, GLuint tex) {
		targetMaterial["ambient"]->set3fv(glm::value_ptr(ambient));
		targetMaterial["diffuse"]->set3fv(glm::value_ptr(diffuse));
		targetMaterial["specular"]->set3fv(glm::value_ptr(specular));
		targetMaterial["emissive"]->set3fv(glm::value_ptr(emissive));
		targetMaterial["shininess"]->setFloat(shininess);
		targetMaterial["reflexive"]->setFloat(reflexive);
		targetMaterial["opacity"]->setFloat(opacity);
		targetMaterial["refractiveIndex"]->setFloat(refractiveIndex);
		targetMaterial["isTextured"]->setInt(0);

		if(tex==0) {
			optix::Buffer buffer = context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT, 1u, 1u);
			float* buffer_data = static_cast<float*>(buffer->map());
			buffer_data[0] = 0.0f;
			buffer_data[1] = 0.0f;
			buffer->unmap();
			optix::TextureSampler sampler = context->createTextureSampler();
			sampler->setArraySize(1u);
			sampler->setMipLevelCount(1u);
			sampler->setBuffer(0u, 0u, buffer);
			sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
			targetMaterial["sampler"]->setTextureSampler(sampler);
		} else {
			loadTexture(tex);
		}
	}

	void Material::loadTexture(GLuint tex) {
		optix::TextureSampler sampler;
		sampler = context->createTextureSamplerFromGLImage(tex, RT_TARGET_GL_TEXTURE_2D);
		sampler->setWrapMode(0, RT_WRAP_REPEAT);
		sampler->setWrapMode(1, RT_WRAP_REPEAT);
		sampler->setIndexingMode(RT_TEXTURE_INDEX_NORMALIZED_COORDINATES);
		sampler->setReadMode(RT_TEXTURE_READ_ELEMENT_TYPE);
		sampler->setMaxAnisotropy(1.0f);
		sampler->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
		targetMaterial["sampler"]->setTextureSampler(sampler);
		targetMaterial["isTextured"]->setInt(1);
	}
}