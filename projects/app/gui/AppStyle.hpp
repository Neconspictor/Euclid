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
	protected:
		void pushStyleChangesSelf() override;
		void popStyleChangesSelf() override;
	};

	class ConfigurationStyle2 : public nex::engine::gui::StyleClass
	{
	protected:
		void pushStyleChangesSelf() override;
		void popStyleChangesSelf() override;
	};
}
