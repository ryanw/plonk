#include "include/plonk/frame.h"

Frame::Frame(Context &ctx, FrameIndex index) : ctx(ctx), index(index) {
}


Frame::~Frame() {}

void Frame::present() {
	ctx.presentFrame(*this);
}

