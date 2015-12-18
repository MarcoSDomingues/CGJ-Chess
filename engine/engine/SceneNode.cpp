#pragma once
#include "SceneNode.h"
#include "VSShaderLib.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "SceneGraph.h"
#include "ManagerLight.h"

SceneNode::SceneNode() {
	modelMatrix = Matrix4().identity();
	transform = Transform();
	boundingBox = new BoundingBox();
}

SceneNode::~SceneNode() {}

void SceneNode::addSceneNode(char* name, SceneNode* scene_node) {
	sceneNodes.insert(std::pair<char*, SceneNode*>(name, scene_node));
	scene_node->name = name;
	scene_node->parentNode = this;
	scene_node->sceneGraph = sceneGraph;
}

SceneNode* SceneNode::getSceneNode(char* name) {
	return sceneNodes[name];
}

void SceneNode::loadMaterialUniforms() {
	if (shaderProgram) {
		if (shaderProgram->needBlend) material->setAlpha(0.5);
	}
	shaderProgram->setUniform("mat.ambient", material->getAmbient());
	shaderProgram->setUniform("mat.diffuse", material->getDiffuse());
	shaderProgram->setUniform("mat.specular", material->getSpecular());
	shaderProgram->setUniform("mat.shininess", material->getShininess());

	Matrix4* nm = new Matrix4(modelMatrix);
	shaderProgram->setUniform("NormalMatrix", nm->invert().transpose().data());

	shaderProgram->setUniform("mlwNumPointLights", ManagerLight::instance()->getLightsCount());

}

void SceneNode::loadTextureUniforms() {
	texture->draw();
	shaderProgram->setUniform("tex_map", 0);
}

void SceneNode::updateModelMatrix() {
	modelMatrix = Matrix4().translate(transform.position);
	//TODO
	//.rotate(transform.rotation.x, Vector3(1, 0, 0))
	//.rotate(transform.rotation.y, Vector3(0, 1, 0))
	//.rotate(transform.rotation.z, Vector3(0, 0, 1))
	//.scale(transform.scale.x, transform.scale.y, transform.scale.z);
}

void SceneNode::setLightUniforms() {
	PointLight* pointlight = ManagerLight::instance()->getPointLight("main");


	//shaderProgram->setUniform("pointLights", );
	//shaderProgram->setUniform("mat.ambient", material->getAmbient());
	//shaderProgram->setUniform("mat.ambient", material->getAmbient());
	//shaderProgram->setUniform("mat.ambient", material->getAmbient());
	//shaderProgram->setUniform("mat.ambient", material->getAmbient());

}

Matrix4* SceneNode::calculateGraphModelMatrix() {
	return new Matrix4();
}

void SceneNode::setUniforms() {
	//calculate model matrix
	updateModelMatrix();

	shaderProgram->setUniform("ModelMatrix", modelMatrix.data());

	if (material) loadMaterialUniforms();
	if (texture) loadTextureUniforms();
}

void SceneNode::update() {
	if (isDebug) std::cout << "\nUpdating SceneNode::name::" << name;
	//std::cout << transform.position.x << "," << transform.position.y << ";" << transform.position.z << "\n";
	if (sceneGraph->checkIntersection) {
		if (boundingBox->checkRayIntersection(sceneGraph->rayOrigin, sceneGraph->rayDirection)) {
			std::cout << "colision\n";
			objectPicked = true;
		}
	} else objectPicked = false;

	if (objectPicked) {
		float camDist = sceneGraph->camera->Distance;
		transform.setPosition(sceneGraph->rayPoint.x * camDist, sceneGraph->rayPoint.y * camDist, sceneGraph->rayPoint.z * camDist);
		modelMatrix = Matrix4().translate(transform.position.x, transform.position.y, transform.position.z);
	}

	boundingBox->setPosition(transform.position.x, transform.position.y, transform.position.z);
}

void SceneNode::draw() {
	if (isDebug) std::cout << "\nDrawaing SceneNode::name::" << name;

	if (shaderProgram) {
		if (shaderProgram->needBlend) {
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);
			glEnable(GL_CULL_FACE);
			glDepthMask(GL_FALSE);
		}
		setUniforms();
		if (shaderProgram->affectedByLights) setLightUniforms();
		if (mesh) mesh->draw();

		if (shaderProgram->needBlend) {
			glDisable(GL_BLEND);
			glDisable(GL_CULL_FACE);
			glDepthMask(GL_TRUE);
		}
	}

	for (_sceneNodesIterator = sceneNodes.begin(); _sceneNodesIterator != sceneNodes.end(); ++_sceneNodesIterator) {
		_sceneNodesIterator->second->draw();
	}
}

