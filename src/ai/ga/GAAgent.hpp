#include "core/Simulation.hpp"
#include "ai/common/Observation.hpp"
#include "ai/common/Agent.hpp"

namespace sil
{
	typedef std::array<uint8_t, stateSize> Genome; //遺伝子 ： 65535パターンに対するdirec表

	class GAAgent : public Agent
	{
	public:
		explicit GAAgent(const Genome& gen);
		void reset() override;
		//envを読み出して次の行動を策定
		Direc nextDirec(const Environment& env) override ;
	private:
		const Genome* genes_;
		int currentStep_{};
		Direc current_{ Direc::Stop };
	};
}