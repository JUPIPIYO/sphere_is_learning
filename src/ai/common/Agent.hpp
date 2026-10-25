#include "core/Simulation.hpp"

namespace sil
{
    class Agent {
    public:
        virtual ~Agent() = default;
        virtual void reset() = 0;
        virtual Direc nextDirec(const Environment& env) = 0;
    };
}