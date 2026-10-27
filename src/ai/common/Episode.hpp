#pragma once
#include "core/Simulation.hpp"
#include "ai/common/Agent.hpp"
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

	float calucFitness(StepResult result);

	Metrics evaluate(Agent& agent, uint32_t seed);
}