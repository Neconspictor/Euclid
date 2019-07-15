#pragma once
namespace nex {

	template<class Container>
	struct Range {
		typename Container::iterator begin;
		typename Container::iterator end;
	};

	template<class Container>
	class ConstRange {
	public:
		//using iterator = typename Container::const_iterator;
		using const_iterator = typename Container::const_iterator;


		explicit ConstRange() {}

		ConstRange(const Container& container) {
			mBegin = container.begin();
			mEnd = container.end();
		}

		const_iterator begin() const {
			return mBegin;
		}

		const_iterator end() const {
			return mEnd;
		}

	private:

		const_iterator mBegin;
		const_iterator mEnd;
	};
}