#ifndef UTIL_HPP
#define UTIL_HPP

namespace util
{
	template <typename F>
	struct FinalAction {
		FinalAction(F f) : clean_{ f } {}
		~FinalAction() { if (enabled_) clean_(); }
		void disable() { enabled_ = false; };
	private:
		F clean_;
		bool enabled_{ true };
	};

	namespace global_path {
		static const std::string TEXTURE_PATH = "./_work/data/textures/";
		static const std::string SHADER_PATH = "./shaders/";
	}
}

template <typename F>
util::FinalAction<F> finally(F f) {
	return util::FinalAction<F>(f);
}

#endif UTIL_HPP