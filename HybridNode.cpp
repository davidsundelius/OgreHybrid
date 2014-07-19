#include "HybridNode.h"
#include <OGRE/OgreVector3.h>
#include <glm/gtc/type_ptr.hpp>
#define PI 3.14159215

namespace Hybrid {
	using namespace optix;

	//Public
	Node::Node(Context context, Model* model) : m_context(context) {
		isRaytraced=false;
		transform = m_context->createTransform();
		std::vector<Material> materials = model->getMaterials();
		instance = m_context->createGeometryInstance(model->getModel(), materials.begin(), materials.end());
		if(model->isLarge()) {
			acc = m_context->createAcceleration("MedianBvh", "Bvh");
		} else {
			acc = m_context->createAcceleration("NoAccel", "NoAccel");
		}
		group = m_context->createGeometryGroup(&instance, &instance+1);
		group->setAcceleration(acc);
		acc->markDirty();
		transform->setChild(group);
		pos=glm::vec3(0.0f,0.0f,0.0f);
		sc=glm::vec3(1.0f,1.0f,1.0f);
		rot=glm::mat4(1.0f,0.0f,0.0f,0.0f,
					  0.0f,1.0f,0.0f,0.0f,
					  0.0f,0.0f,1.0f,0.0f,
					  0.0f,0.0f,0.0f,0.0f);
		doTransform();
	}

	Node::~Node() {
	}

	Transform Node::getTransformation() {
		return transform;
	}

	void Node::move(glm::vec3 pos) {
		this->pos = pos;
		doTransform();
	}

	void Node::scale(glm::vec3 sc) {
		this->sc = sc;
		doTransform();
	}

	void Node::rotate(Ogre::Matrix3 rot) {
		this->rot = glm::mat4(rot.GetColumn(0).x, rot.GetColumn(1).x, rot.GetColumn(2).x, 0.0f,
							  rot.GetColumn(0).y, rot.GetColumn(1).y, rot.GetColumn(2).y, 0.0f,
							  rot.GetColumn(0).z, rot.GetColumn(1).z, rot.GetColumn(2).z, 0.0f,
							  0.0f				, 0.0f				, 0.0f				, 1.0f);
		doTransform();
	}

	//Private
	void Node::doTransform() {
		glm::mat4 tm = rot*glm::mat4( sc.x, 0.0f, 0.0f, pos.x,
								  0.0f, sc.y, 0.0f, pos.y,
								  0.0f, 0.0f, sc.z, pos.z,
								  0.0f, 0.0f, 0.0f, 1.0f);
		transform->setMatrix(0,glm::value_ptr(tm),0);
	}

	void Node::nodeUpdated(const Ogre::Node* node) {
		Ogre::Vector3 v = node->getPosition();
		move(glm::vec3(v.x, v.y, v.z));
		v = node->getScale();
		scale(glm::vec3(v.x, v.y, v.z));
		Ogre::Matrix3 r;
		node->getOrientation().ToRotationMatrix(r);
		rotate(r);
	}
}