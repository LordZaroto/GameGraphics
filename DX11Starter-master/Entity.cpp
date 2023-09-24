#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> mesh)
{
    this->mesh = mesh;
    transform = std::make_shared<Transform>();
}

Entity::~Entity()
{
}

std::shared_ptr<Transform> Entity::GetTransform()
{
    return transform;
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
    return mesh;
}
