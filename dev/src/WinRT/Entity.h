#pragma once

#include <string>

namespace idlgen::WinRT
{
	enum class EntityKind
	{
		Namespace,
		Primitive,
		Enum,
		EnumFlags,
		Struct,
		Nullable,
        RuntimeClass,
		Interface,
		Delegate,
		Array
	};

	class Entity
	{
      private:
        std::string _identifier;
      public:
		~Entity()
		{
		}
        virtual EntityKind Kind() const = 0;
		void SetIdentifier(std::string identifier)
		{
            _identifier = std::move(identifier);
		}
		const std::string& Identifier() const
		{
            return _identifier;
		}
	};
}
