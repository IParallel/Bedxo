# Requirements
* DIRECTX SDK

# How to build
* Clone the repo
* Open the project
* Run

# How to use
* There's a **Bedxo::Application** class under **Application.h**
  * this class takes a **Bedxo::AppConfig** struct as ctor parameter
  ```cpp
    auto appConfig = Bedxo::AppConfig();
    appConfig.Title = "Bedxo Application";
    Bedxo::Application app = Bedxo::Application(appConfig);
  ```
  * You have the **Bedxo::Layer** class to create your imgui layers
  ```cpp
  class ExampleDerivedLayer : public Bedxo::Layer
  {
	void virtual OnRender(Bedxo::Application* app) override
	{
    // your imgui code here
	}

	void virtual OnStart(Bedxo::Application* app) override
	{
    // this calls once per layer
	}
  };
  ```
  * you can add this layers using the **Application::AddLayer** method
  ```cpp
    auto mainLayer = std::make_shared<ExampleDerivedLayer>();
    app.AddLayer(mainLayer);
  ```
  * call the **Application::Start** method (blocking call)
  ```cpp
    app.Start()
  ```
  * All
  ```cpp
    auto appConfig = Bedxo::AppConfig();
    appConfig.Title = "Bedxo Application";
    Bedxo::Application app = Bedxo::Application(appConfig);
    auto mainLayer = std::make_shared<ExampleDerivedLayer>();
    app.AddLayer(mainLayer);
    app.Start();
  ```