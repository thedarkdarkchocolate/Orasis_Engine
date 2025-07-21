#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Render.hpp"
#include "GameObject.hpp"
#include "Kmb_movement_controller.hpp"
#include "Descriptors.hpp"
#include "Render_Systems/RenderSystem.hpp"
#include "Render_Systems/PointLightSystem.hpp"

#include <memory>
#include <chrono>
#include <vector>
#include <stdexcept>
#include <array>