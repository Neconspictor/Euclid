#pragma once
#include <gui/Style.hpp>

namespace App
{
	class AppStyle : public nex::engine::gui::Style
	{
	public:
		virtual ~AppStyle() = default;
		void apply() override;
	};

	class ConfigurationStyle : public nex::engine::gui::StyleClass
	{
	public:
		void pushStyleChanges() override;
		void popStyleChanges() override;
	};

	class ConfigurationStyle2 : public nex::engine::gui::StyleClass
	{
	public:
		void pushStyleChanges() override;
		void popStyleChanges() override;
	};
}
