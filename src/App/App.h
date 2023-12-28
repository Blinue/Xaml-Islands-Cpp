#pragma once
#include "App.g.h"
#include <winrt/Windows.UI.Xaml.Hosting.h>

namespace winrt::XamlIslandsCpp::App::implementation {

class App : public App_base<App, Markup::IXamlMetadataProvider> {
public:
	App();
	~App();

	void Close();

private:
	Hosting::WindowsXamlManager _windowsXamlManager{ nullptr };
	bool _isClosed = false;

	////////////////////////////////////////////////////
	// 
	// IXamlMetadataProvider 相关
	// 
	/////////////////////////////////////////////////////
public:
	Markup::IXamlType GetXamlType(Interop::TypeName const& type) {
		return AppProvider()->GetXamlType(type);
	}

	Markup::IXamlType GetXamlType(hstring const& fullName) {
		return AppProvider()->GetXamlType(fullName);
	}

	com_array<Markup::XmlnsDefinition> GetXmlnsDefinitions() {
		return AppProvider()->GetXmlnsDefinitions();
	}

private:
	com_ptr<XamlMetaDataProvider> _appProvider;
	com_ptr<XamlMetaDataProvider> AppProvider() {
		if (!_appProvider) {
			_appProvider = make_self<XamlMetaDataProvider>();
		}
		return _appProvider;
	}
};

}

namespace winrt::XamlIslandsCpp::App::factory_implementation {

class App : public AppT<App, implementation::App> {
};

}
