#include "Namespace.h"

void idlgen::WinRT::Namespace::AddEntity(std::unique_ptr<Entity> entity)
{
    _entities.emplace_back(std::move(entity));
}
