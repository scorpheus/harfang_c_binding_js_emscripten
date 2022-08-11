/* 
 *  Copyright harfang_c_binding_js_emscripten - 2020-2022 Movida Production - Camille Dudognon - Thomas Simonnet
 */

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/val.h>
#include <emscripten/wire.h>

#include <engine/scene.h>
#include <engine/assets.h>
#include <engine/render_pipeline.h>
#include <engine/forward_pipeline.h>
#include <engine/scene_forward_pipeline.h>
#include <engine/fps_controller.h>

#include <foundation/format.h>
#include <foundation/log.h>
#include <foundation/clock.h>
#include <platform/input_system.h>
#include <platform/window_system.h>

bool AddAssetsFolder(std::string s){
	return hg::AddAssetsFolder(s.c_str());
}

// callback to load file
bool is_downloading;
void progress_load_zip(unsigned, void *, int p) {
	float loading_value = p / 100.f;
	hg::log(hg::format("Loading progress: %1").arg(p));
}
void error_load_zip(unsigned, void *, int err) {
	hg::log(hg::format("Loading error: %1").arg(err));
}

void load_zip(unsigned, void *, const char *file_name) {
	hg::log(hg::format("Loading file: %1").arg(file_name));
	hg::AddAssetsPackage(file_name);
	is_downloading = true;
}

void AddAssetsPackage(std::string s){
	is_downloading = false;
	emscripten_async_wget2(s.c_str(), s.c_str(), "GET", "", nullptr, &load_zip, &error_load_zip, &progress_load_zip);
}

bool AssetsPackageIsAdded(){return is_downloading;}

struct hg::Window {
	uintptr_t unused;
};

hg::Window *NewWindow(int width, int height){
	return hg::NewWindow(width, height);
}
 bool RenderInit(hg::Window *window){
 	return hg::RenderInit(window, bgfx::RendererType::OpenGLES);
 }
static void SetViewClear(bgfx::ViewId view_id, const hg::Color &col) {
	bgfx::setViewClear(view_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL, hg::ColorToABGR32(col));
}
void SetViewRect(bgfx::ViewId _id, uint16_t _x, uint16_t _y, uint16_t _width, uint16_t _height){
	bgfx::setViewRect(_id, _x, _y, _width, _height);
}

bool LoadSceneFromAssets(const std::string &name, hg::Scene &scene, hg::PipelineResources &resources, const hg::PipelineInfo &pipeline){
	hg::LoadSceneContext ctx;
	return hg::LoadSceneFromAssets(name.c_str(), scene, resources, pipeline, ctx, hg::LSSF_All);
}
bgfx::ViewId SubmitSceneToPipeline(bgfx::ViewId view_id, const hg::Scene &scene, const hg::Rect<int> &rect, bool fov_axis_is_horizontal, hg::ForwardPipeline &pipeline,
	const hg::PipelineResources &resources){
	hg::SceneForwardPipelinePassViewId new_pass_views;
	hg::SubmitSceneToPipeline(view_id, scene, rect, fov_axis_is_horizontal, pipeline, resources, new_pass_views);
	return view_id;
}

static const uint16_t CF_Color = BGFX_CLEAR_COLOR;
static const uint16_t CF_Depth = BGFX_CLEAR_DEPTH;
static const uint16_t CF_Stencil = BGFX_CLEAR_STENCIL;

EMSCRIPTEN_BINDINGS(harfang) {
   // init
	emscripten::function("InputInit", &hg::InputInit);
	emscripten::function("WindowSystemInit", &hg::WindowSystemInit);
	
 	emscripten::function("AddAssetsFolder", &AddAssetsFolder);
 	emscripten::function("AddAssetsPackage", &AddAssetsPackage);
 	emscripten::function("AssetsPackageIsAdded", &AssetsPackageIsAdded);	

	emscripten::class_<hg::Window>("Window");
	emscripten::function("NewWindow", &NewWindow, emscripten::allow_raw_pointers());
	
	emscripten::class_<hg::ForwardPipeline>("ForwardPipeline");	
	emscripten::function("CreateForwardPipeline", &hg::CreateForwardPipeline);
	emscripten::function("RenderInit", &RenderInit, emscripten::allow_raw_pointers());

	// scene utils to draw 
	emscripten::function("TickClock", &hg::tick_clock);
	emscripten::function("time_to_sec_f", &hg::time_to_sec_f);

  	emscripten::class_<hg::PipelineInfo>("PipelineInfo");
  	emscripten::class_<hg::PipelineResources>("PipelineResources")
    	.constructor();

	emscripten::function("GetForwardPipelineInfo", &hg::GetForwardPipelineInfo);

  	emscripten::class_<hg::Mouse>("Mouse")
    	.constructor()
        .function("Update", &hg::Mouse::Update);

  	emscripten::class_<hg::Keyboard>("Keyboard")
    	.constructor()
        .function("Update", &hg::Keyboard::Update)
        .function("Down", &hg::Keyboard::Down)
        .function("Pressed", &hg::Keyboard::Pressed)
        .function("Released", &hg::Keyboard::Released);

	emscripten::enum_<hg::Key>("Key")
        .value("Z", hg::Key::K_Z)
        .value("Q", hg::Key::K_Q)
        .value("S", hg::Key::K_S)
        .value("D", hg::Key::K_D)
        ;
		
	emscripten::function("FpsController", emscripten::select_overload<void(const hg::Keyboard &, const hg::Mouse &, hg::Vec3 &, hg::Vec3 &, float, hg::time_ns)>(&hg::FpsController));

  	emscripten::class_<hg::Node>("Node")
    	.constructor()
        .function("GetTransform", &hg::Node::GetTransform);

  	emscripten::class_<hg::Scene>("Scene")
    	.constructor()
        .function("Update", &hg::Scene::Update)
        .function("GetNode", emscripten::select_overload<hg::Node(const std::string &) const>(&hg::Scene::GetNode));

  	emscripten::class_<hg::Vec3>("Vec3")
		.property("x", &hg::Vec3::x)
		.property("y", &hg::Vec3::y)
		.property("z", &hg::Vec3::z);

  	emscripten::class_<hg::Transform>("Transform")
    	.constructor()
        .function("GetPos", &hg::Transform::GetPos)
        .function("SetPos", &hg::Transform::SetPos)
        .function("GetRot", &hg::Transform::GetRot)
        .function("SetRot", &hg::Transform::SetRot);

  	emscripten::class_<hg::Rect<int>>("IntRect")
    	.constructor<int , int , int , int>();
	
	emscripten::function("LoadSceneFromAssets", &LoadSceneFromAssets);
	emscripten::function("SubmitSceneToPipeline", &SubmitSceneToPipeline);

  	emscripten::class_<hg::Color>("Color")
    	.constructor<float , float , float , float>()
    	.constructor<float , float , float>();
	emscripten::function("ColorToABGR32", &hg::ColorToABGR32);

	//render	
	emscripten::constant("CF_Color", CF_Color);
	emscripten::constant("CF_Depth", CF_Depth);
	emscripten::constant("CF_Stencil", CF_Stencil);
	emscripten::function("SetViewClear", &SetViewClear);
	emscripten::function("SetViewRect", &SetViewRect);
	emscripten::function("touch", &bgfx::touch);
		
	emscripten::function("UpdateWindow", &hg::UpdateWindow, emscripten::allow_raw_pointers());
	emscripten::function("frame", &bgfx::frame);
}