#include <post_processing/HBAO.hpp>
#include <random>
#include <texture/Texture.hpp>
#include <glm/glm.hpp>
#include <platform/gui/ImGUI.hpp>

using namespace std; 
using namespace glm;


hbao::HBAO::HBAO(unsigned int windowWidth,
	unsigned int windowHeight) 
	:
	windowWidth(windowWidth),
	windowHeight(windowHeight),
	m_blur_sharpness(40.0f)
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

float hbao::HBAO::randomFloat(float a, float b) {
	uniform_real_distribution<float> dist(a, b);
	random_device device;
	default_random_engine gen(device());
	return dist(gen);
}

float hbao::HBAO::lerp(float a, float b, float f) {
	return a + f*(b - a);
}

class Test {
public:
	void operator()() {
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