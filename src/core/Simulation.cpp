#include "core/Simulation.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
using namespace std;

namespace sil {
	//無名空間に補助関数置いておく
	namespace 
	{
		//位置pと直方体の距離を取得
		float distanceSquared(Vec2 p, Rect b)
		{
			//pとの最小x・z距離を取得する
			//直方体の内部だったら0
			float x = clamp(p.x, b.center.x - b.half.x, b.center.x + b.half.x);
			const float z = std::clamp(p.z, b.center.z - b.half.z, b.center.z + b.half.z);
			return (p.x - x) * (p.x - x) + (p.z - z) * (p.z - z);
		}
	}

    string StateToStr(GameState s)
    {
        if (s == GameState::Running) return "Running";
        else if (s == GameState::Success) return "Success";
        else if (s == GameState::Collision) return "Collision";
        else /*(s == GameState::Timeout)*/ return "Timeout";
    }

	//xとzの値から向かう方向Direcを決定
	Direc direcFromAxes(int x, int z)
	{
		if (z > 0)
		{
			if (x > 0) return Direc::UpRight;
			else if (x < 0) return Direc::UpLeft;
			else return Direc::Up;
		}
		else if (z < 0)
		{
			if (x > 0) return Direc::DownRight;
			else if (x < 0) return Direc::DownLeft;
			else return Direc::Down;
		}
		else
		{
			if (x > 0) return Direc::Right;
			else if (x < 0) return Direc::Left;
			else return Direc::Stop;
		}
	}

	//逆変換
	Vec2 direction(Direc direc) {
		Vec2 d{};
		switch (direc) {
		case Direc::Stop: break;
		case Direc::Up: d.z = 1; break;
		case Direc::Down: d.z = -1; break;
		case Direc::Left: d.x = -1; break;
		case Direc::Right: d.x = 1; break;
		case Direc::UpRight: d = { 1, 1 }; break;
		case Direc::DownRight: d = { 1, -1 }; break;
		case Direc::UpLeft: d = { -1, 1 }; break;
		case Direc::DownLeft: d = { -1, -1 }; break;
		}
		//長さを合わせる
		const float length = std::sqrt(d.x * d.x + d.z * d.z);
		if (length > 0) { d.x /= length; d.z /= length; }
		return d;
	}

	bool touches(Player p, Rect b)
	{
		//接触判定
		return distanceSquared(p.pos, b) <= p.radius * p.radius;
	}

	//直方体同士の接触判定
	bool overlaps(Rect a, Rect b)
	{
		return abs(a.center.x - b.center.x) <= a.half.x + b.half.x
			&& abs(a.center.z - b.center.z) <= a.half.z + b.half.z;
	}

    Stage makeStage() {
        Stage s;
        s.bounds = { {0, 5}, {5, 6} };

        Player p;
        p.pos = { 0, 0 };
        s.player = p;

        s.goal = { {0, 10}, {4.5f, 0.25f} };
        s.walls = { {{-2.8f, 5.5f}, {0.3f, 1.0f}} };
        s.obstacles = {
            {{{0, 3}, {0.55f, 0.55f}}, Axis::X, -3.5f, 3.5f, 2.0f, 0.0f},
            {{{2, 6}, {0.4f, 0.8f}}, Axis::Z, 5.0f, 7.0f, 1.5f, 0.0f}
        };
        return s;
    }

    Environment::Environment(Stage stage) : stage_(move(stage)) 
    {
        //Stage設定が不正なものだったらNG
        Player p = stage_.player;
        bool isNG = p.radius <= 0 || p.speed <= 0 || stage_.maxSteps <= 0 || stage_.bounds.half.x < p.radius || stage_.bounds.half.z < p.radius;
        if (isNG) 
        {
            throw std::invalid_argument("invalid stage dimensions or timing");
        }

        //ステージ内の各obstavleについて、不正を探す
        for (const auto& o : stage_.obstacles) {
            if (!(o.high > o.low) || !(o.speed >= 0) || !(o.phase >= 0)|| !(o.box.half.x > 0) || !(o.box.half.z > 0)) {
                throw std::invalid_argument("invalid obstacle");
            }
        }
        reset(0);
        if (blocked(state_.player.pos)) throw std::invalid_argument("start is blocked");
    }

