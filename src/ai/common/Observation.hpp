#pragma once
#include "core/Simulation.hpp"
#include <array>

namespace sil
{

	enum class Cell : uint8_t {
		Empty = 0x00,
		Wall = 0x01,
		Obstacle = 0x02,
		Goal = 0x03
	};

	inline constexpr size_t CellKinds = 4; //セルの種類(セルに存在するオブジェクトの種類)の数
	inline constexpr size_t neighborCount = 8; //隣接マスの数 3x3 - 1
	inline constexpr float cellWidth = 1.0f; //セルの大きさ
	inline constexpr int decisionSteps = 4; //方策を変更するフレーム感覚

	inline constexpr size_t stateSize = 1 << 16; //ありえる状況の総数

	typedef std::array<Cell, neighborCount> Observation; //周囲の状況を格納する配列


	inline constexpr size_t actionSize = 9;

	//rectについてその範囲のCell状況を確認
	Cell classifyCell(const Stage& stage, const State& state, Rect& rect);

	//Player周囲の情報を実際に見てobservationを作成
	Observation observe(const Environment& env);

	//Observationの内容を0 ~ 65535の値に変換
	size_t toIndex(const Observation& observation);

}