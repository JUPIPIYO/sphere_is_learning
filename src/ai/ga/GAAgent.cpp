#include "ai/ga/GAAgent.hpp"

namespace sil
{
	GAAgent::GAAgent(const Genome& gen)
	{
		genes_ = &gen;
	}

	void GAAgent::reset()
	{
		currentStep_ = 0;
	}

	Direc GAAgent::nextDirec(const Environment& env)
	{
		if (currentStep_ % decisionSteps == 0)
		{
			current_ = static_cast<Direc>((*genes_)[toIndex(observe(env))]);
			;
		}

		currentStep_++;

		return current_;
	}
}