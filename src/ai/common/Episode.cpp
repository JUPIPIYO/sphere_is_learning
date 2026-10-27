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
		//�����쐬���A���ۂɂǂ�Ȑ��ʂ𐶂ݏo�����𔻒�
		Environment env(makeStage());
		env.reset(seed);


		StepResult lastResult{GameState::Running, 0};
		while (env.state().gameState == GameState::Running)
		{
			Direc d = agent.nextDirec(env);
			lastResult = env.step(d);
		}

		//����
		return Metrics{ calucFitness(lastResult), env.bestZ(), env.surviveSec(), env.state().gameState };
	}
}