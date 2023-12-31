#include "Entity.h"

Entity::Entity(std::shared_ptr<Mesh> mesh , std::shared_ptr<Material> material)
{
    this->mesh = mesh;
    this->material = material;
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

std::shared_ptr<Material> Entity::GetMaterial()
{
    return material;
}

void Entity::SetMaterial(std::shared_ptr<Material> material)
{
    this->material = material;
}
