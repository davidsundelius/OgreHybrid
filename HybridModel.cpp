#include <RenderSystems/GL/OgreGLTexture.h>
#include <glm/glm.hpp>
#include <fstream>
#include <vector>
#include <Optix/optixu/optixu.h>
#ifndef NOMINMAX
	#define NOMINMAX 0
#endif
#include <Optix\optix_math.h>
#include <glm/gtc/type_ptr.hpp>
#include "HybridMaterial.h"
#include "HybridModel.h"

namespace Hybrid {
	using namespace optix;

	//Public
	Model::Model(Context context, PRIM_TYPE type) : m_context(context) {
		loadBasicMaterial();
		loadPrimitive(type);
	}

	Model::Model(Context context, PRIM_TYPE type, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, glm::vec3 emissive, float shininess, float reflexive, float opacity, float refractiveIndex) : m_context(context) {
		loadPrimitive(type);
		Material* mat = new Material(m_context, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.3f, 0.8f, 0.9f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), 10.0f, 0.2f, 1.0f, 1.33f, 0);
		materials.push_back(mat->getTargetMaterial());
	}

	Model::Model(Context context, std::string filename) : m_context(context) {
		loadBasicMaterial();
		loadObj(filename);
	}

	Model::Model(Context context, Ogre::Entity* entity) : m_context(context) {
		parseMesh(entity);
	}

	Model::~Model(){
	}

	Geometry Model::getModel() {
		return m_geometry;
	}

	std::vector<optix::Material> Model::getMaterials() {
		return materials;
	}

	void Model::setMaterial(Ogre::MaterialPtr material) {
		materials.clear();
		Material* mat = new Material(m_context, material);
		materials.push_back(mat->getTargetMaterial());
	}

	bool Model::isLarge() {
		return large;
	}

	//Private
	void Model::loadPrimitive(PRIM_TYPE type) {
		switch(type) {
			case PT_SPHERE: {
				float spherepos[] =  {0, 0, 0, 1};
				Program sphereIntersectionProgram;
				Program sphereBoundingProgram;
				m_geometry = m_context->createGeometry();
				m_geometry->setPrimitiveCount(1u);
				sphereBoundingProgram = m_context->createProgramFromPTXFile("Kernel/sphere.cu.ptx", "sphereBounds");
				m_geometry->setBoundingBoxProgram(sphereBoundingProgram);
				sphereIntersectionProgram = m_context->createProgramFromPTXFile("Kernel/sphere.cu.ptx", "sphereIntersect");
				m_geometry->setIntersectionProgram(sphereIntersectionProgram);
				m_geometry["sphere"]->set4fv(spherepos);
				break;
			}
			case PT_PLANE: {
				float normal[] =  {0, 1, 0};
				float planepos[] =  {0, 0, 0};
				Program planeIntersectionProgram;
				Program planeBoundingProgram;
				m_geometry = m_context->createGeometry();
				m_geometry->setPrimitiveCount(1u);
				planeBoundingProgram = m_context->createProgramFromPTXFile("Kernel/plane.cu.ptx", "planeBounds");
				m_geometry->setBoundingBoxProgram(planeBoundingProgram);
				planeIntersectionProgram = m_context->createProgramFromPTXFile("Kernel/plane.cu.ptx", "planeIntersect");
				m_geometry->setIntersectionProgram(planeIntersectionProgram);
				m_geometry["n"]->set3fv(normal);
				m_geometry["pos"]->set3fv(planepos);
				break;
			}
		}
		large=false;
	}

	void Model::loadObj(std::string filename) {
		std::vector<std::string*> lines;
		std::vector<glm::vec3> vertices;
		std::vector<Polygon> polygons;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;

		//Load data from obj-file
		std::ifstream ins(filename.c_str());
		if(!ins.is_open()) {
			printf("File is open");
			return;
		}
		char buf[256];
		while(!ins.eof())
		{
			ins.getline(buf,256);
			lines.push_back(new std::string(buf));
		}
		for(int i=0;i<lines.size();i++) {
			if(lines[i]->c_str()[0]=='#')  {
				continue;
			} else if(lines[i]->c_str()[0]=='v' && lines[i]->c_str()[1]==' ') {
				float x, y, z;
				sscanf_s(lines[i]->c_str(),"v %f %f %f", &x, &y, &z);
				vertices.push_back(glm::vec3(x, y, z));
			} else if(lines[i]->c_str()[0]=='v' && lines[i]->c_str()[1]=='n') {
				float x, y, z;
				sscanf_s(lines[i]->c_str(),"vn %f %f %f", &x, &y, &z);
				normals.push_back(glm::vec3(x, y, z));   
			} else if(lines[i]->c_str()[0]=='f') {
				Polygon p;
				int v[3], n[4];// t[4];
				//if(count(lines[i]->begin(),lines[i]->end(),' ')==3) {
				sscanf_s(lines[i]->c_str(),"f %d//%d %d//%d %d//%d", &v[0], &n[0], &v[1], &n[1], &v[2], &n[2]);
				p.indices = glm::ivec3(v[0]-1,v[1]-1,v[2]-1);
				//glm::vec3 v1 = vertices[v[0]] - vertices[v[1]];
				//glm::vec3 v2 = vertices[v[0]] - vertices[v[2]];
				//printf("%d, %d, %d \n",p.indices.x,p.indices.y,p.indices.z);
				p.normals = glm::ivec3(n[0]-1,n[1]-1,n[2]-1);
				p.material = 0;
				polygons.push_back(p);
				//}
			}
		}

		//Upload data to GPU
		loadTriangleMesh(vertices,normals,texcoords,polygons);
	}

	void Model::parseMesh(Ogre::Entity* entity) {
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec3> normals;
		std::vector<Polygon> polygons;
		std::vector<glm::vec2> texcoords;

		bool addedShared = false;
		bool textured = false;
	
		int sharedOffset = 0;
		int offset = 0;
		Ogre::MeshPtr mesh = entity->getMesh();
		for(int i=0; i<entity->getNumSubEntities(); ++i) {
			Ogre::SubEntity* se = entity->getSubEntity(i);
			Ogre::SubMesh* sm = se->getSubMesh();
			offset = vertices.size();
			Ogre::VertexData* vd = addedShared ? mesh->sharedVertexData : sm->vertexData;
			if(vd==0) {
				vd = mesh->sharedVertexData;
			}
			if(!sm->useSharedVertices || (sm->useSharedVertices && !addedShared)) {
				if(sm->useSharedVertices) {
					addedShared=true;
					sharedOffset = offset;
				}
				//Vertices
				const Ogre::VertexElement* vertexPos = vd->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
				Ogre::HardwareVertexBufferSharedPtr vb = vd->vertexBufferBinding->getBuffer(vertexPos->getSource());
				unsigned char* vertex = static_cast<unsigned char*>(vb->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
				Ogre::Real* vptr; 
				for(int j=0; j<vd->vertexCount; ++j, vertex+=vb->getVertexSize()) {
					vertexPos->baseVertexPointerToElement(vertex, &vptr);
					glm::vec3 v;
					v.x=(*vptr++);
					v.y=(*vptr++);
					v.z=(*vptr++);
					vertices.push_back(v);
				}
				vb->unlock();

				//Normals
				const Ogre::VertexElement* vertexNormal = vd->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);
				Ogre::HardwareVertexBufferSharedPtr nb = vd->vertexBufferBinding->getBuffer(vertexNormal->getSource());
				unsigned char* normal = static_cast<unsigned char*>(nb->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
				Ogre::Real* nptr; 
				for(int j=0; j<vd->vertexCount; ++j, normal+=nb->getVertexSize()) {
					vertexNormal->baseVertexPointerToElement(normal, &nptr);
					glm::vec3 n;
					n.x=(*nptr++);
					n.y=(*nptr++);
					n.z=(*nptr++);
					normals.push_back(n);
				}
				nb->unlock();

				//Texcoords
				const Ogre::VertexElement* vertexTexcoord = vd->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);
				if(vertexTexcoord!=NULL) {
					Ogre::HardwareVertexBufferSharedPtr tb = vd->vertexBufferBinding->getBuffer(vertexTexcoord->getSource());
					unsigned char* texcoord = static_cast<unsigned char*>(tb->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
					Ogre::Real* tptr; 
					for(int j=0; j<vd->vertexCount; ++j, texcoord+=tb->getVertexSize()) {
						vertexTexcoord->baseVertexPointerToElement(texcoord, &tptr);
						glm::vec2 t;
						t.x=(*tptr++);
						t.y=(*tptr++);
						/*char message[320];
						sprintf(message, "\nTexcoords for %s:\nt.x:%f, t.y:%f\n",entity->getName().c_str(),t.x,t.y);
						Ogre::LogManager::getSingleton().logMessage(message);*/
						//MessageBox(0, ("Laddar texcoords: " + sm->getMaterialName() + "\nFick nummer:" + Ogre::StringConverter::toString(mId)).c_str(), "Raytracer error", MB_OK | MB_ICONEXCLAMATION); 
						texcoords.push_back(t);
					}
					tb->unlock();
					textured=true;
				} else {
					glm::vec2 t;
					t.x=0.0f;
					t.y=0.0f;
					texcoords.push_back(t);
				}
			}

			//Materials
			int mId = 0;
			if(Ogre::MaterialManager::getSingleton().resourceExists(sm->getMaterialName())) {
				Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(sm->getMaterialName());
				Material* hybridMaterial = new Material(m_context, material);
				materials.push_back(hybridMaterial->getTargetMaterial());
				mId = materials.size()-1;
				//MessageBox(0, ("Laddar material: " + sm->getMaterialName() + "\nFick nummer:" + Ogre::StringConverter::toString(mId)).c_str(), "Raytracer error", MB_OK | MB_ICONEXCLAMATION); 
			} else {
				Ogre::String ms = se->getMaterialName();
				if(Ogre::MaterialManager::getSingleton().resourceExists(ms)) {
					Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(ms);
					Material* hybridMaterial = new Material(m_context, material);
					materials.push_back(hybridMaterial->getTargetMaterial());
					mId = materials.size()-1;
				}
			}

			//Polygons
			Ogre::IndexData* id = sm->indexData;
			int numPolygons = id->indexCount/3;
			polygons.reserve(numPolygons);

			unsigned short* ps;
			unsigned int* pi;
			Ogre::HardwareIndexBufferSharedPtr ib = id->indexBuffer;
			bool use32 = (ib->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
			if(use32) pi = static_cast<unsigned int*>(ib->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			else ps = static_cast<unsigned short*>(ib->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			for(int j=0; j<numPolygons; ++j) {
				Polygon p;
				unsigned int index = use32? *pi++ : *ps++;
				p.indices[0] = index+offset;
				p.normals[0] = index+offset;
				if(textured) p.texcoords[0] = index+offset;
				index = use32? *pi++ : *ps++;
				p.indices[1] = index+offset;
				p.normals[1] = index+offset;
				if(textured) p.texcoords[1] = index+offset;
				index = use32? *pi++ : *ps++;
				p.indices[2] = index+offset;
				p.normals[2] = index+offset;
				if(textured) p.texcoords[2] = index+offset;
				p.material = mId;
				polygons.push_back(p);
			}
			ib->unlock();
		}
		/*char message[320];
		sprintf(message, "\n\n=====================HÄÄÄÄR===============p[0].indices: %d %d %d \n p[0].normals: %d %d %d \n vertices[0]: %f %f %f \n normals[0]: %f %f %f \n, offset: %d\n sharedOffset: %d",
			polygons[70].indices[0], polygons[70].indices[1], polygons[70].indices[2],
			polygons[70].normals[0], polygons[70].normals[1], polygons[70].normals[2],
			vertices[1800].x, vertices[1800].y, vertices[1800].z,
			normals[1800].x, normals[1800].y, normals[1800].z,
			offset, sharedOffset);
		Ogre::LogManager::getSingleton().logMessage(message);*/

		//MessageBox(0, ("Entity "+entity->getName()+" översatt med " + Ogre::StringConverter::toString(materials.size()) + " material.").c_str(), "Raytracer error", MB_OK | MB_ICONEXCLAMATION);
		if(materials.size()==0) {
			loadBasicMaterial();
		}
		loadTriangleMesh(vertices,normals,texcoords,polygons);
	}

	void Model::loadTriangleMesh(std::vector<glm::vec3> vertices, std::vector<glm::vec3> normals, std::vector<glm::vec2> texcoords, std::vector<Polygon> polygons) {
		Buffer vertexBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, vertices.size());
		float3* vertexData = static_cast<float3*>(vertexBuffer->map());
		for(int i=0; i<vertices.size(); ++i) {
			vertexData[i] = make_float3(vertices[i].x,vertices[i].y,vertices[i].z);
		}
		vertexBuffer->unmap();

		Buffer normalBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, normals.size());
		float3* normalData = static_cast<float3*>(normalBuffer->map());
		for(int i=0; i<normals.size(); ++i) {
			normalData[i] = make_float3(normals[i].x,normals[i].y,normals[i].z);
		}
		normalBuffer->unmap();
	
		Buffer texcoordBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, texcoords.size());
		float2* texcoordData = static_cast<float2*>(texcoordBuffer->map());
		for(int i=0; i<texcoords.size(); ++i) {
			texcoordData[i] = make_float2(texcoords[i].x,texcoords[i].y);
		}
		//memcpy(static_cast<void*>(texcoordData), static_cast<void*>(&(texcoords[0])), sizeof(float)*texcoords.size()*2);   
		texcoordBuffer->unmap();
	
		Buffer vIndexBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, polygons.size());
		int3* vIndexData = static_cast<int3*>(vIndexBuffer->map());
		Buffer nIndexBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, polygons.size());
		int3* nIndexData = static_cast<int3*>(nIndexBuffer->map());
		Buffer tIndexBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_INT3, polygons.size());
		int3* tIndexData = static_cast<int3*>(tIndexBuffer->map());
		Buffer mIndexBuffer = m_context->createBuffer(RT_BUFFER_INPUT, RT_FORMAT_UNSIGNED_INT, polygons.size());
		unsigned int* mIndexData = static_cast<unsigned int*>(mIndexBuffer->map());

		int3 indices;
		for(int i=0; i<polygons.size(); ++i) {
			indices.x = polygons[i].indices.x;
			indices.y = polygons[i].indices.y;
			indices.z = polygons[i].indices.z;
			vIndexData[i] = indices;

			indices.x = polygons[i].normals.x;
			indices.y = polygons[i].normals.y;
			indices.z = polygons[i].normals.z;
			nIndexData[i] = indices;

			indices.x = polygons[i].texcoords.x;
			indices.y = polygons[i].texcoords.y;
			indices.z = polygons[i].texcoords.z;
			tIndexData[i] = indices;

			mIndexData[i]=polygons[i].material;
			//Ogre::LogManager::getSingleton().logMessage("This is material mIndexData[" + Ogre::StringConverter::toString(i) + "]: " + Ogre::StringConverter::toString(mIndexData[i]) + "\n");
		}
		vIndexBuffer->unmap();
		nIndexBuffer->unmap();
		tIndexBuffer->unmap();
		mIndexBuffer->unmap();
	
		float* vbd = static_cast<float*>(vertexBuffer->map() );
		RTsize numVertices;
		RTgeometry geo;
		vertexBuffer->getSize(numVertices);

		//rtuCreateClusteredMesh(m_context->get(), 0, &geo, (unsigned int)vertices.size(), vbd, polygons.size(), (const unsigned int*)vIndexData, (const unsigned int*)mIndexData);
		rtuCreateClusteredMeshExt(m_context->get(), 0, &geo, 
									 (unsigned int)vertices.size(), vbd, 
									 polygons.size(), (const unsigned int*)vIndexData,
									 (const unsigned int*)mIndexData,
									 normalBuffer->get(),
									 (const unsigned int*)nIndexData,
									 texcoordBuffer->get(),
									 (const unsigned int*)tIndexData);

		vertexBuffer->unmap();
		m_geometry = Geometry::take(geo);
		large=true;
	}

	int Model::loadBasicMaterial() {
		Material* mat = new Material(m_context, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.3f, 0.8f, 0.9f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), 10.0f, 0.2f, 1.0f, 1.33f, 0);
		materials.push_back(mat->getTargetMaterial());
		return materials.size();
	}
}