#include "pch.h"
#include "App.h"
#if __has_include("App.g.cpp")
#include "App.g.cpp"
#endif


namespace winrt::XamlIslandsCpp::App::implementation {

App::App() {
	__super::Initialize();
	
	AddRef();
	m_inner.as<::IUnknown>()->Release();
}

App::~App() {
	Close();
}

}
