#pragma once
#include <Optix/optixu/optixpp_namespace.h>
#include <glm/glm.hpp>
#include <GL/GL.h>
#include <Ogre.h>

namespace Hybrid {
	enum PRIM_TYPE {
		PT_SPHERE,
		PT_PLANE
	};

	class Model {
		public:
			Model(optix::Context context, PRIM_TYPE type);
			Model(optix::Context context, PRIM_TYPE type, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 emissive, float shininess, float reflexive, float opacity, float refractiveIndex);
			Model(optix::Context context, std::string filename);
			Model(optix::Context context, Ogre::Entity* entity);
			~Model();

			optix::Geometry getModel();
			std::vector<optix::Material> getMaterials();
			void setMaterial(Ogre::MaterialPtr material);
			bool isLarge();
		private:
			struct Polygon {
				glm::ivec3 indices;
				glm::ivec3 normals;
				glm::ivec3 texcoords;
				unsigned int material;
			};
		
			optix::Context m_context;
			optix::Geometry m_geometry;
			std::vector<optix::Material> materials;
			bool large;

			void loadPrimitive(PRIM_TYPE type);
			void loadObj(std::string filename);
			void parseMesh(Ogre::Entity* entity);
			void loadTriangleMesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> texcoords, std::vector<Polygon> polygons);
			int loadBasicMaterial();
	};
}