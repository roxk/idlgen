#pragma once

#include "Entity.h"
#include <vector>
#include <memory>

namespace idlgen::WinRT
{
class Namespace : public Entity
{
  private:
    std::vector<std::unique_ptr<Entity>> _entities;
  public:
      EntityKind Kind() const override
      {
          return EntityKind::Namespace;
    }
      void AddEntity(std::unique_ptr<Entity> entity);
};
}
