#pragma once

class nk_context;
struct NVGcontext;

class Application {
private:
	Application(const Application&);
	Application& operator=(const Application&);
public:
	inline Application() { }
	inline virtual ~Application() { Shutdown(); }

	inline virtual void Initialize() {}
	inline virtual void Update(float inDeltaTime) { }
	inline virtual void Render(float inAspectRatio) { }
	inline virtual void ImGui(nk_context* inContext) { }
	inline virtual void NanoGui(NVGcontext* inContext) { }
	inline virtual void Shutdown() { }
};

