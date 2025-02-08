#pragma once
namespace Bedxo
{
	class Application;
	class Layer
	{
	public:
		Layer() {}
		virtual ~Layer() {}
		virtual void OnDraw(Application* app) {};
		// Called every frame
		virtual void OnRender(Application* app) {};
		// Called only once
		virtual void OnStart(Application* app) {};
	};
}