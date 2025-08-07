#pragma once

#include "Type.h"
#include <memory>
#include <optional>

namespace idlgen::WinRT
{
class Nullable : public Type
	{
      private:
        std::optional<std::reference_wrapper<Entity>> _entity;
      public:
		  EntityKind Kind() const override
		  {
              return EntityKind::Nullable;
		}
          void SetValue(Entity& entity)
          {
              _entity = entity;
          }
          bool HasValue() const
          {
              return _entity.has_value();
          }
          Entity& Value()
          {
              return _entity == nullptr ? std::nullopt : _entity->get();
        }
	};
}