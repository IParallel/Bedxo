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
		ImGui::Text("Logo texture: %d", m_Logo->GetTexture());
		ImGui::ImageButton("Click Me",m_Logo->GetTexture(), m_Logo->GetSize());
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
        ImGui::Text("Logo texture: %d", m_Logo->GetTexture());
        ImGui::Image(m_Logo->GetTexture(), m_Logo->GetSize());
        ImGui::End();
    }

    void virtual OnStart(Bedxo::Application* app) override
    {
        m_Logo = app->LoadImageFromFile("./favicon.png");
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
    auto mainLayer = std::make_shared<MainLayer>();
    auto mainLayer2 = std::make_shared<MainLayer2>();
    app.AddLayer(mainLayer);
    app.AddLayer(mainLayer2);
    app.Start();
}