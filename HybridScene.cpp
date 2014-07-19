#include "HybridScene.h"

namespace Hybrid {
	using namespace optix;

	Scene::Scene(Context context) {
		this->context = context;
		isInit=false;
	}

	Scene::~Scene() {
	}

	void Scene::initialize() {
		isInit=false;
		//Geometry
		Transform* objects = new Transform[nodes.size()];
		for(int i=0;i<nodes.size();++i) {
			objects[i] = nodes[i]->getTransformation();
		}
		top = context->createGroup(&objects[0],&objects[nodes.size()]);
		acc = context->createAcceleration("NoAccel", "NoAccel");
		top->setAcceleration(acc);
		acc->markDirty();
		isInit=true;
	}

	void Scene::parseScene(Ogre::SceneManager* sceneMgr) {
		this->sceneMgr = sceneMgr;
		for(Ogre::Node::ChildNodeIterator it = sceneMgr->getRootSceneNode()->getChildIterator(); it.hasMoreElements(); ) {
			Ogre::SceneNode* n = static_cast<Ogre::SceneNode*>(it.getNext());
			parseNode(n);
		}
		float ambientColor[] = {sceneMgr->getAmbientLight().r,sceneMgr->getAmbientLight().g, sceneMgr->getAmbientLight().b};
		context["ambientColor"]->set3fv(ambientColor);
		light_buffer = context->createBuffer(RT_BUFFER_INPUT);
		light_buffer->setFormat(RT_FORMAT_USER);
		light_buffer->setElementSize(sizeof(Light));
		reparseLights();
	}

	void Scene::reparseLights() {
		if(sceneMgr!=NULL) {
			lights.clear();
			for(Ogre::SceneManager::MovableObjectIterator it = sceneMgr->getMovableObjectIterator("Light"); it.hasMoreElements(); ) {
				Ogre::Light* l = static_cast<Ogre::Light*>(it.getNext());
				parseLight(l);
			}
			light_buffer->setSize(lights.size());
			memcpy(light_buffer->map(), lights.data(), lights.size()*sizeof(Light));
			light_buffer->unmap();
			context["lights"]->set(light_buffer);
		}
	}

	int Scene::addNode(Node* n) {
		nodes.push_back(n);
		int i = nodes.size()-1;
		if(isInit) {
			unsigned int nr = top->getChildCount();
			top->setChildCount(nr+1);
			top->setChild(nr,n->getTransformation());
			acc->markDirty();
		}
		return i;
	}

	Node* Scene::getNode(int i) {
		return nodes[i];
	}

	Group Scene::getScene() {
		return top;
	}

	//Private
	void Scene::parseNode(Ogre::SceneNode* node) {
		for(Ogre::SceneNode::ObjectIterator it = node->getAttachedObjectIterator(); it.hasMoreElements(); ) {
			Ogre::MovableObject* m = static_cast<Ogre::MovableObject*>(it.getNext());
			if(m->getMovableType()=="Entity") {
				Ogre::Entity* e = static_cast<Ogre::Entity*>(m);
				//Parse model
				Model* m = new Model(context, e);
				Node* n = new Node(context, m);
				addNode(n);
				node->setListener(n);
			} else {
				MessageBox(0, ("Cannot parse node of type " + m->getMovableType()).c_str(), "Raytracer error", MB_OK | MB_ICONEXCLAMATION);
			}
		}
	}

	void Scene::parseLight(Ogre::Light* light) {
		Light l;
		l.pos=make_float3(light->getPosition().x,light->getPosition().y,light->getPosition().z);
		l.color=make_float3(light->getDiffuseColour().r,light->getDiffuseColour().g,light->getDiffuseColour().b);
		l.castShadows=0;
		l.padding=0;
		lights.push_back(l);
	}
}