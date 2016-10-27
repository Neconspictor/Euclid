#include "platform/windows/input/gainput/HoldResetGesture.hpp"
#include <gainput/gainput.h>
#include <gainput/gestures/GainputHoldGesture.h>
#include <gainput/GainputInputState.h>
#include <gainput/GainputHelpers.h>

namespace gainput {

	HoldResetGesture::HoldResetGesture(InputManager& manager, DeviceId device, unsigned index, DeviceVariant /*variant*/) :
		InputGesture(manager, device, index), xTolerance_(0), yTolerance_(0), oneShot_(false),
		oneShotReset_(true), timeSpan_(0),
		firstDownTime_(0), firstDownX_(0), firstDownY_(0)
	{
		actionButton_.buttonId = InvalidDeviceButtonId;
		xAxis_.buttonId = InvalidDeviceButtonId;
		yAxis_.buttonId = InvalidDeviceButtonId;

		state_ = manager_.GetAllocator().New<InputState>(manager.GetAllocator(), 1);
		GAINPUT_ASSERT(state_);
		previousState_ = manager_.GetAllocator().New<InputState>(manager.GetAllocator(), 1);
		GAINPUT_ASSERT(previousState_);
	}

	HoldResetGesture::~HoldResetGesture()
	{
		manager_.GetAllocator().Delete(state_);
		manager_.GetAllocator().Delete(previousState_);
	}

	void
		HoldResetGesture::Initialize(DeviceId actionButtonDevice, DeviceButtonId actionButton, bool oneShot, uint64_t timeSpan)
	{
		actionButton_.deviceId = actionButtonDevice;
		actionButton_.buttonId = actionButton;
		xAxis_.buttonId = InvalidDeviceButtonId;
		yAxis_.buttonId = InvalidDeviceButtonId;
		oneShot_ = oneShot;
		timeSpan_ = timeSpan;
	}

	void
		HoldResetGesture::Initialize(DeviceId actionButtonDevice, DeviceButtonId actionButton,
			DeviceId xAxisDevice, DeviceButtonId xAxis, float xTolerance,
			DeviceId yAxisDevice, DeviceButtonId yAxis, float yTolerance,
			bool oneShot,
			uint64_t timeSpan)
	{
		actionButton_.deviceId = actionButtonDevice;
		actionButton_.buttonId = actionButton;
		xAxis_.deviceId = xAxisDevice;
		xAxis_.buttonId = xAxis;
		xTolerance_ = xTolerance;
		yAxis_.deviceId = yAxisDevice;
		yAxis_.buttonId = yAxis;
		yTolerance_ = yTolerance;
		oneShot_ = oneShot;
		timeSpan_ = timeSpan;
	}

	void
		HoldResetGesture::InternalUpdate(InputDeltaState* delta)
	{
		if (actionButton_.buttonId == InvalidDeviceButtonId)
		{
			return;
		}

		const InputDevice* actionDevice = manager_.GetDevice(actionButton_.deviceId);
		GAINPUT_ASSERT(actionDevice);

		bool positionValid = false;
		float posX = 0.0f;
		float posY = 0.0f;
		if (xAxis_.buttonId != InvalidDeviceButtonId && yAxis_.buttonId != InvalidDeviceButtonId)
		{
			const InputDevice* xAxisDevice = manager_.GetDevice(xAxis_.deviceId);
			GAINPUT_ASSERT(xAxisDevice);
			posX = xAxisDevice->GetFloat(xAxis_.buttonId);
			const InputDevice* yAxisDevice = manager_.GetDevice(yAxis_.deviceId);
			GAINPUT_ASSERT(yAxisDevice);
			posY = yAxisDevice->GetFloat(yAxis_.buttonId);
			positionValid = true;
		}

		if (actionDevice->GetBool(actionButton_.buttonId))
		{
			if (firstDownTime_ == 0)
			{
				firstDownTime_ = manager_.GetTime();
				firstDownX_ = posX;
				firstDownY_ = posY;
			}
		}
		else
		{
			oneShotReset_ = true;
			firstDownTime_ = 0;
			HandleButton(*this, *state_, delta, HoldTriggered, false);
			return;
		}

		if (positionValid
			&& (Abs(posX - firstDownX_) > xTolerance_ || Abs(posY - firstDownY_) > yTolerance_))
		{
			firstDownTime_ = 0;
		}

		bool downLongEnough = firstDownTime_ + timeSpan_ <= manager_.GetTime();

		if (downLongEnough)
		{
			firstDownTime_ = manager_.GetTime();
		}

		if (oneShot_)
		{
			if (downLongEnough && oneShotReset_)
			{
				HandleButton(*this, *state_, delta, HoldTriggered, true);
				oneShotReset_ = false;
			}
			else
			{
				HandleButton(*this, *state_, delta, HoldTriggered, false);
			}
		}
		else
		{
			HandleButton(*this, *state_, delta, HoldTriggered, downLongEnough);
		}
	}

}