#pragma once
#include "core/Simulation.hpp"
#include "ai/common/Observation.hpp"
#include "ai/common/Agent.hpp"
#include <functional>   // std::greater
#include <queue>
#include <utility>      // std::pair
#include <vector>

namespace sil
{
	//展開する深さは100程度に束縛する
	//決めた方向の10フレーム移動
	inline constexpr int macroStep = 10;
	inline constexpr int planDirecSize = 5;
	inline constexpr int samePlanStep = macroStep * planDirecSize;

	inline constexpr float grid = 0.2f; //x,zを丸める時の単位 macroStepでplayerが動く範囲を下に算出

	class AstarAgent : public Agent
	{
	public:
		explicit AstarAgent(int _expandDepth, int _maxExpand);
		void reset() override;
		//envを読み出して次の行動を策定
		Direc nextDirec(const Environment& env) override;
	private:
		int currentStep_{};
		Direc current_{ Direc::Stop };
		std::array<Direc, planDirecSize> planDirec;
		int expandDepth;
		int maxExpand;
	};

	struct Node {
		Environment Env; //現在の環境
		int g;           // 経過step数
		int h;         // 右端までの推定step数
		int parent;      // 親ノードが 何番目か
		Direc direc;   // 親からここに来た行動(方向)
		//このノードが存在する(x, z)座標を丸めたもの 枝刈りに使う
		long long key;
	};

	using Entry = std::pair<int, int>;
	using f_que = std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>>;
}