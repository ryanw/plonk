#pragma once

#include "context.h"
#include <chrono>

typedef uint32_t FrameIndex;

class Frame {
public:
	void present();
	~Frame();

private:
	friend class Context;

	Context &ctx;
	FrameIndex index;

	Frame(Context &ctx, FrameIndex index);
};
