#include "ai/common/Episode.hpp"
#include "ai/common/Agent.hpp"
#include "core/Simulation.hpp"

namespace sil
{
	float calucFitness(StepResult result)
	{
		return result.reward;
	}

	Metrics evaluate(Agent& agent, uint32_t seed)
	{
		//環境を作成し、実際にどんな成果を生み出すかを判定
		Environment env(makeStage());
		env.reset(seed);


		StepResult lastResult{GameState::Running, 0};
		while (env.state().gameState == GameState::Running)
		{
			Direc d = agent.nextDirec(env);
			lastResult = env.step(d);
		}

		float fitness = env.bestZ();

		//結果
		return Metrics{ fitness, env.bestZ(), env.surviveSec(), env.state().gameState };
	}
}