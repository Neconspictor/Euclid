#pragma once
#include <nex/gui/Style.hpp>

namespace nex::gui
{
	class AppStyle : public nex::gui::Style
	{
	public:
		virtual ~AppStyle() = default;
		void apply() override;
	};

	class ConfigurationStyle : public nex::gui::StyleClass
	{
	protected:
		void pushStyleChangesSelf() override;
		void popStyleChangesSelf() override;
	};

	class ConfigurationStyle2 : public nex::gui::StyleClass
	{
	protected:
		void pushStyleChangesSelf() override;
		void popStyleChangesSelf() override;
	};
}
