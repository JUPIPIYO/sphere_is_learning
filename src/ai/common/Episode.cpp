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
		//ŠÂ‹«‚ğì¬‚µAÀÛ‚É‚Ç‚ñ‚È¬‰Ê‚ğ¶‚İo‚·‚©‚ğ”»’è
		Environment env(makeStage());
		env.reset(seed);


		StepResult lastResult{GameState::Running, 0};
		while (env.state().gameState == GameState::Running)
		{
			Direc d = agent.nextDirec(env);
			lastResult = env.step(d);
		}

		//Œ‹‰Ê
		return Metrics{ calucFitness(lastResult), env.bestZ(), env.remainSec(), env.state().gameState };
	}
}