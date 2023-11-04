#pragma once

#include "context.h"

class Renderer {
public:
	Renderer(Context *ctx);

private:
	Context *ctx;
};
