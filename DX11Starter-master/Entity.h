#pragma once

#include <memory>
#include "Transform.h"
#include "Mesh.h"

class Entity
{
public:
	Entity(std::shared_ptr<Mesh> mesh);
	~Entity();

	//Getters
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
};

