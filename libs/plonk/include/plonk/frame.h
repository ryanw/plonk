#pragma once

#include "context.h"

typedef uint32_t FrameIndex;

class Frame {
public:
	void present();
	~Frame();

private:
	friend class Context;

	ContextPtr ctx;
	FrameIndex index;

	Frame(ContextPtr ctx, FrameIndex index);
};
