#include "core/Simulation.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
    // 条件を満たさなければ、理由を表示してテストを失敗させる。
    void check(bool ok, const char* message) {
        if (!ok) throw std::runtime_error(message);
    }
    // 小数の計算誤差を許容して比較する。
    bool near(float a, float b) { return std::abs(a - b) < 0.0001f; }
    // 移動だけを調べるため、壁と障害物を取り除く。
    sil::Stage emptyStage() {
        auto s = sil::makeStage();
        s.walls.clear();
        s.obstacles.clear();
        return s;
    }
}
int main() {
    try {
        // 直進と斜め移動が、1更新で同じ距離だけ進むことを確認する。
        sil::Environment up(emptyStage()), diagonal(emptyStage());
        up.step(sil::Direc::Up);
        diagonal.step(sil::Direc::UpRight);
        const auto p = diagonal.state().player.pos;
        check(near(up.state().player.pos.z, 0.05f), "speed times dt");
        check(near(std::sqrt(p.x * p.x + p.z * p.z), 0.05f), "diagonal speed");
        check(sil::direcFromAxes(0, 0) == sil::Direc::Stop, "cancelled keys");

        // 円が長方形にちょうど接すると接触、隙間があれば非接触になる。
        sil::Player touchingPlayer;
        touchingPlayer.pos = {0, 0};
        touchingPlayer.radius = 1.0f;
        check(sil::touches(touchingPlayer, {{2, 0}, {1, 1}}), "touch includes tangent");
        touchingPlayer.radius = 0.9f;
        check(!sil::touches(touchingPlayer, {{2, 0}, {1, 1}}), "separated circle");

        // 壁の手前で止まり、めり込まないことを確認する。
        auto s = emptyStage();
        s.walls = { {{0, 1}, {1, 0.2f}} };
        sil::Environment wall(s);
        for (int i = 0; i < 30; ++i) wall.step(sil::Direc::Up);
        check(wall.state().player.pos.z <= 0.5f + 0.0001f, "wall blocks movement");

        // 制限回数に達した更新で時間切れになる。
        s = emptyStage();
        s.maxSteps = 1;
        sil::Environment timeout(s);
        check(timeout.step(sil::Direc::Stop).end == sil::GameState::Timeout, "timeout boundary");
        // 終了条件が重なる場合は、成功、衝突、時間切れの順に優先する。
        s.goal = { {0, 0}, {1, 1} };
        s.obstacles = { {{{0, 0}, {1, 1}}, sil::Axis::X, 0, 1, 0, 0} };
        sil::Environment priority(s);
        check(priority.step(sil::Direc::Stop).end == sil::GameState::Success, "success wins all ties");
        s.goal = { {0, 10}, {1, 0.1f} };
        sil::Environment collision(s);
        check(collision.step(sil::Direc::Stop).end == sil::GameState::Collision, "collision before timeout");

        // 同じシードと行動列で、同じ位置になることを確認する。
        sil::Environment a(sil::makeStage()), b(sil::makeStage());
        a.reset(42); b.reset(42);
        for (int i = 0; i < 100; ++i) {
            a.step(sil::Direc::Stop); b.step(sil::Direc::Stop);
            check(a.state().player.pos.z == b.state().player.pos.z, "same player state");
            check(a.state().obstacles[0].center.x == b.state().obstacles[0].center.x, "same obstacle state");
        }
        // リセットで更新回数、ゲーム状態、障害物の位相が初期値に戻る。
        a.reset(42);
        check(a.state().currentStep == 0 && a.state().gameState == sil::GameState::Running, "reset terminal state");
        check(near(a.state().obstacles[0].center.x, -3.5f), "reset phase");
        // 1.5秒後は折り返し地点から戻り、X=0.5になる。
        auto turnStage = emptyStage();
        turnStage.obstacles = { {{{0, 5}, {0.5f, 0.5f}}, sil::Axis::X, 0, 1, 1, 0} };
        sil::Environment turn(turnStage);
        for (int i = 0; i < 90; ++i) turn.step(sil::Direc::Stop);
        check(near(turn.state().obstacles[0].center.x, 0.5f), "reflection keeps overshoot");
        std::cout << "simulation tests passed\n";
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
