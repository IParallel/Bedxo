#include <iostream>
#include <Bedxo/Application.h>
#include <Bedxo/Layer.h>
#include <memory>
#include "logo.h"
class MainLayer : public Bedxo::Layer
{
	void virtual OnRender(Bedxo::Application* app) override
	{
		ImGui::Begin("hello");
		ImGui::Text("a");
		if (ImGui::Button("CLICK ME"))
			++clickedTimes;
		ImGui::SameLine();
		ImGui::Text("Clicked: %d", clickedTimes);
		ImGui::Text("Logo texture: %x", m_Logo->GetTexture());
		ImGui::ImageButton("Click Me", m_Logo->GetTexture(), m_Logo->GetSize());

		static float t = 0;
		static float values[120];
		for (int i = 0; i < 120; i++)
		{
			values[i] = 0.5f + 0.5f * sinf(2 * 3.1415f * 0.1f * (i + t));
		}
		t += 0.1f;
		ImGui::PlotLines("Frame Times", values, IM_ARRAYSIZE(values));

		ImGui::End();
		//ImGui::ShowDemoWindow();
	}

	void virtual OnStart(Bedxo::Application* app) override
	{
		m_Logo = app->LoadImageFromMemory((void*)::logo, sizeof(::logo));
		app->m_Config.TitleBarIconData = m_Logo;
	}
private:
	int clickedTimes = 0;
	std::shared_ptr<Bedxo::Image> m_Logo = nullptr;
};

class MainLayer2 : public Bedxo::Layer
{
	void virtual OnRender(Bedxo::Application* app) override
	{
		ImGui::Begin("hello2");
		ImGui::Text("First app");
		if (ImGui::Button("CLICK ME"))
			++clickedTimes;
		ImGui::SameLine();
		ImGui::Text("Clicked: %d", clickedTimes);
		ImGui::Text("Logo texture: %x", m_Logo->GetTexture());
		ImGui::Image(m_Logo->GetTexture(), m_Logo->GetSize());
		ImGui::End();
	}

	void virtual OnStart(Bedxo::Application* app) override
	{
		try {
			m_Logo = app->LoadImageFromFile("./favicon.png");
		}
		catch (...)
		{
			std::cout << "Failed to load image" << std::endl;
		}
	}
private:
	int clickedTimes = 0;
	std::shared_ptr<Bedxo::Image> m_Logo = nullptr;
};

#ifdef _DIST
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main()
#endif
{
	auto appConfig = Bedxo::AppConfig();
	appConfig.Title = "Bedxo Application";
	Bedxo::Application app = Bedxo::Application(appConfig);
	auto mainLayer2 = std::make_shared<MainLayer2>();
	auto mainLayer = std::make_shared<MainLayer>();
	app.AddLayer(mainLayer);
	app.AddLayer(mainLayer2);
	app.Start();
}