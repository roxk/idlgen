#pragma once

#include <string>

namespace idlgen::WinRT
{
	enum class MemberKind
	{
		Method,
		ClassMethod,
		Property
	};

	class Member
	{
      private:
        std::string _identifier;
      public:
        virtual MemberKind MemberKind() const = 0;
		void SetIdentifier(std::string identifier)
		{
            _identifier = std::move(identifier);
		}
		const std::string& Identifier()
		{
            return _identifier;
		}
	};
}