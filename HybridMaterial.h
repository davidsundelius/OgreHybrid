#pragma once
#include "HybridPluginPrerequisites.h"
#include <Optix/optixu/optixpp_namespace.h>
#pragma once
#include <Ogre.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

namespace Hybrid {
	class _HybridExport Material {
		public:
			Material(optix::Context context, Ogre::MaterialPtr baseMaterial);
			Material(optix::Context context, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 emissive, float shininess, float reflexive, float opacity, float refractiveIndex,  GLuint tex);
			~Material();
			void setReflexive(float reflexive);
			void setOpacity(float opacity);
			void setRefractiveIndex(float refractiveIndex);
			float getReflexive();
			float getOpacity();
			float getRefractiveIndex();
			void update();

			void setBaseMaterial(Ogre::MaterialPtr material);
			optix::Material getTargetMaterial();
		private:
			Ogre::MaterialPtr baseMaterial;
			optix::Material targetMaterial;
			optix::Context context;

			static optix::Program closestHitProgram;
			static optix::Program anyHitProgram;

			float reflexive;
			float opacity;
			float refractiveIndex;

			void createMaterial();
			void prepareMaterial(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 emissive, float shininess, GLuint tex);
			void loadTexture(GLuint tex);
	};
}