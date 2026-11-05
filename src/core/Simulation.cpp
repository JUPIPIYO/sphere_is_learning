#include "core/Simulation.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>
#include <numbers>
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

        float distance(Vec2 a, Vec2 b)
        {
            return sqrtf((a.x - b.x) * (a.x - b.x) + (a.z - b.z) * (a.z - b.z));
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

    bool touches(Player p, Obstacle o)
    {
        Shape s = o.shape; 
        if (auto b = std::get_if<Box>(&s))
        {
            Rect r{ o.pos, b->half };
            return distanceSquared(p.pos, Rect{ o.pos, b->half }) <= p.radius * p.radius;
        }
        else if (auto c = std::get_if<Circle>(&s))
        {
            //Circle同士なので簡単
            return (p.pos.x - o.pos.x) * (p.pos.x - o.pos.x) + (p.pos.z - o.pos.z) * (p.pos.z - o.pos.z) <= (p.radius + c->radius) * (p.radius + c->radius);
        }
        else return false;
    }

	//直方体同士の接触判定
	bool overlaps(Rect a, Rect b)
	{
		return abs(a.center.x - b.center.x) <= a.half.x + b.half.x
			&& abs(a.center.z - b.center.z) <= a.half.z + b.half.z;
	}

    //直方体とObstacle
    //これまでに作ったoverlapとtouchesを転用する
    bool overlaps(Rect a, Obstacle o)
    {
        if (auto s = std::get_if<Box>(&o.shape))
        {
            return overlaps(a, Rect{ o.pos, s->half });
        }
        else if (auto s = std::get_if<Circle>(&o.shape))
        {
            return distanceSquared(o.pos, a) <= s->radius * s->radius;
        }
    }

    Vec2 Obstacle::newPosition(double time,Vec2 currentPos , Vec2 playerPos, double dt)
    {
        //TODO : motionごとにtime時点でのposを返す
        Vec2 pos{};
        if(auto m = std::get_if<RoundTrip>(&motion))
        {
            const double length = distance(m->start, m->end);
            double moved_dist = m->speed * time; //これまで進んだ距離
            double current_dist = fmod(moved_dist, 2 * length);
            //startとendをどこに内分する点か、割合で求める
            double per = current_dist <= length ? current_dist / length : (2 * length - current_dist) / length;
            //xで進んだ距離とzで進んだ距離に引き戻す
            pos.x = m->start.x + (m->end.x - m->start.x) * per;
            pos.z = m->start.z + (m->end.z - m->start.z) * per;
        }
        else if(auto m = std::get_if<Orbit>(&motion))
        {
            //円軌道
            double moved_angle = m->defaultPhase + m->angularSpeed * time;
            double current_angle = fmod(moved_angle, 360);

            pos.x = m->center.x + m->radius * cosf(current_angle * numbers::pi / 180);
            pos.z = m->center.z + m->radius * sinf(current_angle * numbers::pi / 180);


        }
        else if(auto m = std::get_if<Chase>(&motion))
        {
            //追跡してくる
            float d = distance(playerPos, currentPos);
            pos.x = currentPos.x + m->speed * dt * (playerPos.x - currentPos.x) / d;
            pos.z = currentPos.z + m->speed * dt * (playerPos.z - currentPos.z) / d;
        }

        return pos;
    }

    //更新位置
    Rect ObstacleRect::currentPosition(double time)
    {
        const double length = high - low;
        double moved_dist = speed * time; //これまで進んだ距離
        double current_dist = fmod(moved_dist, 2 * length);
        if (current_dist <= length)
        {
            //順路
            float current = low + current_dist;
        }
        else
        {
            //往路
            float current = low + (2 * length - current_dist);
        }
        const double u = fmod(speed * time, 2 * length);
        const float value = low + static_cast<float>(u <= length ? u : 2 * length - u);

        Rect b = box;

        if (axis == Axis::X) b.center.x = value;
        else b.center.z = value;

        return b;
    }

    Stage makeStage() {
        Stage s;
        //s.bounds = { {0, 5}, {5, 6} }; //stage 1
        s.bounds = { {0, 10}, {5, 11} }; //stage 2
        //s.bounds = { {0, 15}, {6, 16} }; //stage 3

        Player p;
        p.pos = { 0, 0 };
        s.player = p;

        s.goal = { {0, s.bounds.center.z * 2}, {8, 0.25f} };

        //boundsから自動計算
        float x_max = s.bounds.half.x;
        float z_min = -1.0f;
        float z_max = s.bounds.center.z + s.bounds.half.z;
        float center_z = s.bounds.center.z;
        float x_length_half = s.bounds.half.x;
        float z_length_half = s.bounds.half.z;
        s.walls = 
        { 
            //横の壁 : | |
            {{-1 * x_max, center_z}, {0.2f, z_length_half}}, {{x_max, center_z}, {0.2f, z_length_half}} ,
            //縦の壁 : ＝
            {{0, z_min}, {x_length_half, 0.1f}}, {{0, z_max}, {x_length_half, 0.1f}} ,

            {{3, 13}, {2.5f, 0.2f}},//stage 2
        };
        s.obstacles = {
            //stage 1
            {Box{{0.55f, 0.55f}}, {0, 3}, RoundTrip{{0, 3}, {0, 5}, 2.0f}}, 
            {Box{{0.4f, 0.8f}}, {2, 6}, RoundTrip{{2, 5}, {2, 7}, 1.5f}},
            { Circle{0.5f} , {1, 4}, Orbit{ {1, 3} ,1, 180.0f, 0 } },
            { Circle{0.3f} , {0, 5}, Chase{2.0f} },
            //stage 2
            { Circle{0.7f} , {-3, 2}, Chase{1.5f} },
            {Box{{0.4f, 0.8f}}, {3, 3}, Orbit{{3, 8}, 3 , 180, 2.5f}},
            {Circle{0.5f}, {2, 6}, Orbit{{-2, 14}, 2 , 240, 4.0f}},
            {Circle{0.7f}, {2, 6}, RoundTrip{{-1, 5}, {-3, 18}, 4.0f}},
            //stage 3
            //{Box{{1.0f, 1.0f}}, {2, 6}, RoundTrip{{-1, 5}, {-3, 20}, 3.0f}},
            //{Box{{1.0f, 1.0f}}, {2, 6}, RoundTrip{{4, 16}, {-3, 22}, 4.0f}},
            //{Box{{0.7f, 0.4f}}, {3, 3}, Orbit{{0, 17}, 5 , 90, 5.0f}},
            //{Box{{2.0f, 0.5f}}, {2, 6}, RoundTrip{{-3, 25}, {3, 25}, 2.5f}},
            //{ {{0, 3}, {0.55f, 0.55f}}, Axis::X, -3.5f, 3.5f, 2.0f},
           //{ {{2, 6}, {0.4f, 0.8f}}, Axis::Z, 5.0f, 7.0f, 1.5f}
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
        for (const auto& o : stage_.obstacles) 
        {
            if (false /*TODO  obstacleごとの不正発見*/) throw std::invalid_argument("invalid obstacle"); 
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
        for (int i = 0 ; i < stage_.obstacles.size();i++)
        {
            state_.obstacles[i]= stage_.obstacles[i];
        }

        //現在のZ座標をbestに設定
        bestZ_ = stage_.player.pos.z;
        placeObstacles(0);
    }

    void Environment::placeObstacles(double time) {
        for (std::size_t i = 0; i < state_.obstacles.size(); ++i) 
        {
            Obstacle& o = state_.obstacles[i];
            state_.obstacles[i] = Obstacle{o.shape, o.newPosition(time, o.pos, state().player.pos, dt), o.motion};
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
        for (const auto& o : state_.obstacles) hit = hit || touches(state_.player, o);

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