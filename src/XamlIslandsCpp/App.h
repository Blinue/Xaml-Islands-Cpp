#pragma once
#include "App.g.h"
#include <winrt/Windows.UI.Xaml.Hosting.h>

namespace winrt::XamlIslandsCpp::implementation {

class App : public App_base<App, Markup::IXamlMetadataProvider> {
public:
	App();

private:
	Hosting::WindowsXamlManager _windowsXamlManager{ nullptr };

	////////////////////////////////////////////////////
	// 
	// IXamlMetadataProvider 相关
	// 
	/////////////////////////////////////////////////////
public:
	Markup::IXamlType GetXamlType(Interop::TypeName const& type) {
		return _AppProvider()->GetXamlType(type);
	}

	Markup::IXamlType GetXamlType(hstring const& fullName) {
		return _AppProvider()->GetXamlType(fullName);
	}

	com_array<Markup::XmlnsDefinition> GetXmlnsDefinitions() {
		return _AppProvider()->GetXmlnsDefinitions();
	}

private:
	const com_ptr<implementation::XamlMetaDataProvider>& _AppProvider() {
		if (!_appProvider) {
			_appProvider = make_self<implementation::XamlMetaDataProvider>();
		}
		return _appProvider;
	}

	com_ptr<implementation::XamlMetaDataProvider> _appProvider;
};

}

namespace winrt::XamlIslandsCpp::factory_implementation {

class App : public AppT<App, implementation::App> {
};

}
