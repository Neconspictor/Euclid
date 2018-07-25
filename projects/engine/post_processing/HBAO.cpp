#include <post_processing/HBAO.hpp>
#include <glm/glm.hpp>
#include <platform/gui/ImGUI.hpp>
#include <iostream>
#include <chrono>
#include <random>
#include <boost/random.hpp>

// GCC under MINGW has no support for a real random device!
#if defined(__MINGW32__)  && defined(__GNUC__)
#include <boost/random/random_device.hpp>
#endif

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

hbao::HBAO_ConfigurationView::HBAO_ConfigurationView(HBAO * hbao, nex::engine::gui::Menu* confifMenu) : m_hbao(hbao), m_showConfigMenu(false)
{

	using namespace nex::engine::gui;

	m_blur_sharpness = m_hbao->getBlurSharpness();

	MenuItemPtr menuItem = std::make_unique<MenuItem>([&](MenuItem* menuItem)
	{
		if (ImGui::Checkbox("HBAO", &m_showConfigMenu)){}

	});

	confifMenu->addMenuItem(std::move(menuItem));
}

void hbao::HBAO_ConfigurationView::drawSelf()
{
	// update hbao model
	m_hbao->setBlurSharpness(m_blur_sharpness);

	if (m_showConfigMenu) {
		ImGui::Begin("", NULL, ImGuiWindowFlags_NoTitleBar);
		// render configuration properties
		ImGui::Text("HBAO configuration");
		ImGui::SliderFloat("blur sharpness", &m_blur_sharpness, 0.0f, 1000.0f);
		ImGui::End();
	}
}