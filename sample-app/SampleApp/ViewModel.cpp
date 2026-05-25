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
{}

void winrt::SampleApp::author::ViewModelWithInternalInterface::OveriddableMethod()
{}

winrt::hstring winrt::SampleApp::author::ViewModelWithInternalInterface::ToString(winrt::author::ignore)
{
	return winrt::hstring();
}

void winrt::SampleApp::author::DerivedViewModel::OverridableMethod(winrt::author::override)
{}

void winrt::SampleApp::author::DerivedViewModel::Method()
{
}
