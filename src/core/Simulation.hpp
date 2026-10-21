#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace sil {
	//座標
	struct Vec2 { float x{}, z{}; };
	//直方体オブジェクト
	struct Rect {
		Vec2 center{}, half{}; 
	};
	//方向：左右がX、上下がZ
	enum class Axis {X, Z};

	//移動方向
	enum class Direc {Stop, Up, Down, Left, Right, UpRight, UpLeft, DownRight, DownLeft};

	//ゲーム状態
	enum class GameState {Running, Success, Collision, Timeout};
	std::string StateToStr(GameState s);

	//障害物の直方体オブジェクト
	struct Obstacle {
		Rect box; //直方体
		Axis axis{ Axis::X }; //進む向き
		float low{}, high{}, speed{}, phase{};
		//補足:phaseは初期位置からどれだけ進んだか(初期位置はlow)
	};

	struct Player {
		Vec2 pos;
		//プレイヤーの設定
		float radius{ 0.3f };
		float speed{ 3.0f };
	};

	//Stateの初期状態
	struct Stage {
		Rect bounds;
		Player player;
		//制限時間(60fpsでの)
		int maxSteps{ 1200 };
		std::vector<Rect> walls;
		//障害物の初期位置
		std::vector<Obstacle> obstacles;
		Rect goal;
	};

	//Stageの現在の状況
	struct State {
		Player player;
		//現在の障害物の位置・大きさ
		std::vector<Rect> obstacles;
		int currentStep{};
		GameState gameState{ GameState::Running };
	};

	struct StepResult { GameState end; float reward; };

	Vec2 direction(Direc action);

	Direc direcFromAxes(int x, int z);

	bool touches(Player p, Rect box);
	bool overlaps(Rect a, Rect b); //直方体同士の重なりを計算
	Stage makeStage();

	//ゲームの実行環境
	class Environment {
	public:
		static constexpr double dt = 1.0 / 60.0; //60fps
		explicit Environment(Stage stage);
		void reset(std::uint32_t seed); //ゲーム状態をもとに戻す関数
		StepResult step(Direc action);
		const Stage& stage() const { return stage_; }
		const State& state() const { return state_; }
	private:
		bool blocked(Vec2 position) const;
		void placeObstacles(double time);
		Stage stage_;
		State state_;
		float bestZ_{};
	};
}
