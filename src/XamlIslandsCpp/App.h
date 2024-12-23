#pragma once
#include "App.g.h"
#include <winrt/Windows.UI.Xaml.Hosting.h>
#include "Event.h"
#include "AppSettings.h"

namespace XamlIslandsCpp {
class MainWindow;
}

namespace winrt::XamlIslandsCpp::implementation {

struct RootPage;

class App : public App_base<App, Markup::IXamlMetadataProvider> {
public:
	static App& Get();

	App();
	App(const App&) = delete;
	App(App&&) = delete;

	void Initialize();

	int Run();

	void Quit();

	const CoreDispatcher& Dispatcher() const noexcept {
		return _dispatcher;
	}

	const com_ptr<RootPage>& RootPage() const noexcept;

	const ::XamlIslandsCpp::MainWindow& MainWindow() const noexcept {
		return *_mainWindow;
	}

	::XamlIslandsCpp::MainWindow& MainWindow() noexcept {
		return *_mainWindow;
	}

	bool IsLightTheme() const noexcept {
		return _isLightTheme;
	}

	::XamlIslandsCpp::Event<bool> ThemeChanged;

private:
	void _AppSettings_ThemeChanged(::XamlIslandsCpp::AppTheme theme);

	void _UpdateColorValuesChangedRevoker();

	void _UpdateTheme();

	Hosting::WindowsXamlManager _windowsXamlManager{ nullptr };

	std::unique_ptr<::XamlIslandsCpp::MainWindow> _mainWindow;
	CoreDispatcher _dispatcher{ nullptr };

	::XamlIslandsCpp::Event<::XamlIslandsCpp::AppTheme>::EventRevoker _themeChangedRevoker;
	Windows::UI::ViewManagement::UISettings _uiSettings;
	Windows::UI::ViewManagement::UISettings::ColorValuesChanged_revoker _colorValuesChangedRevoker;
	bool _isLightTheme = true;

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
