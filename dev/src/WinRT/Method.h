#pragma once

#include "Member.h"
#include "Type.h"
#include <optional>
#include <vector>

namespace idlgen::WinRT
{
	class Method : public Member
	{
      public:
		  enum class ParameterKind
		  {
			  None,
			  Ref,
			  Out
		  };
		  struct Parameter
		  {
              std::string name;
              Type& type;
              ParameterKind kind;
		  };
      private:
        std::optional<std::reference_wrapper<Type>> _returnType;
        std::vector<Parameter> _parameters;
      public:
		  virtual idlgen::WinRT::MemberKind MemberKind() const override
		  {
              return MemberKind::Method;
		}
		  void SetReturnType(Type& type)
		  {
              _returnType = type;
		  }
          void AddParameter(std::string name, Type& type, ParameterKind kind = ParameterKind::None);
		  const std::vector<Parameter>& Parameters() const
		  {
              return _parameters;
		  }
	};
}