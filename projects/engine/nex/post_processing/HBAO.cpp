#include <nex/post_processing/HBAO.hpp>
#include <glm/glm.hpp>
#include <nex/gui/ImGUI.hpp>
#include <iostream>
#include <chrono>
#include <random>
#include <boost/random.hpp>
#include <imgui/imgui_internal.h>
#include <nex/gui/Util.hpp>

// GCC under MINGW has no support for a real random device!
#if defined(__MINGW32__)  && defined(__GNUC__)
#include <boost/random/random_device.hpp>
#endif

struct ImGuiNextWindowData;
using namespace std; 
using namespace glm;


hbao::HBAO::HBAO(unsigned int windowWidth,
	unsigned int windowHeight) 
	:
	m_blur_sharpness(40.0f),
	windowWidth(windowWidth),
	windowHeight(windowHeight)
{
}

float hbao::HBAO::getBlurSharpness() const
{
	return m_blur_sharpness;
}

void hbao::HBAO::setBlurSharpness(float sharpness)
{
	m_blur_sharpness = sharpness;
}

float hbao::HBAO::randomFloat(float a, float b)
{
// GCC under MINGW has no support for a real random device!
#if defined(__MINGW32__)  && defined(__GNUC__)
    //typedef boost::mt19937 gen_type;
    //long unsigned int seed = std::chrono::steady_clock::now().time_since_epoch().count();
    boost::random::random_device seeder;
    boost::random::mt19937 rng(seeder());
    boost::random::uniform_real_distribution<double> gen(a, b);
    return gen(rng);
#else
    uniform_real_distribution<double> dist(a, b);
    random_device device;
    mt19937 gen(device());
    return dist(gen);

#endif
}

float hbao::HBAO::lerp(float a, float b, float f) {
	return a + f*(b - a);
}

class Test {
public:
	void operator()() const
	{
		if (ImGui::MenuItem("operator()")) {
			std::cout << "called Test()" << std::endl;
		}
	}
};

hbao::HBAO_ConfigurationView::HBAO_ConfigurationView(HBAO * hbao, 
	nex::engine::gui::Menu* configMenu, Drawable* parent, std::string menuTitle) : m_hbao(hbao), m_menuTitle(menuTitle), m_parent(parent)
{

	using namespace nex::engine::gui;

	m_blur_sharpness = m_hbao->getBlurSharpness();
	m_isVisible = false;


	MenuItemPtr menuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
	{
		std::string label = m_menuTitle + "###" + m_id;
		if (ImGui::Checkbox(label.c_str(), &m_isVisible))
		{
			if (m_parent != nullptr)
				m_parent->setVisible(true);
		}

	});

	configMenu->addMenuItem(std::move(menuItem));
}

void hbao::HBAO_ConfigurationView::drawSelf()
{
	// update hbao model
	m_hbao->setBlurSharpness(m_blur_sharpness);

	if (m_isVisible) {
		ImGuiContext&  context = *ImGui::GetCurrentContext();
		ImGuiNextWindowData windowData = context.NextWindowData;
		
		// render configuration properties
		ImGui::PushID(m_id.c_str());
		ImGui::LabelText("", m_menuTitle.c_str());
		ImGui::SliderFloat("blur sharpness", &m_blur_sharpness, 0.0f, 1000.0f);
		ImGui::SliderFloat("blur sharpness##1", &m_test, 0.0f, 1000.0f);

		//ImGui::Dummy(ImVec2(100, 200));
		ImGui::Dummy(ImVec2(0, 20));
		nex::engine::gui::Separator(2.0f);
		//ImGui::GetWindowDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(),
		//	ImVec2(100.f, 120.f), ImColor(255, 255, 0, 255), "Hello World##HelloWorld!", 0, 0.0f, 0);

		ImGui::PopID();
	}
}