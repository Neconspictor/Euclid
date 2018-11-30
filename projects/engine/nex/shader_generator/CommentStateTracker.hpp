#pragma once
#include <type_traits>
#include "StateTracker.hpp"

namespace nex
{
	enum class CommentState
	{
		NO_COMMENT, // We are currently in no comment
		LINE_COMMENT, // We are in a line comment
		MULTI_LINE_COMMENT, // We are in a multi line comment
	};

	class CommentStateTracker : public StateTracker
	{
	public:

		CommentStateTracker(CommentState initialState = CommentState::NO_COMMENT);

		void update(const StreamPos& streamPos) override;
		bool isActive() const override;

	private:

		static bool isCommentState(CommentState state);

		/**
		* Holds the current state of Comment
		*/
		CommentState mState;

		template <typename E>
		static constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
			return static_cast<typename std::underlying_type<E>::type>(e);
		}
	};
}