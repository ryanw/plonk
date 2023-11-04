#include "include/plonk/renderer.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <optional>
#include <vector>

Renderer::Renderer(Context *ctx) : ctx(ctx) { std::cout << "Creating Renderer\n"; }
