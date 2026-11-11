#include "ai/common/Episode.hpp"
#include "ai/common/Agent.hpp"
#include "core/Simulation.hpp"

namespace sil
{
	//GAのfitnessを計算
	float calucFitness(const Environment& env)
	{
		if (env.state().gameState == GameState::Success)
		{
			//成功の場合は早いほどgood
			return 100 - env.surviveSec();
		}
		return env.bestZ(); //+ 0.1 * env.surviveSec();
	}

	Metrics evaluate(Agent& agent, uint32_t seed, int stageIdx)
	{
		//環境を作成し、実際にどんな成果を生み出すかを判定
		Environment env(makeStage(stageIdx));
		env.reset(seed);


		StepResult lastResult{GameState::Running, 0};
		while (env.state().gameState == GameState::Running)
		{
			Direc d = agent.nextDirec(env);
			lastResult = env.step(d);
		}

		float fitness = calucFitness(env);

		//結果
		return Metrics{ fitness, env.bestZ(), env.surviveSec(), env.state().gameState };
	}
}