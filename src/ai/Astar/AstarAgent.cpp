#include "ai/Astar/AstarAgent.hpp"
#include <unordered_set>


namespace
{
	long long calucKey(sil::Vec2 pos)
	{
		int gridX = static_cast<int>(std::floor((pos.x + sil::boundX) / sil::grid));
		int gridZ = static_cast<int>(std::floor((pos.z + 1) / sil::grid));
		return static_cast<long long>(gridX << 6 + gridZ);
	}
}

namespace sil
{

	AstarAgent::AstarAgent(int _expandDepth = 100, int _maxExpand = 1000)
	{
		expandDepth = _expandDepth;
		maxExpand = _maxExpand;
	}

	int heuristic(const Environment& env, float z_max)
	{
		//今の位置からz_maxまでどれくらいかかるか？ = zmaxまでの直線距離に一致
		//gがstepなのでhをstep数に換算
		return static_cast<int>((z_max - env.state().player.pos.z) / (env.stage().player.speed * env.dt));
	}

	void AstarAgent::reset()
	{
		currentStep_ = 0;
	}



	Direc AstarAgent::nextDirec(const Environment& env)
	{
		//参考：https://www.researchgate.net/publication/224177833_The_2009_Mario_AI_Competition
		//画面に映る分のオブジェクトを保持
		//現在写ってる画面の最北(zがもっとも大きい地点)に最短で行けるノードを展開する
		//そのノードのうち、最初のxステップでの方策(進行方向)を実際に入力する
		//今回x = 5(つまり5ステップ分)だけ前回の方策を使い、次の時点でまた決定を始める
		// また、5フレームは毎回同じ行動を取る(展開数が多くなりすぎるので)。つまり5 x 5 = 25fpsの間は同じ方策を取り続ける
		//「写っている画面」は大体横断に3.5秒かかる(最南から最北まで) = 210step
		// なので展開する深さは100程度に束縛する
		float z_half_space = 6;

		if (currentStep_ % samePlanStep != 0)
		{
			//既存のplanを用いる
			//macroStepだけ同一の行動を取る粒度
			int planIdx = (currentStep_ % samePlanStep) / macroStep;
			currentStep_++;

			return planDirec[planIdx];
		}

		//ノード展開用の仮想envを作成
		//そのための仮想stageをまず作成
		Stage s;
		//playerの位置を中心に上下hald_space = 6だけ取得し、その区間について敵の情報などを取得する
		Player p = env.state().player;
		float z_min = p.pos.z - z_half_space;
		float z_max = p.pos.z + z_half_space;
		//boundsを仮想的に作成
		s.bounds = { {0, p.pos.z},{boundX, 2 * z_half_space} };
		s.player = p;
		s.walls = env.stage().walls;
		//各Obstacleについて、画面内にあれば含める
		for (auto o : env.state().obstacles)
		{
			if (z_min <= o.pos.z && o.pos.z <= z_max)
			{
				s.obstacles.push_back(o);
			}
		}
		s.goal = env.stage().goal;
		//stateについて、stepだけ前の内容をそのまま入れる
		State st;
		st.player = p;
		st.player = s.player;
		st.currentStep = env.state().currentStep;
		st.gameState = GameState::Running;
		st.obstacles.resize(s.obstacles.size());
		for (int i = 0; i < s.obstacles.size(); i++)
		{
			st.obstacles[i] = s.obstacles[i];
		}
		//env完成
		Environment dummyEnv{ s, st };

		int firstStep = st.currentStep;

		//ここからノード展開
		std::vector<Node> nodeVec;
		f_que  open = {};

		int goalIdx = -1;
		int bestIdx = 0;

		//まず現時点を入れる
		int h = heuristic(dummyEnv, z_max);
		nodeVec.push_back({ dummyEnv, 0, h, -1, Direc::Stop, calucKey(dummyEnv.state().player.pos)});
		open.push({ 0 + h, 0});

		int expandTime = 0;

		std::unordered_set<long long> visited;  // 探索開始時に空にする

		//ループ
		while (!open.empty() && expandTime < maxExpand) 
		{
			expandTime++;

			// f最小を取り出す
			int idx = open.top().second;
			open.pop();

			//Step数がexpandDepth以上ならもう無視
			if (nodeVec[idx].g > expandDepth) continue;
	
			if (nodeVec[idx].Env.state().gameState == GameState::Success)
			{
				goalIdx = idx;
				break;
			}
			else if (nodeVec[idx].Env.state().gameState != GameState::Running)
			{
				//失敗状態なら飛ばす
				continue;
			}

			// 子ノードを生成
			for (Direc d : {Direc::Stop, Direc::Up, Direc::Down, Direc::Left, Direc::Right, Direc::UpRight, Direc::UpLeft, Direc::DownRight, Direc::DownLeft})
			{
				//複製を作って、それを1step進める
				auto kidE = nodeVec[idx].Env;
				//macroStep回だけstepを進める
				bool isGameover = false;
				StepResult result;
				for(int i = 0; i < macroStep; i++) 
				{
					result = kidE.step(d);
					//枝刈り ： 起動中じゃなくなった場合
					if (result.end != GameState::Running)
					{
						isGameover = true;
						break;
					}
				}
				//早期終了した場合、Successじゃないなら捨てる
				if (isGameover && result.end != GameState::Success) { continue; }

				int g = (idx == -1) ? 0 : nodeVec[idx].g + macroStep;
				int h = heuristic(kidE, z_max);

				Vec2 currentPos = kidE.state().player.pos;
				long long key = calucKey(currentPos);
				//枝刈り
				if (!visited.insert(key).second) continue;

				nodeVec.push_back({ kidE, g, h, idx, d, key });
				int childIdx = (int)nodeVec.size() - 1;
				open.push({ g + h, childIdx });

				//この時点でのhのbestを入れる
				if (h < nodeVec[bestIdx].h) bestIdx = childIdx;
			}
		}
		int last = (goalIdx != -1) ? goalIdx : bestIdx;  
		//finalNodeから逆順に通っていく
		std::vector<Direc> goDirec = {};
		Node node = nodeVec[last];
		int id = node.parent;
		while (id >= 0)
		{
			goDirec.push_back(node.direc);
			node = nodeVec[id];
			id = node.parent;
		}

		for (int i = 0; i < planDirecSize ; i++)
		{
			if (i < goDirec.size())
			{
				planDirec[i] = goDirec[goDirec.size() - 1 - i];
			}
			else planDirec[i] = Direc::Stop; //Error Handling
		}

		//macroStepだけ同一の行動を取る粒度
		int planIdx = (currentStep_ % samePlanStep) / macroStep;
		currentStep_++;

		return planDirec[planIdx];

	}
}