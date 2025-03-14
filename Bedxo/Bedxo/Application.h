#pragma once
#define IMGUI_ENABLE_FREETYPE
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <string>
#include <vector>
#include <memory>
#include "Layer.h"
#include "Image.h"
#include <functional>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
namespace Bedxo
{

	struct AppConfig
	{
		std::string Title = "Bedxo Application";
		/// <summary>
		/// This one may be set on the start of any layer <para/>
		/// The icon height will be clamped to 30px, the width will not be changed
		/// </summary>
		std::shared_ptr<Image> TitleBarIconData = nullptr;
		//custom menu callback to draw whatever you want in the 40px title bar after the logo
		std::function<void(Application*)> MenuBarCallback = nullptr;
		// Should be in IM_COL32 format
		ImU32 TitleBarBgColor = IM_COL32(10, 10, 10, 255);
		// If this enabled the app will not load the default font
		bool UseCustomFont = false;
		// Window size
		ImVec2 WindowSize = ImVec2(1280, 720);
		/// <summary>
		/// If this is set to false the docking will be managed by the user. <para/>
		/// Otherwise every layer will be added in order creating a tabbed window
		///	<summary>
		bool UseAutoDocking = true;
	};

	class Application
	{
	public:
		Application(AppConfig);
		void Start();
		bool CreateDeviceD3D(HWND hWnd);
		void CreateRenderTarget();
		void CleanupDeviceD3D();
		void CleanupRenderTarget();
		void ResizeSwapChain();
		void Frame(HWND hwnd);

		void AddLayer(std::shared_ptr<Layer> layer);

		std::shared_ptr<Image> LoadImageFromMemory(const void* data, size_t size);
		std::shared_ptr<Image> LoadImageFromFile(const std::string& path);

		AppConfig m_Config;

		static UINT g_ResizeWidth;
		static UINT g_ResizeHeight;
	private:

		static LRESULT CALLBACK WndProcStub(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
			// Get `this` pointer from window data
			Application* pThis = reinterpret_cast<Application*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
			if (pThis) {
				return pThis->WndProc(hwnd, msg, wParam, lParam);  // Call member function
			}
			return DefWindowProc(hwnd, msg, wParam, lParam);
		}

		LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		ID3D11Device* m_pd3dDevice = nullptr;
		ID3D11DeviceContext* m_pd3dDeviceContext = nullptr;
		IDXGISwapChain* m_pSwapChain = nullptr;
		bool m_SwapChainOccluded = false;
		ID3D11RenderTargetView* m_mainRenderTargetView = nullptr;
		std::vector<std::shared_ptr<Layer>> m_Layers;

		std::shared_ptr<Image> m_MinimizeIcon;
		std::shared_ptr<Image> m_MaximizeIcon;
		std::shared_ptr<Image> m_RestoreIcon;
		std::shared_ptr<Image> m_CloseIcon;

		bool m_Maximized = false;
		bool m_Inited = false;
		bool m_Rendering = false;
		ImVec2 lastSize;
		ImVec2 lastPos;
	};
}