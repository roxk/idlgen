#pragma once

#include "Entity.h"

namespace idlgen::WinRT
{
	class Delegate : public Entity
	{
      public:
		  EntityKind Kind() const override
		  {
              return EntityKind::Delegate;
		}
	};
}