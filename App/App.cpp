#include <iostream>
#include <Bedxo/Application.h>
#include <Bedxo/Layer.h>
#include <memory>

class MainLayer : public Layer
{
    void virtual OnRender() override
    {
        ImGui::Begin("hello");
        ImGui::Text("a");
        if (ImGui::Button("CLICK ME"))
            ++clickedTimes;
        ImGui::SameLine();
        ImGui::Text("Clicked: %d", clickedTimes);
        ImGui::End();
        ImGui::ShowDemoWindow();
    }
private:
    int clickedTimes = 0;
};

class MainLayer2 : public Layer
{
    void virtual OnRender() override
    {
        ImGui::Begin("hello2");
        ImGui::Text("First app");
        if (ImGui::Button("CLICK ME"))
            ++clickedTimes;
        ImGui::SameLine();
        ImGui::Text("Clicked: %d", clickedTimes);
        ImGui::End();
    }
private:
    int clickedTimes = 0;
};


int main()
{
    Application app = Application();
    app.m_WindowTitle = "Tetassasdasdasd";
    auto mainLayer = std::make_shared<MainLayer>();
    auto mainLayer2 = std::make_shared<MainLayer2>();
    app.AddLayer(mainLayer);
    app.AddLayer(mainLayer2);
    app.Start();
}