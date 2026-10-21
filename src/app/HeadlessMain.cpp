#include "core/Simulation.hpp"
#include <iostream>

using namespace sil;

int main()
{
	Environment env(makeStage());

	env.reset(42);

	std::cout << "Game Start." << "\n";

	while (env.state().gameState == GameState::Running)
	{
		//‚Æ‚è‚ ‚¦‚¸ã•ûŒü‚Éi‚Þ•ûŽ®‚Å
		env.step(Direc::Up);
	}

	std::cout << "end step = " << env.state().currentStep << "\n"
		<< "Z = " << env.state().player.pos.z << "\n"
		<< "end = " << StateToStr(env.state().gameState) << "\n";
}