    void Environment::reset(std::uint32_t seed) {
        // このステージは乱数を使わない。将来ランダム化する場合も外部のseedで制御する。
        (void)seed;
        state_.player = stage_.player;
        state_.currentStep = 0;
        state_.gameState = GameState::Running;
        state_.obstacles.resize(stage_.obstacles.size());

        //現在のZ座標をbestに設定
        bestZ_ = stage_.player.pos.z;
        placeObstacles(0);
    }

    void Environment::placeObstacles(double time) {
        for (std::size_t i = 0; i < stage_.obstacles.size(); ++i) 
        {
            auto& o = stage_.obstacles[i];

            const double length = o.high - o.low;
            double moved_dist = o.phase + o.speed * time; //これまで進んだ距離
            double current_dist = fmod(moved_dist, 2 * length);
            if (current_dist <= length)
            {
                //順路
                float current = o.low + current_dist;
            }
            else
            {
                //往路
                float current = o.low + (2 * length - current_dist);
            }
            const double u = fmod(o.phase + o.speed * time, 2 * length);
            const float value = o.low + static_cast<float>(u <= length ? u : 2 * length - u);

            if (o.axis == Axis::X) o.box.center.x = value;
            else o.box.center.z = value;

            state_.obstacles[i] = o.box;
        }
    }

    //playerが壁にぶつかっていないか
    bool Environment::blocked(Vec2 p) const {
        const auto b = stage_.bounds;
        const float r = stage_.player.radius;
        if (p.x - r< b.center.x - b.half.x|| p.x + r> b.center.x + b.half.x
            || p.z - r < b.center.z - b.half.z || p.z + r > b.center.z + b.half.z) return true;
        for (const auto& wall : stage_.walls) {
            if (distanceSquared(p, wall) < r * r) return true;
        }
        return false;
    }

    StepResult Environment::step(Direc direc) {
        if (state_.gameState != GameState::Running) throw std::logic_error("reset before stepping a finished episode");

        const Vec2 d = direction(direc);

        bool hit = false;
        bool goaled = false;

        //constexpr int parts = 4;
        const float move = stage_.player.speed * dt; //移動量

        Vec2 next = state_.player.pos;
        next.x += d.x * move;
        if (!blocked(next)) state_.player.pos.x = next.x; //この位置でblockしないならgo

        next = state_.player.pos;
        next.z += d.z * move;
        if (!blocked(next)) state_.player.pos.z = next.z;

        //何フレーム目かを把握
        double time = state_.currentStep * dt;

        placeObstacles(time);

        //goal or 障害物接触を判定
        goaled = goaled || touches(state_.player, stage_.goal);
        for (const auto& box : state_.obstacles) hit = hit || touches(state_.player, box);

        /*
        for (int part = 1; part <= parts; ++part) {
            Vec2 next = state_.player.pos;
            next.x += d.x * move;
            if (!blocked(next)) state_.player.pos.x = next.x; //この位置でblockしないならgo

            next = state_.player.pos;
            next.z += d.z * move;
            if (!blocked(next)) state_.player.pos.z = next.z;

            //何フレーム目かを把握
            double time = (state_.currentStep + static_cast<double>(part) / parts) * dt;

            placeObstacles(time);

            //goal or 障害物接触を判定
            goaled = goaled || touches(state_.player, stage_.goal);
            for (const auto& box : state_.obstacles) hit = hit || touches(state_.player, box);
        }
        */

        state_.currentStep++;
        if (goaled) state_.gameState = GameState::Success;
        else if (hit) state_.gameState = GameState::Collision;
        else if (state_.currentStep >= stage_.maxSteps) state_.gameState = GameState::Timeout;

        //ここまでの最良のZと比較して進捗を計算
        const float progress = max(0.0f, state_.player.pos.z - bestZ_);
        bestZ_ = max(bestZ_, state_.player.pos.z);
        float reward = progress - 0.001f;
        if (state_.gameState == GameState::Success) reward += 10.0f;
        if (state_.gameState == GameState::Collision) reward -= 1.0f;
        return { state_.gameState, reward };
    }

}