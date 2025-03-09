#pragma once

#include "context.h"
#include <chrono>

typedef uint32_t FrameIndex;

class Frame {
public:
	void present();
	~Frame();

private:
	friend Context;
	friend class Context;

	ContextPtr ctx;
	FrameIndex index;

	Frame(ContextPtr ctx, FrameIndex index);
};
