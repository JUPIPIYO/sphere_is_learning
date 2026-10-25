#include "core/Simulation.hpp"
#include <array>

namespace sil
{
	struct Metrics
	{
		float fitness{0.0f};
		float bestZ{};
		float surviveSeconds{};

		GameState end{ GameState::Running };
	};
}