#include "core/Simulation.hpp"
#include "ai/common/Observation.hpp"
#include <array>

using namespace std;

namespace sil
{
	//rectについてその範囲のCell状況を確認
	Cell classifyCell(const Stage& stage, const State& state, Rect& rect)
	{
		for (int i = 0; i < state.obstacles.size(); i++)
		{
			auto obs = state.obstacles[i];
			if (overlaps(rect, obs))
			{
				return Cell::Obstacle;
			}
		}

		if (overlaps(rect, stage.goal)) return Cell::Goal;

		for (int i = 0; i < stage.walls.size(); i++)
		{
			auto obs = stage.walls[i];
			if (overlaps(obs, rect))
			{
				return Cell::Wall;
			}
		}

		//この時点で何もなければempty
		return Cell::Empty;
	}

	//Player周囲の情報を実際に見てobservationを作成
	Observation observe(const Environment& env)
	{
		Observation obs{};
		auto p = env.state().player.pos;

		//Playerの周囲8マスを順に見ていく
		int idx = 0;
		for (float x : { 1 * cellWidth, 0.0f, -1 * cellWidth })
		{
			for (float z : { 1 * cellWidth, 0.0f, -1 * cellWidth})
			{
				if (x == 0 && z == 0) continue;

				Rect r = Rect{ {p.x + x, p.z + z}, {cellWidth / 2, cellWidth / 2}};
				obs[idx] = classifyCell(env.stage(), env.state(), r);

				idx++;
			}
		}

		return obs;
	}

	//Observationの内容を0 ~ 65535の値に変換
	size_t toIndex(const Observation& observation)
	{
		size_t gene = 0;
		for (int i = 0; i < observation.size(); i++)
		{
			gene += static_cast<int>(observation[i]) << (2 * i);
		}

		return gene;
	}
}