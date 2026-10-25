#include "core/Simulation.hpp"
#include <array>

using namespace std;

namespace sil
{
	enum class Cell : uint8_t{
		Empty = 0x00, 
		Wall = 0x01, 
		Obstacle = 0x02, 
		Goal = 0x03
	};

	inline constexpr size_t CellKinds = 4; //セルの種類(セルに存在するオブジェクトの種類)の数
	inline constexpr size_t neighborCount = 8; //隣接マスの数 3x3 - 1
	inline constexpr float cellWidth = 1.0f; //セルの大きさ
	inline constexpr int decisionSteps = 4; //方策を変更するフレーム感覚

	inline constexpr size_t stateSize = 4 ^ 8; //ありえる状況の総数

	typedef array<Cell, neighborCount> Observation; //周囲の状況を格納する配列


	inline constexpr size_t actionSize = 9;

	typedef array<uint8_t, stateSize> Genome; //遺伝子 ： 65535パターンに対するdirec表

	//rectについてその範囲のCell状況を確認
	Cell classifyCell(const Stage& stage,const State& state, Rect& rect)
	{
		for (int i = 0; i < state.obstacles.size(); i++)
		{
			auto obs = state.obstacles[i];
			if (overlaps(obs, rect))
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
				p.x += x;
				p.z += z;

				Rect r = Rect{ p, {cellWidth / 2, cellWidth / 2} };
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


	class GenomeController
	{
	public:
		explicit GenomeController(const Genome& gen)
		{
			genes_ = &gen;
		}

		//envを読み出して次の行動を策定
		Direc nextDirec(const Environment& env)
		{
			if (currentStep_ % 6 == 0) 
			{
				current_ = static_cast<Direc>((*genes_)[toIndex(observe(env))]);
;			}

			currentStep_++;

			return current_;
		}
	private:
		const Genome* genes_;
		int currentStep_{};
		Direc current_{ Direc::Stop };
	};


	struct Metrics
	{
		float fitness{};
		float bestZ;
		float surviveSeconds;
		
		GameState end{ GameState::Running };
	};

	float calucFitness(StepResult result)
	{
		return result.reward;
	}

	Metrics evaluate(const Genome& genes, uint32_t seed)
	{
		//環境を作成し、実際にどんな成果を生み出すかを判定
		Environment env(makeStage());
		env.reset(seed);

		//Genomeを作成
		GenomeController gene(genes);

		StepResult lastResult;
		while (env.state().gameState == GameState::Running)
		{
			Direc d = gene.nextDirec(env);
			lastResult = env.step(d);
		}

		//結果
		return Metrics{ calucFitness(lastResult), env.bestZ(), env.remainSec(), env.state().gameState };

	}

}