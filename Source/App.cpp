#include <array>

//#include <windows.h>
#include <winrt/Windows.ApplicationModel.Core.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Composition.h>
#include <winrt/Windows.UI.Input.h>

#include "Renderer/D3D12/Renderer.hpp"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

struct App : implements<App, IFrameworkViewSource, IFrameworkView>
{
	std::unique_ptr<Mythology::D3D12::Renderer> m_renderer{};
	winrt::agile_ref<CoreWindow> m_window{};

    IFrameworkView CreateView()
    {
        return *this;
    }

    void Initialize(CoreApplicationView const &)
    {
    }

    void Load(hstring const&)
    {
		IUnknown& window = *static_cast<::IUnknown*>(winrt::get_abi(m_window.get()));
		winrt::Windows::Foundation::Rect const bounds = m_window.get().Bounds();
		m_renderer = std::make_unique<Mythology::D3D12::Renderer>(window, Eigen::Vector2f{ bounds.Width, bounds.Height });
    }

    void Uninitialize()
    {
    }

    void Run()
    {
        CoreWindow window = CoreWindow::GetForCurrentThread();
        window.Activate();

		while (true)
		{
			CoreDispatcher dispatcher = window.Dispatcher();
			dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			m_renderer->render();
			m_renderer->present();
		}
    }

    void SetWindow(CoreWindow window)
    {
        window.PointerPressed({ this, &App::OnPointerPressed });
        window.PointerMoved({ this, &App::OnPointerMoved });

        window.PointerReleased([&](auto && ...)
        {
        });

		m_window = window;
    }

    void OnPointerPressed(IInspectable const &, PointerEventArgs const & args)
    {
    }

    void OnPointerMoved(IInspectable const &, PointerEventArgs const & args)
    {
    }
};

int main()
{
	CoreApplication::Run(App{});

	return 0;
}
