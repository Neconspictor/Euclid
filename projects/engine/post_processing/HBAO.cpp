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

hbao::HBAO_ConfigurationView::HBAO_ConfigurationView(HBAO * hbao) : m_hbao(hbao), showConfigMenu(false)
{
	m_blur_sharpness = m_hbao->getBlurSharpness();

	menuBar.addMenuItem([]() {
		if (ImGui::MenuItem("Lambda")) {
			std::cout << "called lambda function!" << std::endl;
		}
	}, "Settings");
	menuBar.addMenuItem(Test(), "Settings");
}

void hbao::HBAO_ConfigurationView::drawGUI()
{
	// update hbao model
	m_hbao->setBlurSharpness(m_blur_sharpness);


	menuBar.drawGUI();


	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Edit"))
		{
			if (ImGui::Checkbox("HBAO", &showConfigMenu)) {}
			//if (ImGui::MenuItem("HBAO", "")) {
			//	showConfigMenu = !showConfigMenu;
			//}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (showConfigMenu) {
		ImGui::Begin("", NULL, ImGuiWindowFlags_NoTitleBar);
		// render configuration properties
		ImGui::Text("HBAO configuration");
		ImGui::SliderFloat("blur sharpness", &m_blur_sharpness, 0.0f, 1000.0f);
		ImGui::End();
	}
}