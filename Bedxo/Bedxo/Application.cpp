#include "Application.h"
#include <iostream>
#include <windowsx.h>
#include <D3DX11tex.h>
#include "icons.h"
namespace Bedxo
{
	UINT Application::g_ResizeWidth = 0;
	UINT Application::g_ResizeHeight = 0;


	Application::Application(AppConfig config) : m_Config(config)
	{
	}

	void Application::Start()
	{
		ImGui_ImplWin32_EnableDpiAwareness();
		WNDCLASSEXA wc = { sizeof(wc), CS_CLASSDC, WndProcStub, 0L, 0L, GetModuleHandle(nullptr),
			nullptr, nullptr, nullptr, nullptr, m_Config.Title.c_str(), nullptr };
		::RegisterClassEx(&wc);

		HWND hwnd = ::CreateWindowEx(WS_EX_APPWINDOW, // Behave as normal window idk why the normal windows get locked on alt+tab :P
			wc.lpszClassName, m_Config.Title.c_str(),
			WS_POPUP,                        // Borderless window
			100, 100, 1280, 800,
			nullptr, nullptr, wc.hInstance, nullptr);

		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this)); // we store our class pointer to access later in the WndProc function

		// Initialize Direct3D
		if (!CreateDeviceD3D(hwnd))
		{
			CleanupDeviceD3D();
			::UnregisterClass(wc.lpszClassName, wc.hInstance);
			return;
		}

		// Show the window
		::ShowWindow(hwnd, SW_SHOWDEFAULT);
		::UpdateWindow(hwnd);

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		// disabling the imgui.ini file
		io.IniFilename = nullptr;

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// Setup Platform/Renderer backends
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(m_pd3dDevice, m_pd3dDeviceContext);

		// Load Fonts
		// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
		// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
		// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
		// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
		// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
		// - Read 'docs/FONTS.md' for more instructions and details.
		// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
		//io.Fonts->AddFontDefault();
		if (!m_Config.UseCustomFont)
			io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\tahoma.ttf", 18.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
		//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
		//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
		//IM_ASSERT(font != nullptr);

		// Our state
		for (auto& layer : m_Layers)
		{
			layer->OnStart(this);
		}

		m_MinimizeIcon = LoadImageFromMemory((void*)::minimize_icon, sizeof(::minimize_icon));
		m_MaximizeIcon = LoadImageFromMemory((void*)::maximize_icon, sizeof(::maximize_icon));
		m_RestoreIcon = LoadImageFromMemory((void*)::restore_icon, sizeof(::restore_icon));
		m_CloseIcon = LoadImageFromMemory((void*)::close_icon, sizeof(::close_icon));

		// Main loop
		bool done = false;
		m_Inited = true;
		while (!done)
		{
			// Poll and handle messages (inputs, window resize, etc.)
			// See the WndProc() function below for our to dispatch events to the Win32 backend.
			MSG msg;
			while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
				if (msg.message == WM_QUIT)
					done = true;
			}
			if (done)
				break;

			// Handle window being minimized or screen locked
			if (m_SwapChainOccluded && m_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
			{
				::Sleep(10);
				continue;
			}
			m_SwapChainOccluded = false;

			{ // frame render
				Frame(hwnd);
			}
		}

		// Cleanup
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		CleanupDeviceD3D();
		::DestroyWindow(hwnd);
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
	}

	bool Application::CreateDeviceD3D(HWND hWnd)
	{
		// Setup swap chain
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 2;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		UINT createDeviceFlags = 0;
		//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		D3D_FEATURE_LEVEL featureLevel;
		const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
		HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
		if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
			res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pd3dDevice, &featureLevel, &m_pd3dDeviceContext);
		if (res != S_OK)
			return false;

		CreateRenderTarget();
		return true;
	}

	void Application::CreateRenderTarget()
	{
		ID3D11Texture2D* pBackBuffer;
		m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		m_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_mainRenderTargetView);
		pBackBuffer->Release();
	}

	void Application::CleanupDeviceD3D()
	{
		CleanupRenderTarget();
		if (m_pSwapChain) { m_pSwapChain->Release(); m_pSwapChain = nullptr; }
		if (m_pd3dDeviceContext) { m_pd3dDeviceContext->Release(); m_pd3dDeviceContext = nullptr; }
		if (m_pd3dDevice) { m_pd3dDevice->Release(); m_pd3dDevice = nullptr; }
	}

	void Application::CleanupRenderTarget()
	{
		if (m_mainRenderTargetView) { m_mainRenderTargetView->Release(); m_mainRenderTargetView = nullptr; }
	}

	void Application::ResizeSwapChain()
	{
		CleanupRenderTarget();
		m_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
		g_ResizeWidth = g_ResizeHeight = 0;
		CreateRenderTarget();
	}

	void Application::Frame(HWND hwnd)
	{
		if (!m_Inited)
			return;

		if (hwnd == nullptr)
			return;

		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
		auto& io = ImGui::GetIO();
		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		if (m_Rendering)
			return;

		m_Rendering = true; // we need to lock the rendering to prevent calling ImGui::NewFrame multiple times and fail

		ImGui::NewFrame();
		{
			static int titleBarHeight = 40;
			{ // MENU BAR DRAW
				ImGui::SetNextWindowPos(ImVec2(0, 0));
				ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, titleBarHeight));  // Adjust height of title bar
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
				ImGui::PushStyleColor(ImGuiCol_WindowBg, m_Config.TitleBarBgColor); // Dark color

				if (ImGui::Begin((m_Config.Title + "##TitleBar").c_str(), nullptr,
					ImGuiWindowFlags_NoDecoration |
					ImGuiWindowFlags_NoDocking |
					ImGuiWindowFlags_NoMove |
					ImGuiWindowFlags_NoCollapse |
					ImGuiWindowFlags_NoScrollbar)) {

					if (m_Config.TitleBarIconData != nullptr)
					{
						// draw the image in the center of this window
						ImGui::SetCursorPos(ImVec2{ ImGui::GetCursorPosX(), 5 });
						auto size = m_Config.TitleBarIconData->GetSize();
						ImGui::Image(m_Config.TitleBarIconData->GetTexture(), ImVec2{ size.x, 30 });
					}
					ImGui::SameLine(0, 10);

					if (m_Config.MenuBarCallback != nullptr)
					{
						m_Config.MenuBarCallback(this);
					}
					ImGui::SameLine();

					static auto tbButtonSizes = ImVec2{ 32, 32 };
					static int tbButtonSpacing = 5;
					static ImU32 tbButtonBgColor = IM_COL32(0, 0, 0, 0);
					static ImU32 tbButtonHvColor = IM_COL32(255, 255, 255, 100);
					static ImU32 tbButtonHvExitColor = IM_COL32(255, 0, 0, 100);
					auto textSize = ImGui::CalcTextSize(m_Config.Title.c_str());

					ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - textSize.x / 2);
					ImGui::SetCursorPosY(textSize.y / 2);
					ImGui::Text(m_Config.Title.c_str());
					ImGui::SameLine(ImGui::GetWindowWidth() - ((tbButtonSizes.x + 16) * 3));
					ImGui::PushStyleColor(ImGuiCol_Button, tbButtonBgColor);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, tbButtonHvColor);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, tbButtonBgColor);
					auto cursorY = ImGui::GetCursorPosY() - 5;
					ImGui::SetCursorPosY(cursorY);
					ImGui::BeginGroup();
					if (ImGui::ImageButton("minimize", m_MinimizeIcon->GetTexture(), tbButtonSizes)) {
						ShowWindow(hwnd, SW_MINIMIZE);
					}
					ImGui::SameLine(0, tbButtonSpacing);
					ImGui::SetCursorPosY(cursorY);
					if (ImGui::ImageButton("maximize", m_Maximized ? m_RestoreIcon->GetTexture() : m_MaximizeIcon->GetTexture(), tbButtonSizes))
					{

						if (!m_Maximized)
						{
							MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
							RECT wSize;
							GetWindowRect(hwnd, &wSize);
							lastPos.x = wSize.left;
							lastPos.y = wSize.top;
							lastSize.x = wSize.right - wSize.left;
							lastSize.y = wSize.bottom - wSize.top;
							if (GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST), &monitorInfo)) {
								RECT workArea = monitorInfo.rcWork;

								SetWindowPos(hwnd, nullptr,
									workArea.left, workArea.top,
									workArea.right - workArea.left,
									workArea.bottom - workArea.top,
									SWP_NOZORDER | SWP_NOACTIVATE);
								m_Maximized = true;
							}
						}
						else
						{
							SetWindowPos(hwnd, nullptr,
								lastPos.x, lastPos.y,
								lastSize.x,
								lastSize.y,
								SWP_NOZORDER | SWP_NOACTIVATE);
							m_Maximized = false;
						}

					}
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, tbButtonHvExitColor);
					ImGui::SameLine(0, tbButtonSpacing);
					ImGui::SetCursorPosY(cursorY);
					if (ImGui::ImageButton("close", m_CloseIcon->GetTexture(), tbButtonSizes)) {
						PostQuitMessage(0);
					}
					ImGui::EndGroup();
					ImGui::PopStyleColor(4);

					ImGui::End();
				}

				ImGui::PopStyleVar(2);
				ImGui::PopStyleColor(1);
			}

			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

			ImGuiViewport* viewport = ImGui::GetMainViewport();
			auto windowPos = viewport->Pos;
			windowPos.y += titleBarHeight;
			ImGui::SetNextWindowPos(windowPos);
			auto windowSize = viewport->Size;
			windowSize.y -= titleBarHeight;
			ImGui::SetNextWindowSize(windowSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);

			ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f });

			ImGui::Begin("DockSpaceWindow", nullptr, window_flags);
			ImGui::PopStyleVar(3);
			ImGui::PopStyleColor();

			auto dockId = ImGui::GetID("MyDockspace");
			ImGui::DockSpace(dockId, ImVec2{ (float)g_ResizeWidth, (float)g_ResizeHeight }, ImGuiDockNodeFlags_NoUndocking);
			for (auto& layer : m_Layers)
			{
				ImGui::SetNextWindowDockID(dockId, ImGuiCond_Always);
				layer->OnRender(this);
			}

			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		m_pd3dDeviceContext->OMSetRenderTargets(1, &m_mainRenderTargetView, nullptr);
		m_pd3dDeviceContext->ClearRenderTargetView(m_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		// Present
		HRESULT hr = m_pSwapChain->Present(1, 0);   // Present with vsync
		//HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
		m_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
		m_Rendering = false;
	}

	void Application::AddLayer(std::shared_ptr<Layer> layer)
	{
		m_Layers.push_back(layer);
	}

	std::shared_ptr<Image> Application::LoadImageFromMemory(const void* data, size_t size)
	{
		if (m_pd3dDevice == nullptr)
		{
			std::cout << "Failed to load img -> Direct3D device is not initialized" << std::endl;
			return nullptr;
		}
		ID3D11ShaderResourceView* shader;
		D3DX11CreateShaderResourceViewFromMemory(m_pd3dDevice, data, size, nullptr, nullptr, &shader, nullptr);
		// Get the texture size
		ID3D11Resource* resource = nullptr;
		shader->GetResource(&resource);

		D3D11_TEXTURE2D_DESC desc;
		if (resource) {
			ID3D11Texture2D* texture = (ID3D11Texture2D*)resource;
			texture->GetDesc(&desc);
			resource->Release();
		}
		return std::make_shared<Image>(shader, ImVec2(desc.Width, desc.Height));
	}

	std::shared_ptr<Image> Application::LoadImageFromFile(const std::string& path)
	{
		if (m_pd3dDevice == nullptr)
		{
			std::cout << "Failed to load img -> Direct3D device is not initialized" << std::endl;
			return nullptr;
		}
		ID3D11ShaderResourceView* shader;
		D3DX11CreateShaderResourceViewFromFileA(m_pd3dDevice, path.c_str(), nullptr, nullptr, &shader, nullptr);
		// Get the texture size
		ID3D11Resource* resource = nullptr;
		shader->GetResource(&resource);
		D3D11_TEXTURE2D_DESC desc;
		if (resource) {
			ID3D11Texture2D* texture = (ID3D11Texture2D*)resource;
			texture->GetDesc(&desc);
			resource->Release();
		}
		return std::make_shared<Image>(shader, ImVec2(desc.Width, desc.Height));
	}

	LRESULT __stdcall Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		switch (msg)
		{
		case WM_NCHITTEST: {
			if (m_Maximized)
				return HTCLIENT;

			POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
			RECT rect;
			GetClientRect(hWnd, &rect);
			int windowWidth = rect.right;
			int windowHeight = rect.bottom;
			ScreenToClient(hWnd, &pt);

			static const int borderSize = 5;
			static const int titleBarHeight = 40;
			static const int titleBarRightLimit = 135;

			// TOP-LEFT CORNER
			if (pt.y < borderSize && pt.x < borderSize)
				return HTTOPLEFT;

			// TOP-RIGHT CORNER
			if (pt.y < borderSize && pt.x > windowWidth - borderSize)
				return HTTOPRIGHT;

			// BOTTOM-LEFT CORNER
			if (pt.y > windowHeight - borderSize && pt.x < borderSize)
				return HTBOTTOMLEFT;

			// BOTTOM-RIGHT CORNER
			if (pt.y > windowHeight - borderSize && pt.x > windowWidth - borderSize)
				return HTBOTTOMRIGHT;

			// TOP BORDER (Resizable)
			if (pt.y < borderSize)
				return HTTOP;

			// BOTTOM BORDER (Resizable)
			if (pt.y > windowHeight - borderSize)
				return HTBOTTOM;

			// LEFT BORDER (Resizable)
			if (pt.x < borderSize)
				return HTLEFT;

			// RIGHT BORDER (Resizable)
			if (pt.x > windowWidth - borderSize)
				return HTRIGHT;

			// CUSTOM TITLE BAR (Draggable)
			if (pt.y < titleBarHeight && pt.x < windowWidth - titleBarRightLimit)
				return HTCAPTION;
			break;
		}
		case WM_SIZE:
			if (wParam == SIZE_MINIMIZED)
				return 0;
			Application::g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
			Application::g_ResizeHeight = (UINT)HIWORD(lParam);
			ResizeSwapChain();
			Frame(hWnd);
			return 0;
		case WM_SYSCOMMAND:
			if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
				return 0;
			break;
		case WM_DESTROY:
			::PostQuitMessage(0);
			return 0;
		}
		return ::DefWindowProc(hWnd, msg, wParam, lParam);
	}

}
