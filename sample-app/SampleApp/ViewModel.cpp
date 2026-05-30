#include "pch.h"
#include "ViewModel.author.h"
#include "winrt/SampleApp.h"
#include "Generated Files/idlgen.impl.h"

void winrt::SampleApp::author::ViewModel::StaticMethod1()
{
	SampleApp::ViewModel::StaticMethod1();
	SampleApp::ViewModel::StaticMethod2();
}

void winrt::SampleApp::author::ViewModel::StaticMethod2()
{}

void winrt::SampleApp::author::ViewModel::PrivateStaticMethod()
{}

int winrt::SampleApp::author::ViewModel::PrivateInstanceMethod()
{
	return 0;
}
void winrt::SampleApp::author::ViewModelWithInternalInterface::SomeMethod()
{
    self(this)->overridable().MethodWithValueType(SampleApp::Permission::Camera);
	self(this)->overridable().OverridableMethod();
}

void winrt::SampleApp::author::ViewModelWithInternalInterface::OverridableMethod()
{
	int i = 0;
}

void winrt::SampleApp::author::ViewModelWithInternalInterface::MethodWithValueType(Permission permission)
{
	int i = 0;
}

winrt::hstring winrt::SampleApp::author::ViewModelWithInternalInterface::ToString(winrt::author::override)
{
	return winrt::hstring();
}

void winrt::SampleApp::author::DerivedViewModel::OverridableMethod(winrt::author::override)
{
	int i = 0;
}

void winrt::SampleApp::author::DerivedViewModel::MethodWithValueType(Permission permission, winrt::author::override)
{
	int i = 0;
}

void winrt::SampleApp::author::DerivedViewModel::Method(winrt::author::override)
{
}

void winrt::SampleApp::author::ImplementingInternalAuthoredInterface::Method(winrt::author::override)
{
	int i = 0;
}

void winrt::SampleApp::author::DebugableViewModel::DebugPrint()
{}

winrt::hstring winrt::SampleApp::author::DebugableViewModel::ToString()
{
	return winrt::hstring();
}
