#pragma once

#include "Method.h"

namespace idlgen::WinRT
{
	class ClassMethod : public Method
	{
      private:
        bool _isStatic;
      public:
        idlgen::WinRT::MemberKind MemberKind() const override
        {
            return MemberKind::ClassMethod;
        }
        void SetIsStatic(bool isStatic)
        {
            _isStatic = isStatic;
        }
        bool IsStatic() const
        {
            return _isStatic;
        }
	};
}