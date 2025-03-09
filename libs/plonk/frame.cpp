#include "include/plonk/frame.h"

Frame::Frame(ContextPtr ctx, FrameIndex index) : ctx(ctx), index(index) {
}


Frame::~Frame() {}

void Frame::present() {
	ctx->present_frame(*this);
}

