# sphere is learning — C++とDirect3D 11で作るゲーム開発入門

## 目次

- [この教材の使い方](#guide)
- [第1章 ビルドして動かす](#chapter-1)
- [第2章 ゲームに必要なC++の読み方](#chapter-2)
- [第3章 描画から独立したゲームを作る](#chapter-3)
- [第4章 テストでルールを確かめる](#chapter-4)
- [第5章 WindowsとDirect3D 11](#chapter-5)
- [第6章 周辺を観測して行動を決める](#chapter-6)
- [第7章 遺伝的アルゴリズムで方策を育てる](#chapter-7)
- [第8章 保存・読み込み・CLIを作る](#chapter-8)
- [第9章 検証と作品への仕上げ](#chapter-9)
- [付録 サンプルCLI仕様](#cli)
- [参考資料と検証記録](#references)

<a id="guide"></a>

## この教材の使い方

Wキーを押すと球が進む。この短い動作にも、入力を読む処理、位置を変える処理、画面を描く処理が必要です。位置を変える処理を独立させると、キーボードの代わりにAIから同じゲームを操作できます。これが本作の出発点です。

[実装TODO](todo.md)の手順2以降を、実際に書くコードへ落とし込みます。WindowsへのcloneはTODOの手順1を先に済ませてください。C++の文法をすべて覚える必要はありません。各章のコードを動かし、値を変え、結果を予想して確かめます。

このファイルは教材です。リポジトリの`src/`、`tests/`、`CMakeLists.txt`へ実装を配置したものではありません。本文で「完全例」としたコードブロックは、直前のファイル名へ保存する内容です。「差し込み例」は指定した場所へ追加・置換します。コード中に省略記号がない完全例を順に組み合わせて使います。

### TODOとの対応

| TODO | 教材 | 最初に確認する結果 |
| --- | --- | --- |
| 手順2 | 第1章 | コマンドから実行できる |
| 手順3〜5 | 第2〜4章 | 画面なしで移動・衝突・終了判定ができる |
| 手順6〜8 | 第5章 | Windowsで1ステージを遊べる |
| 手順9 | 第6章 | 周辺の配置を36個の数値へ変換できる |
| 手順10 | 第7〜8章 | 世代更新と保存済みAIの再実行ができる |
| 手順11 | 第9章 | 比較条件と実測結果を説明できる |

### 最後にそろうファイル

以下は学習を進めた後の配置です。一度に全部作る必要はありません。各章で指定されたファイルだけを追加します。AIの短い関数は読み比べやすいようヘッダーへ`inline`で定義しています。複数の`.cpp`から使っても同じ関数の重複定義にならないための指定です。大きくなった場合は宣言と実装へ分けられます。

```text
CMakeLists.txt
src/
  core/Simulation.hpp, Simulation.cpp
  app/HeadlessMain.cpp, WindowsMain.cpp, ExperimentMain.cpp
  renderer/d3d11/Renderer.hpp, Renderer.cpp
  ai/Policy.hpp, Training.hpp, AgentFile.hpp
tests/
  SimulationTests.cpp, PolicyTests.cpp
```

### 教材用の選択と、企画上の要件

企画書の未確定事項について、動く例を作るために以下を仮に選びます。正式採用する際は`specification.md`と`architecture.md`へ判断理由を記録してください。以下の数値を企画の確定要件や、学習性能の検証済み設定とは扱いません。

| 対象 | 教材での選択 |
| --- | --- |
| 座標 | Xが右、Zが奥・ゴール方向、Yが高さ |
| 更新 | 1/60秒。時間切れは更新回数が上限以上になった時点 |
| 操作 | 斜め速度を正規化。反対方向の入力は軸ごとに相殺 |
| 壁 | 円と長方形。X方向、Z方向の順に移動を試し、めり込む軸だけ戻す |
| 障害物・ゴール | 円と長方形の接触を含む判定 |
| 再開 | `step()`は結果を返す。呼び出し側が記録後に`reset()` |
| 観測 | 1マス1.5、3×3、壁・障害物の2成分、現在と前回の2時点 |
| AI | 6ステップごとに行動決定、36入力・16隠れ・9出力 |
| 初期ステージ | コンパイル時に作る設定。外部ステージ読込は追加課題 |

<a id="chapter-1"></a>

## 第1章 ビルドして動かす

### 1.1 コンパイルとリンク

C++ファイルはそのままでは実行できません。コンパイラが各`.cpp`を機械語へ変換し、リンカーがそれらとライブラリを結び付けて実行ファイルを作ります。CMakeは、そのための作業手順を生成する道具です。

WindowsではVisual Studioの「C++によるデスクトップ開発」とWindows SDKを用意し、Developer PowerShellを開いてください。以下のコマンドはプロジェクトの最上位フォルダーで実行します。生成器はインストール済みのVisual StudioにCMakeが選ぶものを使い、`-A x64`で対象を指定します。

最初に作るファイルは2個です。

**完全例：`src/app/Hello.cpp`（この節だけの練習用）**

```cpp
#include <iostream>

int main() {
    std::cout << "sphere is learning\n";
    return 0;
}
```

`#include`は宣言を利用可能にします。`main()`はコンソールアプリの開始地点です。`std::cout`へ文字列を渡すと標準出力へ表示され、`\n`で改行します。`return 0`は正常終了を表します。

**完全例：`CMakeLists.txt`（1.1の練習用。1.2で全体を置換）**

```cmake
cmake_minimum_required(VERSION 3.24)
project(SphereIsLearning LANGUAGES CXX)
add_executable(sphere_hello src/app/Hello.cpp)
target_compile_features(sphere_hello PRIVATE cxx_std_20)
```

```powershell
cmake -S . -B build -A x64
cmake --build build --config Debug
.\build\Debug\sphere_hello.exe
```

期待する表示は`sphere is learning`です。最初のコマンドは構成、2個目はビルド、3個目は実行です。`-S`はソース、`-B`は生成物の場所を指定します。`Debug`はデバッグ情報を含む構成です。

**確認課題：** 文字列を変更して再ビルドしてください。実行結果が変わらないときは、保存漏れ、ビルド失敗、別の実行ファイルを起動していないかを順に調べます。

### 1.2 ゲーム用のターゲットへ進む

第3〜4章のファイルを配置する段階で、CMakeを次の内容へ置き換えます。`Hello.cpp`はこの構成から参照されません。第6章以降のAI用ターゲットは後から追記します。

**完全例：`CMakeLists.txt`（第3〜4章までの構成）**

```cmake
cmake_minimum_required(VERSION 3.24)
project(SphereIsLearning LANGUAGES CXX)

add_library(sphere_core src/core/Simulation.cpp)
target_include_directories(sphere_core PUBLIC src)
target_compile_features(sphere_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(sphere_core PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(sphere_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(sphere_headless src/app/HeadlessMain.cpp)
target_link_libraries(sphere_headless PRIVATE sphere_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(sphere_tests tests/SimulationTests.cpp)
    target_link_libraries(sphere_tests PRIVATE sphere_core)
    add_test(NAME simulation COMMAND sphere_tests)
endif()
```

`add_library`は他のプログラムが利用する部品、`add_executable`は起動するプログラムを作ります。`PUBLIC`にしたインクルード先とC++20指定は、`sphere_core`を利用する側へも伝わります。`PRIVATE`の警告設定は、このライブラリ自身へ適用します。

構成を変更したら`cmake -S . -B build -A x64`を再実行します。テストは次のコマンドです。CTestのテスト登録には`include(CTest)`と`add_test()`を利用しています。[CMake公式資料](https://cmake.org/cmake/help/latest/command/add_test.html)

```powershell
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
.\build\Debug\sphere_headless.exe
```

`.gitignore`には少なくとも`/build/`を追加します。macOSで確認するときは`-A x64`を付けず、別の生成物フォルダーで構成します。

```sh
cmake -S . -B build-mac -DCMAKE_BUILD_TYPE=Debug
cmake --build build-mac
ctest --test-dir build-mac --output-on-failure
```

<a id="chapter-2"></a>

## 第2章 ゲームに必要なC++の読み方

### 2.1 値をまとめる

位置にはXとZの2つの数値が必要です。別々の変数で管理するより、「位置という値」にまとめると、受け渡す内容が明確になります。

**読み方の例。第3章のヘッダーに含まれるため、ここでは別ファイルを作りません。**

```cpp
struct Vec2 {
    float x{};
    float z{};
};

Vec2 position{1.0f, 2.0f};
```

`struct`は関連する値をまとめる型です。`position.x`でX成分へアクセスします。`float`は小数を表し、`1.0f`の末尾の`f`も`float`を示します。`{}`で初期化すると、ここではゼロから始まります。未初期化の値を計算に使わない習慣を付けます。

### 2.2 状態と操作を区別する

`enum class Action`は許される操作を名前付きで列挙する型です。数値だけで「0は停止、1は上」と書くより、誤った値を渡しにくくなります。

`class Environment`は、ゲームの状態と、それを更新する操作をまとめます。`public`は外から利用する部分、`private`はクラス自身が管理する部分です。描画側がプレイヤー座標を書き換えることを防ぐため、取得関数では`const`参照を返します。

```cpp
const State& state() const;
```

左の`const State&`は「コピーを作らず、変更できない参照を返す」という意味です。右の`const`は「この関数は環境の状態を変更しない」という約束です。参照先は環境が所有しているので、環境を破棄した後にその参照を使ってはいけません。

### 2.3 配列と寿命

数が固定された36個の入力には`std::array<float, 36>`、ステージによって数が違う壁には`std::vector<Rect>`を使います。どちらも自分のデータを所有し、寿命が終わると解放します。最初から`new`や`delete`を使う必要はありません。

描画の章では`std::unique_ptr`と`ComPtr`が登場します。前者は1つの所有者が動的オブジェクトを管理し、後者はDirect3Dが使うCOMオブジェクトの参照を管理します。資源の取得と解放をオブジェクトの寿命へ結び付ける考え方がRAIIです。

### 2.4 サンプルで繰り返し出てくる記法

| 記法 | 読み方と、このゲームでの用途 |
| --- | --- |
| `namespace sil { ... }` | 名前をまとめる領域。`sil::Action`のように所属を付ける |
| `auto` | 右辺から変数の型を推論する。値の内容が自動で決まる意味ではない |
| `const auto& wall` | 型を推論した、変更できない参照。壁のコピーを作らず読む |
| `for (const auto& wall : walls)` | 配列の各要素について処理を繰り返す |
| `a ? b : c` | aが真ならb、偽ならcを選ぶ。条件が複雑なら`if`へ書き換えて読む |
| `static_cast<int>(end)` | 列挙型などを明示的に別の型へ変換する |
| `throw std::runtime_error(...)` | 通常の処理を中断し、呼出元の`catch`へ失敗を伝える |
| `[&](...) { ... }` | 小さな名前なし関数。`[&]`は周囲の変数を参照して使う指定 |
| `std::move(stage)` | データの所有物を移せる形で渡す。移した後の中身を当てにしない |

`constexpr`は定数として扱える値や計算を定義する指定です。`static constexpr double dt`は、環境を何個作っても共通に使う固定値を表します。毎回の実時間をここへ書き込むものではありません。

関数の途中にある`return`は、その時点で呼出元へ戻ります。たとえば`blocked()`は、外へ出る条件を一つでも見つけると`true`を返し、それ以降の検査は行いません。

`#pragma once`は同じヘッダーを1つのコンパイル単位で重複して読み込むことを防ぎます。ヘッダーへ型の宣言、`.cpp`へ処理の定義を書くのが基本です。宣言だけあって定義をビルドへ含めていない場合は、コンパイルが通ってもリンク時に関数が見つからないエラーになります。

**小さな練習：** 後に出てくる`actionFromAxes()`の条件演算子を、`if`だけで同じ結果になるように書き直してください。9行動の対応表を先に紙へ書くと、実装を読まずに正しさを確認できます。

<a id="chapter-3"></a>

## 第3章 描画から独立したゲームを作る

### 3.1 データと公開インターフェース

球の中心を動かすだけなら、3D描画は不要です。床面のXとZだけを計算し、描画時に球の高さを付け加えます。衝突判定も床面上の円と長方形として扱います。

以下の完全例を第3章の到達点として使います。初めて書くときは、型の定義、`reset()`、移動、衝突、障害物、終了判定の順に読み、各機能を第4章のテストで確認してください。

**完全例：`src/core/Simulation.hpp`**

```cpp
#pragma once
#include <cstdint>
#include <vector>

namespace sil {
struct Vec2 { float x{}, z{}; };
struct Rect { Vec2 center{}, half{}; };
enum class Axis { X, Z };
enum class Action { Stop, Up, Down, Left, Right, UpRight, DownRight, UpLeft, DownLeft };
enum class End { Running, Success, Collision, Timeout };

struct Obstacle {
    Rect box;
    Axis axis{Axis::X};
    float low{}, high{}, speed{}, phase{};
};

struct Stage {
    Rect bounds;
    Vec2 start;
    float radius{0.3f};
    float speed{3.0f};
    int maxSteps{1200};
    std::vector<Rect> walls;
    std::vector<Obstacle> obstacles;
    Rect goal;
};

struct State {
    Vec2 player;
    std::vector<Rect> obstacles;
    int steps{};
    End end{End::Running};
};

struct StepResult { End end; float reward; };

Vec2 direction(Action action);
Action actionFromAxes(int x, int z);
bool touches(Vec2 center, float radius, Rect box);
bool overlaps(Rect a, Rect b);
Stage makeStage();

class Environment {
public:
    static constexpr double dt = 1.0 / 60.0;
    explicit Environment(Stage stage);
    void reset(std::uint32_t seed);
    StepResult step(Action action);
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
```

`Rect::half`は幅そのものではなく半分の長さです。中心Xが2、半幅が1なら、左端は1、右端は3です。円と長方形を扱うときに、中心から各辺までの距離を計算しやすくなります。

障害物は、`low`から`high`まで中心が移動します。`phase`は移動距離に加える初期位相で、0なら下限から開始します。`speed`は負にせず、折り返しによって方向が変わります。

### 3.2 移動量を時間から求める

速度が毎秒3なら、1/60秒で進む距離は`3 / 60 = 0.05`です。フレームごとに3進めると描画回数で速さが変わるため、必ず時間を掛けます。

斜めの方向`(1, 1)`の長さは√2です。長さで割れば、直進と同じ長さ1になります。相反するキーは、右を1、左を−1として足すと自然に打ち消せます。

### 3.3 円と長方形の接触

円の中心から長方形の最も近い点までの距離が、半径以下なら接触です。`std::clamp`で中心座標を長方形の範囲へ収めると、その最も近い点が求まります。平方根を使わず、距離の二乗と半径の二乗を比較できます。

壁は接しただけなら移動を許し、めり込みだけを拒否します。障害物とゴールは接触を含めます。この違いを同じ比較演算子で済ませないことが、壁沿いに動けるかどうかに関わります。

**完全例：`src/core/Simulation.cpp`**

```cpp
#include "core/Simulation.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace sil {
namespace {
float distanceSquared(Vec2 p, Rect b) {
    const float x = std::clamp(p.x, b.center.x - b.half.x, b.center.x + b.half.x);
    const float z = std::clamp(p.z, b.center.z - b.half.z, b.center.z + b.half.z);
    return (p.x - x) * (p.x - x) + (p.z - z) * (p.z - z);
}
}

Action actionFromAxes(int x, int z) {
    if (z > 0) return x > 0 ? Action::UpRight : x < 0 ? Action::UpLeft : Action::Up;
    if (z < 0) return x > 0 ? Action::DownRight : x < 0 ? Action::DownLeft : Action::Down;
    return x > 0 ? Action::Right : x < 0 ? Action::Left : Action::Stop;
}

Vec2 direction(Action action) {
    Vec2 d{};
    switch (action) {
    case Action::Stop: break;
    case Action::Up: d.z = 1; break;
    case Action::Down: d.z = -1; break;
    case Action::Left: d.x = -1; break;
    case Action::Right: d.x = 1; break;
    case Action::UpRight: d = {1, 1}; break;
    case Action::DownRight: d = {1, -1}; break;
    case Action::UpLeft: d = {-1, 1}; break;
    case Action::DownLeft: d = {-1, -1}; break;
    }
    const float length = std::sqrt(d.x * d.x + d.z * d.z);
    if (length > 0) { d.x /= length; d.z /= length; }
    return d;
}

bool touches(Vec2 p, float r, Rect b) { return distanceSquared(p, b) <= r * r; }
bool overlaps(Rect a, Rect b) {
    return std::abs(a.center.x - b.center.x) <= a.half.x + b.half.x
        && std::abs(a.center.z - b.center.z) <= a.half.z + b.half.z;
}

Stage makeStage() {
    Stage s;
    s.bounds = {{0, 5}, {5, 6}};
    s.start = {0, 0};
    s.goal = {{0, 10}, {4.5f, 0.25f}};
    s.walls = {{{-2.8f, 5.5f}, {0.3f, 1.0f}}};
    s.obstacles = {
        {{{0, 3}, {0.55f, 0.55f}}, Axis::X, -3.5f, 3.5f, 2.0f, 0.0f},
        {{{2, 6}, {0.4f, 0.8f}}, Axis::Z, 5.0f, 7.0f, 1.5f, 0.0f}
    };
    return s;
}

Environment::Environment(Stage stage) : stage_(std::move(stage)) {
    if (!(stage_.radius > 0) || !(stage_.speed >= 0) || stage_.maxSteps <= 0
        || !(stage_.bounds.half.x > stage_.radius) || !(stage_.bounds.half.z > stage_.radius)) {
        throw std::invalid_argument("invalid stage dimensions or timing");
    }
    for (const auto& o : stage_.obstacles) {
        if (!(o.high > o.low) || !(o.speed >= 0) || !(o.phase >= 0)
            || !(o.box.half.x > 0) || !(o.box.half.z > 0)) {
            throw std::invalid_argument("invalid obstacle");
        }
    }
    reset(0);
    if (blocked(state_.player)) throw std::invalid_argument("start is blocked");
}

void Environment::reset(std::uint32_t seed) {
    // このステージは乱数を使わない。将来ランダム化する場合も外部のseedで制御する。
    (void)seed;
    state_.player = stage_.start;
    state_.steps = 0;
    state_.end = End::Running;
    state_.obstacles.resize(stage_.obstacles.size());
    bestZ_ = stage_.start.z;
    placeObstacles(0);
}

void Environment::placeObstacles(double time) {
    for (std::size_t i = 0; i < stage_.obstacles.size(); ++i) {
        const auto& o = stage_.obstacles[i];
        const double length = o.high - o.low;
        const double u = std::fmod(o.phase + o.speed * time, 2 * length);
        const float value = o.low + static_cast<float>(u <= length ? u : 2 * length - u);
        state_.obstacles[i] = o.box;
        if (o.axis == Axis::X) state_.obstacles[i].center.x = value;
        else state_.obstacles[i].center.z = value;
    }
}

bool Environment::blocked(Vec2 p) const {
    const auto b = stage_.bounds;
    const float r = stage_.radius;
    if (p.x < b.center.x - b.half.x + r || p.x > b.center.x + b.half.x - r
        || p.z < b.center.z - b.half.z + r || p.z > b.center.z + b.half.z - r) return true;
    for (const auto& wall : stage_.walls) {
        if (distanceSquared(p, wall) < r * r) return true;
    }
    return false;
}

StepResult Environment::step(Action action) {
    if (state_.end != End::Running) throw std::logic_error("reset before stepping a finished episode");
    const Vec2 d = direction(action);
    bool hit = false;
    bool reached = false;
    // 教材用の低速ステージでは、1/60秒をさらに4回に分けて接触を確認する。
    constexpr int parts = 4;
    const float move = stage_.speed * static_cast<float>(dt / parts);
    for (int part = 1; part <= parts; ++part) {
        Vec2 next = state_.player;
        next.x += d.x * move;
        if (!blocked(next)) state_.player.x = next.x;
        next = state_.player;
        next.z += d.z * move;
        if (!blocked(next)) state_.player.z = next.z;
        const double time = (state_.steps + static_cast<double>(part) / parts) * dt;
        placeObstacles(time);
        reached = reached || touches(state_.player, stage_.radius, stage_.goal);
        for (const auto& box : state_.obstacles) hit = hit || touches(state_.player, stage_.radius, box);
    }
    ++state_.steps;
    if (reached) state_.end = End::Success;
    else if (hit) state_.end = End::Collision;
    else if (state_.steps >= stage_.maxSteps) state_.end = End::Timeout;

    const float progress = std::max(0.0f, state_.player.z - bestZ_);
    bestZ_ = std::max(bestZ_, state_.player.z);
    float reward = progress - 0.001f;
    if (state_.end == End::Success) reward += 10.0f;
    if (state_.end == End::Collision) reward -= 1.0f;
    return {state_.end, reward};
}
}
```

### 3.4 更新の順番を読む

`step()`では、移動、障害物の配置、接触の記録、終了判定、報酬の計算を順に行います。1/60秒の中を4回に分けていますが、外へ返す結果は1ステップにつき1個です。その4回で一度でも成功・衝突があればフラグへ残し、最後に成功を優先します。状態はステップ終端の位置であり、厳密な最初の接触時刻の位置ではありません。

往復運動は、前回位置へ速度を足す代わりに、経過時間から直接計算します。長さ`L`の道を往復すると1周期は移動距離`2L`です。`fmod`で余りを求め、前半なら進み、後半なら折り返します。リセットで時間をゼロにすると位置と位相も戻ります。

壁の処理は、進めなかった軸だけ戻します。斜めに壁へ向かうと、壁と平行な方向には進めます。ただしXとZを同時に厳密解として解く方法ではないため、角付近の振る舞いには軸の処理順の影響があります。

**限界を確認する：** 4分割は連続衝突判定ではありません。極端に薄い障害物や高速移動、角へのかすかな接触では見逃しが起こり得ます。この教材の設定は1分割の相対移動が小さくなるようにしていますが、任意の設定での非貫通は保証しません。速度や厚さを変更する際は最大速度・最小厚さでテストし、必要なら移動経路全体を調べる連続衝突判定へ進みます。外部ステージを読み込む段階では、有限値・全寸法・初期重なり・経路の範囲も検証してください。

報酬は、到達記録を更新した分から小さな時間費用を引き、成功・衝突時の値を加えた例です。上下往復するたびに前進報酬を再取得しないよう、`bestZ_`を保持しています。教材では短くするためここに置きますが、報酬項を増やすときは設定と計算を切り出し、移動や遺伝操作を変えずに調整できるようにします。

### 3.5 初めてのヘッドレス実行

**完全例：`src/app/HeadlessMain.cpp`**

```cpp
#include "core/Simulation.hpp"
#include <iostream>

int main() {
    sil::Environment env(sil::makeStage());
    env.reset(42);
    while (env.state().end == sil::End::Running) {
        env.step(sil::Action::Up);
    }
    std::cout << "steps=" << env.state().steps
              << " z=" << env.state().player.z
              << " end=" << static_cast<int>(env.state().end) << '\n';
}
```

この例は上へ進むだけです。描画も待機もないので、実際の制限時間より短い時間で終了できます。終了後に勝手に初期化しないため、終了理由と最後の位置を確認できます。

**確認課題：** `Action::Stop`へ変更すると、位置を保ったまま時間切れまで進みます。`maxSteps = 1200`なら1200回目の更新で時間切れです。停止するプレイヤーに障害物が届くステージでは、その前に衝突する点にも注意してください。

<a id="chapter-4"></a>

## 第4章 テストでルールを確かめる

画面で「止まったように見える」だけでは、壁を少し突き抜けていないか分かりません。小さな条件を数値で確認すると、描画の問題とゲームルールの問題を分けて調べられます。

次の例では外部のテストライブラリを増やさず、条件を満たさないと例外で失敗します。`assert()`はビルド設定によって無効になるため、ここでは独自の`check()`を使います。

**完全例：`tests/SimulationTests.cpp`**

```cpp
#include "core/Simulation.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace {
void check(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}
bool near(float a, float b) { return std::abs(a - b) < 0.0001f; }
sil::Stage emptyStage() {
    auto s = sil::makeStage();
    s.walls.clear();
    s.obstacles.clear();
    return s;
}
}
int main() {
    try {
        sil::Environment up(emptyStage()), diagonal(emptyStage());
        up.step(sil::Action::Up);
        diagonal.step(sil::Action::UpRight);
        const auto p = diagonal.state().player;
        check(near(up.state().player.z, 0.05f), "speed times dt");
        check(near(std::sqrt(p.x * p.x + p.z * p.z), 0.05f), "diagonal speed");
        check(sil::actionFromAxes(0, 0) == sil::Action::Stop, "cancelled keys");
        check(sil::touches({0, 0}, 1, {{2, 0}, {1, 1}}), "touch includes tangent");
        check(!sil::touches({0, 0}, 0.9f, {{2, 0}, {1, 1}}), "separated circle");

        auto s = emptyStage();
        s.walls = {{{0, 1}, {1, 0.2f}}};
        sil::Environment wall(s);
        for (int i = 0; i < 30; ++i) wall.step(sil::Action::Up);
        check(wall.state().player.z <= 0.5f + 0.0001f, "wall blocks movement");

        s = emptyStage();
        s.maxSteps = 1;
        sil::Environment timeout(s);
        check(timeout.step(sil::Action::Stop).end == sil::End::Timeout, "timeout boundary");
        s.goal = {{0, 0}, {1, 1}};
        s.obstacles = {{{{0, 0}, {1, 1}}, sil::Axis::X, 0, 1, 0, 0}};
        sil::Environment priority(s);
        check(priority.step(sil::Action::Stop).end == sil::End::Success, "success wins all ties");
        s.goal = {{0, 10}, {1, 0.1f}};
        sil::Environment collision(s);
        check(collision.step(sil::Action::Stop).end == sil::End::Collision, "collision before timeout");

        sil::Environment a(sil::makeStage()), b(sil::makeStage());
        a.reset(42); b.reset(42);
        for (int i = 0; i < 100; ++i) {
            a.step(sil::Action::Stop); b.step(sil::Action::Stop);
            check(a.state().player.z == b.state().player.z, "same player state");
            check(a.state().obstacles[0].center.x == b.state().obstacles[0].center.x, "same obstacle state");
        }
        a.reset(42);
        check(a.state().steps == 0 && a.state().end == sil::End::Running, "reset terminal state");
        check(near(a.state().obstacles[0].center.x, -3.5f), "reset phase");
        auto turnStage = emptyStage();
        turnStage.obstacles = {{{{0, 5}, {0.5f, 0.5f}}, sil::Axis::X, 0, 1, 1, 0}};
        sil::Environment turn(turnStage);
        for (int i = 0; i < 90; ++i) turn.step(sil::Action::Stop);
        check(near(turn.state().obstacles[0].center.x, 0.5f), "reflection keeps overshoot");
        std::cout << "simulation tests passed\n";
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
```

まずこの例をCTestから実行します。失敗したら、表示された条件だけに絞って原因を調べます。確認課題として、上下左右のフィールド境界、壁の角、終了後の`step()`、ゴール手前での非接触も追加してください。テストの期待値を実装の計算式からそのまま作るより、仕様から独立に求めることが大切です。


<a id="chapter-5"></a>

## 第5章 WindowsとDirect3D 11

この章では、第4章までで作った描画なしの `sil::Environment` を Windows の
ウィンドウに接続する。ここで新しく作るコードは、ゲーム規則を決めない。
`Environment::step()` がゲームを更新し、Direct3D 11 側は更新済みの状態を
**読むだけ**で描画する。この境界を守れば、同じ `Environment` を後から
ヘッドレス学習にも使える。

この章の完成時には、球体を WASD で動かし、床、壁、往復する障害物、ゴールを
固定カメラで表示できる。終了した試行は結果を記録してすぐ `reset()` する。

> **対象環境**: Windows 11 x64、Visual Studio の MSVC、C++20、Direct3D 11、
> DirectX Tool Kit (DirectXTK) を想定する。macOS ではこの章のアプリはビルド
> しない。Windows 実機での動作確認は、実装した本人が行う必要がある。

### 5.1 共通ゲームを読み取る

第3章の`core/Simulation.hpp`をそのまま使います。ここへDirect3DやWin32のヘッダーを追加しません。`env.stage()`で配置と寸法、`env.state()`で現在位置を読み、`env.step(action)`で更新します。

`Vec2`の成分はXとZです。`Action::Up`を+Zとして扱い、球の高さYは描画側だけで付けます。この約束を入力・シミュレーション・描画で共有します。

#### ここで確認すること

まずは第4章のヘッドレステストを実行する。`Environment` を含むターゲットが
Windows のウィンドウや DirectX ライブラリなしでビルドできなければ、先に依存
関係を直す。描画を足してからでは、原因を分けにくくなる。

### 5.2 描画担当を小さなクラスに分ける

5.3〜5.5節のコードをビルドする前に、5.6節のDirectXTK導入とCMake追記を済ませます。GUIはこの段階では不要です。

Win32 のメッセージ処理、入力、固定時間更新を `WindowsMain.cpp` に置く。
Direct3D のデバイス、バックバッファ、深度バッファ、図形を描く処理は
`Renderer` に閉じ込める。`Renderer` は `Environment` を所有せず、状態を変更も
しない。

作成するファイルは次の3つである。以下の「完全例」は、そのまま各ファイルへ
転記するためのコードである。

```text
src/
├── app/
│   └── WindowsMain.cpp
└── renderer/
    └── d3d11/
        ├── Renderer.hpp
        └── Renderer.cpp
```

### 5.3 完全例: `src/renderer/d3d11/Renderer.hpp`

```cpp
#pragma once

#include <cstdint>
#include <memory>

#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <GeometricPrimitive.h>

namespace sil {
struct Stage;
struct State;
}

namespace sil::renderer {

// Direct3D 11 の資源を所有し、Stage と State を読むだけで3D表示するクラス。
class Renderer final {
public:
    Renderer(HWND window, std::uint32_t width, std::uint32_t height);
    ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    // WM_SIZE を受けたメインループから呼ぶ。幅または高さが0なら何もしない。
    void resize(std::uint32_t width, std::uint32_t height);

    // この関数はシミュレーションを書き換えない。Present は呼ばない。
    void render(const Stage& stage, const State& state);
    void present();

    // 第5.7節で Dear ImGui の DirectX 11 バックエンドを初期化するときに使う。
    ID3D11Device* device() const noexcept { return device_.Get(); }
    ID3D11DeviceContext* context() const noexcept { return context_.Get(); }

private:
    void createDeviceAndSwapChain(HWND window);
    void createSizeDependentResources(std::uint32_t width, std::uint32_t height);

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthTexture_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView_;

    D3D11_VIEWPORT viewport_{};
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;

    std::unique_ptr<DirectX::GeometricPrimitive> cube_;
    std::unique_ptr<DirectX::GeometricPrimitive> sphere_;
};

} // namespace sil::renderer
```

`ComPtr` は COM オブジェクトの参照カウントを管理するスマートポインタである。
`ID3D11Device` などを生ポインタで保持して `Release()` を手で呼ぶ必要がない。
`GeometricPrimitive` は DirectXTK が用意する、立方体や球を描くためのクラスである。
このプロジェクトでは初期段階で複雑なモデルを使わないため、目的にちょうどよい。

`render()` と `present()` を分けた点にも理由がある。第5.7節で ImGui を追加する際、
3Dを描いた後、画面を交換する前に ImGui を描く必要があるからである。

### 5.4 完全例: `src/renderer/d3d11/Renderer.cpp`

```cpp
#include "renderer/d3d11/Renderer.hpp"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include <DirectXColors.h>
#include <DirectXMath.h>
#include "core/Simulation.hpp"

namespace {

void throwIfFailed(HRESULT result, const char* operation) {
    if (SUCCEEDED(result)) {
        return;
    }

    std::ostringstream message;
    message << operation << " failed (HRESULT=0x"
            << std::hex << std::uppercase << static_cast<unsigned long>(result) << ')';
    throw std::runtime_error(message.str());
}

// DirectXTK の単位立方体を、XZ平面上の Rect と指定した高さへ変換して描く。
void drawRect(
    DirectX::GeometricPrimitive& cube,
    const sil::Rect& rect,
    float centerY,
    float height,
    DirectX::FXMVECTOR color,
    DirectX::FXMMATRIX view,
    DirectX::FXMMATRIX projection) {
    using namespace DirectX;

    const XMMATRIX world =
        XMMatrixScaling(rect.half.x * 2.0F, height, rect.half.z * 2.0F) *
        XMMatrixTranslation(rect.center.x, centerY, rect.center.z);
    cube.Draw(world, view, projection, color);
}

} // namespace

namespace sil::renderer {

Renderer::Renderer(HWND window, std::uint32_t width, std::uint32_t height) {
    createDeviceAndSwapChain(window);

    // DirectXTK の図形生成はデバイスコンテキストを受け取る。false は左手系座標。
    cube_ = DirectX::GeometricPrimitive::CreateCube(context_.Get(), 1.0F, false);
    sphere_ = DirectX::GeometricPrimitive::CreateSphere(context_.Get(), 1.0F, 24, false);

    resize(width, height);
}

void Renderer::createDeviceAndSwapChain(HWND window) {
    DXGI_SWAP_CHAIN_DESC swapChainDescription{};
    swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDescription.SampleDesc.Count = 1;
    swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDescription.BufferCount = 1;
    swapChainDescription.OutputWindow = window;
    swapChainDescription.Windowed = TRUE;
    swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    // Visual Studio の Graphics Diagnostics / D3D11 debug layer を使うための指定。
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    constexpr D3D_FEATURE_LEVEL requestedLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };
    D3D_FEATURE_LEVEL actualLevel{};

    throwIfFailed(
        D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            requestedLevels,
            static_cast<UINT>(std::size(requestedLevels)),
            D3D11_SDK_VERSION,
            &swapChainDescription,
            swapChain_.GetAddressOf(),
            device_.GetAddressOf(),
            &actualLevel,
            context_.GetAddressOf()),
        "D3D11CreateDeviceAndSwapChain");
}

void Renderer::resize(std::uint32_t width, std::uint32_t height) {
    if (width == 0 || height == 0) {
        return;
    }

    // 初回も ResizeBuffers 後も、描画先と深度バッファを作り直す。
    if (width_ != 0 && height_ != 0) {
        context_->OMSetRenderTargets(0, nullptr, nullptr);
        renderTargetView_.Reset();
        depthStencilView_.Reset();
        depthTexture_.Reset();

        throwIfFailed(
            swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0),
            "IDXGISwapChain::ResizeBuffers");
    }

    createSizeDependentResources(width, height);
    width_ = width;
    height_ = height;
}

void Renderer::createSizeDependentResources(std::uint32_t width, std::uint32_t height) {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    throwIfFailed(
        swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())),
        "IDXGISwapChain::GetBuffer");
    throwIfFailed(
        device_->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView_.GetAddressOf()),
        "ID3D11Device::CreateRenderTargetView");

    D3D11_TEXTURE2D_DESC depthDescription{};
    depthDescription.Width = width;
    depthDescription.Height = height;
    depthDescription.MipLevels = 1;
    depthDescription.ArraySize = 1;
    depthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDescription.SampleDesc.Count = 1;
    depthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    throwIfFailed(
        device_->CreateTexture2D(&depthDescription, nullptr, depthTexture_.GetAddressOf()),
        "ID3D11Device::CreateTexture2D for depth");
    throwIfFailed(
        device_->CreateDepthStencilView(
            depthTexture_.Get(), nullptr, depthStencilView_.GetAddressOf()),
        "ID3D11Device::CreateDepthStencilView");

    viewport_.TopLeftX = 0.0F;
    viewport_.TopLeftY = 0.0F;
    viewport_.Width = static_cast<float>(width);
    viewport_.Height = static_cast<float>(height);
    viewport_.MinDepth = 0.0F;
    viewport_.MaxDepth = 1.0F;
}

void Renderer::render(const Stage& stage, const State& state) {
    if (width_ == 0 || height_ == 0) {
        return;
    }

    const float clearColor[] = {0.06F, 0.08F, 0.12F, 1.0F};
    context_->OMSetRenderTargets(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());
    context_->RSSetViewports(1, &viewport_);
    context_->ClearRenderTargetView(renderTargetView_.Get(), clearColor);
    context_->ClearDepthStencilView(
        depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0F, 0);

    using namespace DirectX;
    const float aspect = static_cast<float>(width_) / static_cast<float>(height_);

    // 左手系で -Z 側から原点を見る。画面の右はワールドの +X、上は +Z になる。
    const XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(0.0F, 16.0F, -18.0F, 0.0F),
        XMVectorSet(0.0F, 0.0F, 0.0F, 0.0F),
        XMVectorSet(0.0F, 1.0F, 0.0F, 0.0F));
    const XMMATRIX projection = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(50.0F), aspect, 0.1F, 100.0F);

    constexpr float floorThickness = 0.20F;
    constexpr float wallHeight = 1.00F;
    constexpr float obstacleHeight = 0.75F;

    // 床、ゴール、壁、障害物は単位立方体を拡大して描く。
    drawRect(*cube_, stage.bounds, -floorThickness * 0.5F, floorThickness,
             Colors::DarkSlateGray, view, projection);
    drawRect(*cube_, stage.goal, 0.025F, 0.05F, Colors::LimeGreen, view, projection);

    for (const Rect& wall : stage.walls) {
        drawRect(*cube_, wall, wallHeight * 0.5F, wallHeight, Colors::SlateGray, view, projection);
    }
    for (const Rect& obstacle : state.obstacles) {
        drawRect(*cube_, obstacle, obstacleHeight * 0.5F, obstacleHeight,
                 Colors::Crimson, view, projection);
    }

    // CreateSphere の直径は1なので、半径の2倍で拡大する。
    const XMMATRIX playerWorld =
        XMMatrixScaling(stage.radius * 2.0F, stage.radius * 2.0F, stage.radius * 2.0F) *
        XMMatrixTranslation(state.player.x, stage.radius, state.player.z);
    sphere_->Draw(playerWorld, view, projection, Colors::CornflowerBlue);
}

void Renderer::present() {
    // 1 は垂直同期を待つ指定。学習用のヘッドレス実行はこの関数を呼ばない。
    throwIfFailed(swapChain_->Present(1, 0), "IDXGISwapChain::Present");
}

} // namespace sil::renderer
```

#### コードを読む順番

1. `createDeviceAndSwapChain()` は GPU へ命令を出すための `device_` と
   `context_`、表示前後の画像を交換する `swapChain_` を作る。
2. `resize()` はウィンドウサイズに対応する描画先と深度バッファを破棄して、
   作り直す。深度バッファがないと、奥にある物体が手前の物体を覆う表示になる。
3. `render()` は背景を消去し、カメラの `view`、レンズの `projection`、各物体の
   `world` を渡して図形を描く。
4. `present()` が完成した絵を画面に出す。

`throwIfFailed` は HRESULT が失敗を表すときに例外を送出する。失敗を無視すると、
後の行で null ポインタを使ったような読みにくい停止になりやすい。デバッグ構成で
`D3D11_CREATE_DEVICE_DEBUG` に失敗する場合は、Windows の「設定」→「システム」→
「オプション機能」→「機能を表示」から Graphics Toolsを追加する。まず起動を
優先するなら、そのフラグを一時的に外してもよいが、描画が始まったら有効に戻す。

この最小版はデバイスロストの復旧を実装していない。`Present` が
`DXGI_ERROR_DEVICE_REMOVED` などで失敗した場合は例外で終了する。初期段階では
エラーを見落とさないことを優先し、復旧処理は必要になってから追加する。

#### 実行確認とよくある失敗

球体が壁や床の裏に隠れる、または画面全体が一色だけになる場合は、次を順に確認
する。

- `render()` 内で `ClearDepthStencilView` を呼び、`OMSetRenderTargets` に深度
  ビューを渡しているか。
- `XMMatrixLookAtLH` と `XMMatrixPerspectiveFovLH` を組にしているか。右手系の
  関数を片方だけ混ぜると、カメラの前後が逆になる。
- `state.player` を `(x, radius, z)` へ変換しているか。Y=0 に置くと球の半分が床
  の下に入る。
- `WM_SIZE` の最中にはまだ `resize()` を呼ばず、メインループで一度だけ呼んで
  いるか。ドラッグでサイズ変更中に大量の資源再作成を防げる。

### 5.5 完全例: `src/app/WindowsMain.cpp`

このファイルが Win32 の入口である。固定更新の間隔は、描画のフレーム時間では
なく `Environment::dt` にする。高性能なPCでも低性能なPCでも、1秒あたりに進む
シミュレーションの回数を同じにできる。

```cpp
#include <windows.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>

#include "core/Simulation.hpp"
#include "renderer/d3d11/Renderer.hpp"

namespace {

struct WindowState {
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    bool resizePending = false;
    bool minimized = false;
    bool hasFocus = true;
    bool resetClock = false;
};

bool isKeyDown(int virtualKey) {
    return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
}

sil::Action readAction() {
    // 相反するキーは軸ごとに打ち消す。W+S は vertical==0 になる。
    const int horizontal = (isKeyDown('D') ? 1 : 0) - (isKeyDown('A') ? 1 : 0);
    const int vertical = (isKeyDown('W') ? 1 : 0) - (isKeyDown('S') ? 1 : 0);

    if (horizontal == 0 && vertical == 0) return sil::Action::Stop;
    if (horizontal == 0 && vertical > 0) return sil::Action::Up;
    if (horizontal == 0 && vertical < 0) return sil::Action::Down;
    if (horizontal > 0 && vertical == 0) return sil::Action::Right;
    if (horizontal < 0 && vertical == 0) return sil::Action::Left;
    if (horizontal > 0 && vertical > 0) return sil::Action::UpRight;
    if (horizontal > 0 && vertical < 0) return sil::Action::DownRight;
    if (horizontal < 0 && vertical > 0) return sil::Action::UpLeft;
    return sil::Action::DownLeft;
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    auto* state = reinterpret_cast<WindowState*>(GetWindowLongPtr(window, GWLP_USERDATA));

    switch (message) {
    case WM_NCCREATE: {
        const auto* create = reinterpret_cast<const CREATESTRUCT*>(lParam);
        SetWindowLongPtr(
            window, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(create->lpCreateParams));
        return TRUE;
    }
    case WM_SIZE:
        if (state != nullptr) {
            state->minimized = (wParam == SIZE_MINIMIZED);
            state->width = static_cast<std::uint32_t>(LOWORD(lParam));
            state->height = static_cast<std::uint32_t>(HIWORD(lParam));
            state->resizePending = !state->minimized;
            state->resetClock = true;
        }
        return 0;
    case WM_ACTIVATEAPP:
        if (state != nullptr) {
            state->hasFocus = (wParam != FALSE);
            state->resetClock = true;
        }
        return 0;
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(window, message, wParam, lParam);
    }
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand) {
    constexpr wchar_t windowClassName[] = L"SphereIsLearningWindow";

    WNDCLASSEX windowClass{};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.hInstance = instance;
    windowClass.lpszClassName = windowClassName;
    windowClass.lpfnWndProc = windowProcedure;
    windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    windowClass.hbrBackground = nullptr; // 背景は Direct3D が毎フレーム描く。

    if (RegisterClassEx(&windowClass) == 0) {
        return 1;
    }

    WindowState windowState{};
    const HWND window = CreateWindowEx(
        0,
        windowClassName,
        L"sphere is learning",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        static_cast<int>(windowState.width), static_cast<int>(windowState.height),
        nullptr, nullptr, instance, &windowState);
    if (window == nullptr) {
        return 1;
    }

    ShowWindow(window, showCommand);
    UpdateWindow(window);

    RECT clientRect{};
    GetClientRect(window, &clientRect);
    windowState.width = static_cast<std::uint32_t>(clientRect.right - clientRect.left);
    windowState.height = static_cast<std::uint32_t>(clientRect.bottom - clientRect.top);
    windowState.resizePending = false;

    try {
        sil::renderer::Renderer renderer(window, windowState.width, windowState.height);
        sil::Environment env{sil::makeStage()};
        constexpr std::uint32_t initialSeed = 0;
        env.reset(initialSeed);

        sil::End lastEnd = sil::End::Running;
        double accumulator = 0.0;
        auto previousTime = std::chrono::steady_clock::now();
        bool running = true;

        while (running) {
            MSG message{};
            while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
                if (message.message == WM_QUIT) {
                    running = false;
                }
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
            if (!running) {
                break;
            }

            const auto now = std::chrono::steady_clock::now();
            if (windowState.resetClock) {
                // サイズ変更、最小化、別ウィンドウへの移動中の時間を進めない。
                previousTime = now;
                accumulator = 0.0;
                windowState.resetClock = false;
            }

            if (windowState.resizePending) {
                renderer.resize(windowState.width, windowState.height);
                windowState.resizePending = false;
            }

            if (windowState.minimized || !windowState.hasFocus) {
                previousTime = now;
                accumulator = 0.0;
                Sleep(10);
                continue;
            }

            const double elapsed = std::min(
                std::chrono::duration<double>(now - previousTime).count(), 0.25);
            previousTime = now;
            accumulator += elapsed;

            while (accumulator >= sil::Environment::dt) {
                const sil::StepResult result = env.step(readAction());
                if (result.end != sil::End::Running) {
                    // 終了理由をこの時点で保存してから、待たずに次の試行を始める。
                    lastEnd = result.end;
                    env.reset(initialSeed);
                }
                accumulator -= sil::Environment::dt;
            }

            // stage() と state() は const 参照。Renderer は描画だけを行う。
            renderer.render(env.stage(), env.state());
            renderer.present();

            // 現時点では lastEnd をデバッガで確認できる。第5.7節で画面表示へ使う。
            static_cast<void>(lastEnd);
        }
    } catch (const std::exception& error) {
        MessageBoxA(window, error.what(), "sphere is learning: error", MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}
```

#### なぜ `GetAsyncKeyState` を使うのか

`WM_KEYDOWN` は「キーを押した出来事」を通知する。一方、移動には「いま押されて
いるか」を各シミュレーションステップで調べる方が自然である。`GetAsyncKeyState`
で W/A/S/D の現在の押下状態を読み、二つの軸の符号から9行動へ変換する。

斜め移動の長さを 1 にそろえる処理は、入力側ではなく `Environment::step()` 側で
行う。AI は `Action::UpRight` を直接渡すため、キーボード操作だけに適用される
補正にしてはいけない。

#### 固定時間更新を確認する

`accumulator` は、実時間の端数をためる箱である。描画が速いフレームでは 0 回、
遅いフレームでは複数回 `step()` を呼ぶ。これにより、描画が 60 FPS でなくても
ゲーム規則は 1/60 秒刻みで進む。`elapsed` を 0.25 秒で制限するのは、デバッガで
止めた後などに数千回の更新を一度に実行しないためである。

最小化中またはフォーカスを失った間は更新しない。復帰後に経過時間をまとめて
加算すると、触っていないのに時間切れになるからである。

#### この段階の実行確認

Windows の Developer PowerShell で、プロジェクト根から次を実行する。Visual Studio
のx64環境を使うので、`-A x64` を明示する。

```powershell
cmake -S . -B build -A x64
cmake --build build --config Debug
.\build\Debug\sphere_game.exe
```

確認する項目は以下である。

- 黒に近い青の背景に、床、灰色の壁、赤い障害物、緑のゴール、青い球が出る。
- W はワールドの `+Z` 方向、S は `-Z` 方向へ、D は画面の右と同じ `+X` 方向へ
  動く。これは左手系のカメラと `rhcoords = false` の図形生成を統一した結果である。
- ウィンドウを最小化して戻しても、復帰直後に時間切れにならない。
- サイズを変えても絵が引き伸ばされず、奥行きの前後関係が保たれる。
- 障害物またはゴールに触れた直後、新しい試行がスタート位置から始まる。

先に5.6節の依存ライブラリとCMakeの設定を済ませてから実行する。この教材のコマンドでは Visual Studio ジェネレーターを用いるため、実行ファイルは
`build\Debug\sphere_game.exe` になる。別のジェネレーターへ変えた場合だけ、
ビルドログで出力先を確認する。

### 5.6 CMake との接続

CMakeの基本構成は第1章で作ったものを使う。まず Windows PC で、プロジェクト根から
DirectXTK を Git submodule として追加する。一度追加してコミットすれば、他のPC
では clone 時に `--recurse-submodules` を付けるか、後から
`git submodule update --init --recursive` を実行するだけで同じ版を取得できる。

```powershell
git submodule add https://github.com/microsoft/DirectXTK.git external/DirectXTK
git submodule update --init --recursive
git status
```

次はルートの `CMakeLists.txt` の、既存の `sphere_core` 定義の**後**に加える完全な
Windows 部分である。`DirectXTK` を `add_subdirectory` で組み込む場合の実ターゲット
名は `DirectXTK` である。`Microsoft::DirectXTK` は、パッケージを別途 install して
`find_package` した場合の名前なので、この構成では使わない。シミュレーション
ライブラリへは DirectXTK をリンクしない。補助ツールと初期段階で使わない音声部分は無効にする。

```cmake
if(WIN32)
    set(BUILD_TOOLS OFF CACHE BOOL "Build DirectXTK auxiliary tools" FORCE)
    set(BUILD_XAUDIO_WIN8 OFF CACHE BOOL "Build DirectXTK audio" FORCE)
    add_subdirectory(external/DirectXTK)

    add_executable(sphere_game WIN32
        src/app/WindowsMain.cpp
        src/renderer/d3d11/Renderer.cpp
        src/renderer/d3d11/Renderer.hpp
    )

    target_include_directories(sphere_game PRIVATE src)
    target_compile_definitions(sphere_game PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
    )
    target_compile_options(sphere_game PRIVATE /W4 /utf-8)
    target_link_libraries(sphere_game PRIVATE
        sphere_core
        DirectXTK
        user32
        d3d11
        dxgi
    )
endif()
```

外側の `if(WIN32)` により、macOS は `external/DirectXTK` と Windowsアプリを構成
しない。`WIN32` は `add_executable` では、コンソールではない Win32 アプリとして
`wWinMain` を入口にする指定である。`NOMINMAX` は Windows ヘッダーの `min` / `max`
マクロが `std::min` を壊すのを防ぐ。`UNICODE` と `_UNICODE` は Win32 API を
ワイド文字版にそろえ、`/utf-8` は日本語コメントを含むソースの文字コードを明示
する。`/W4` は警告を見逃さないための水準である。

### 5.7 Dear ImGui をあとから追加する

最初の表示確認では ImGui を入れない。ウィンドウ、固定更新、3D描画の問題と GUI
の問題を分けるためである。ここまで動いた後に、ステージ選択、残り時間、直前の
終了理由を表示するための ImGui を追加する。

以下は前節の**完全例に差し込む断片**であり、ImGui を導入した時点で適用する。
既に動く第5章のコードへ、手順順に加える。

#### 1. Dear ImGui を置き、CMake でバックエンドもコンパイルする

まずプロジェクト根で、ImGui を submodule として追加する。DirectXTK と同様に、
他のPCでは clone 時に `--recurse-submodules` を付けるか、後から
`git submodule update --init --recursive` を実行する。ここで追加した submodule の
参照は、ほかの人にも同じ ImGui の版を渡すためにコミットする。

```powershell
git submodule add https://github.com/ocornut/imgui.git external/imgui
git submodule update --init --recursive
git status
```

ImGui 本体に加えて、Win32用と DirectX 11用の backend の `.cpp` も自分の
ターゲットでコンパイルする。ヘッダーを include するだけでは描画できない。
次の CMake は、第5.6節の `if(WIN32)` ブロックの中で、`sphere_game` を定義した
**後**に加える。

```cmake
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")

add_library(sil_imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_win32.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_dx11.cpp"
)
target_include_directories(sil_imgui PUBLIC "${IMGUI_DIR}")
target_link_libraries(sil_imgui PRIVATE d3d11)

target_link_libraries(sphere_game PRIVATE sil_imgui)
target_compile_definitions(sphere_game PRIVATE SIL_WITH_IMGUI=1)
```

ImGui は企画書で選定済みのデバッグ GUI である。ここではゲーム規則にも描画基盤の
設計にも混ぜず、Windowsアプリだけに依存を限定する。

#### 2. `WindowState` とウィンドウプロシージャを変更する

`WindowsMain.cpp` の include 群へ追加する。

```cpp
#if defined(SIL_WITH_IMGUI)
#include <imgui.h>
#include <backends/imgui_impl_dx11.h>
#include <backends/imgui_impl_win32.h>

// imgui_impl_win32.h では利用側が宣言するよう #if 0 内に置かれている関数。
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND window, UINT message, WPARAM wParam, LPARAM lParam);
#endif
```

`WindowState` に次の1行を追加する。

```cpp
bool imguiReady = false;
```

`windowProcedure` の先頭、`GetWindowLongPtr` の直後に次を加える。

```cpp
#if defined(SIL_WITH_IMGUI)
if (state != nullptr && state->imguiReady &&
    ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam)) {
    return 1;
}
#endif
```

`imguiReady` は、`CreateWindowEx` の途中に来るメッセージを ImGui へ送らないための
印である。ImGui の初期化後だけバックエンドに渡す。

#### 3. 初期化と終了処理を加える

`Renderer renderer(...)` と `Environment env(...)` を作った直後に、次を
挿入する。

```cpp
#if defined(SIL_WITH_IMGUI)
IMGUI_CHECKVERSION();
ImGui::CreateContext();
ImGui::StyleColorsDark();
ImGui_ImplWin32_Init(window);
ImGui_ImplDX11_Init(renderer.device(), renderer.context());
windowState.imguiReady = true;
#endif
```

`try` ブロックを出る直前、`Renderer` が破棄される前に終了処理を置く。

```cpp
#if defined(SIL_WITH_IMGUI)
windowState.imguiReady = false;
ImGui_ImplDX11_Shutdown();
ImGui_ImplWin32_Shutdown();
ImGui::DestroyContext();
#endif
```

例外時にも終了処理を保証するには、小さなRAIIラッパーに初期化と終了処理をまとめる方法がある。
ただし最初は通常の終了経路で初期化順と終了順を理解することを優先する。

#### 4. メインループへフレーム開始、ステージ選択、描画を入れる

`bool running = true;` の直前に、次の二つを追加する。`playing` が false の間は
環境を更新しないので、起動直後にステージが走り始めない。現在は1ステージだけだが、
`selectedStage` を置くことで、後に選択肢を増やす位置が明確になる。

```cpp
int selectedStage = 0;
bool playing = false;
```

ImGui のフレーム開始とウィンドウの生成は、最小化・フォーカス喪失時の
`Sleep(10); continue;` の**後**、固定更新の前に置く。途中で `continue` するときに
`NewFrame()` を呼んでいなければ、開始したまま確定しない ImGui フレームは生じない。

```cpp
#if defined(SIL_WITH_IMGUI)
ImGui_ImplDX11_NewFrame();
ImGui_ImplWin32_NewFrame();
ImGui::NewFrame();

ImGui::Begin("Stage selection");
ImGui::TextUnformatted("Choose a demo stage before starting.");
ImGui::RadioButton("Stage 1: basic obstacle course", &selectedStage, 0);

if (ImGui::Button(playing ? "Return to selection" : "Start selected stage")) {
    playing = !playing;
    if (playing) {
        // 現在は選択肢が Stage 1 のみなので makeStage() を使う。
        // Stage 2 を足すときは selectedStage に応じて別の Stage を渡す。
        env.reset(initialSeed);
        lastEnd = sil::End::Running;
    }
    accumulator = 0.0;
}
const double remainingSeconds = std::max(
    0.0,
    (static_cast<double>(env.stage().maxSteps - env.state().steps) * sil::Environment::dt));
ImGui::Text("State: %s", playing ? "Playing" : "Stopped");
ImGui::Text("Remaining: %.1f s", remainingSeconds);
ImGui::Text("Last result: %s",
    lastEnd == sil::End::Success ? "Success" :
    lastEnd == sil::End::Collision ? "Collision" :
    lastEnd == sil::End::Timeout ? "Timeout" : "Running");
ImGui::End();
#endif
```

固定更新の`while (accumulator >= sil::Environment::dt)`内にある、`const sil::StepResult result = env.step(readAction());`からリセットの`if`ブロック末尾までを、次のコード全体へ置き換える。`accumulator`を減らす行はその後へ残す。`playing` が true のとき
だけゲームを進める。終了時も `playing` を変えないため、終了理由を記録してから
すぐ `reset()` され、次の試行が自動で始まる。ImGui がキーボードを必要としている
間は停止行動を渡す。

```cpp
if (playing) {
    sil::Action action = readAction();
#if defined(SIL_WITH_IMGUI)
    if (ImGui::GetIO().WantCaptureKeyboard) {
        action = sil::Action::Stop;
    }
#endif
    const sil::StepResult result = env.step(action);
    if (result.end != sil::End::Running) {
        lastEnd = result.end;
        env.reset(initialSeed);
    }
}
```

最後に、`renderer.render(...)` と `renderer.present()` の間へ、ImGui の確定済み
描画データを挿入する。

```cpp
renderer.render(env.stage(), env.state());
#if defined(SIL_WITH_IMGUI)
ImGui::Render();
ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif
renderer.present();
```

`WantCaptureKeyboard` は、テキスト入力やショートカットなどを ImGui が受け取り
たいかを表す。これを確認しないと、将来テキストボックスへ W を入力しただけで
プレイヤーも動く。ImGui が描画するのは `Renderer::render()` の後、`present()` の
前でなければならない。`present()` の後では、次のフレームまで画面に出ない。

#### ImGui を追加した後の確認

- 黒い「Stage selection」ウィンドウが3D画面の上に表示される。
- 起動直後は停止しており、`Stage 1: basic obstacle course` を選んでから
  `Start selected stage` を押すと動き始める。
- 残り秒数と、直前の終了理由が表示される。
- ImGui の操作中にゲームへの WASD 入力が流れない。
- 3D画面と GUI が同じフレームに表示される。

### 5.8 公式資料

DirectXTK の [Getting Started](https://github.com/microsoft/DirectXTK/wiki/Getting-Started)
は、Direct3D 11 と DirectXTK を C++ で使うための公式チュートリアルである。
`GeometricPrimitive` を含むサンプルと、デバッグ時の例外確認の考え方を確認できる。
古い DirectX SDK は使わず、Visual Studio と Windows SDK を使う。

Dear ImGui の [公式リポジトリ](https://github.com/ocornut/imgui) と
[バックエンド一覧](https://github.com/ocornut/imgui/tree/master/backends) では、
`imgui_impl_win32.cpp` と `imgui_impl_dx11.cpp` が Win32 / DirectX 11 の組で
あることを確認できる。アップデート時には、ImGui 本体と両バックエンドを同じ
リビジョンでそろえる。

### 5.9 この章の完了条件

- Windows x64で `sphere_game` がビルド・起動する。
- 描画コードは `src/renderer/d3d11` にあり、`src/core` は DirectX を参照しない。
- `Renderer::render` は `const Stage&` と `const State&` だけを読み、`step` や
  `reset` を呼ばない。
- 更新間隔は `Environment::dt` で固定され、描画フレームレートに依存しない。
- WASD が9行動に変換され、相反する方向は停止し、終了結果を記録してから即座に
  リセットする。
- ウィンドウのサイズ変更、最小化、フォーカス喪失からの復帰を確認する。
- ImGui は3D描画が動いてから追加し、`WantCaptureKeyboard` を確認して入力競合を
  防ぐ。

### 5.10 最初は背景色だけを確かめる

初期化直後に全オブジェクトを出すと、カメラと描画先のどちらが原因で表示されないのか分けにくくなります。依存ライブラリを導入したら、まず`Renderer::render()`のカメラ行列以降を一時的に外し、色と深度のクリアだけで起動してください。背景が表示できたら直方体1個、球体1個、全ステージの順に戻します。各段階でサイズ変更も確認します。

0.25秒を超える実時間を切り捨てる処理は、極端な停止後の大量更新を防ぐための教材用の方針です。その状況では現実の時計よりゲーム内時間が遅れます。通常の描画速度の違いによる移動速度変化と、この意図的な時間の切り捨てを区別してください。


<a id="chapter-6"></a>

## 第6章 周辺を観測して行動を決める

### 6.1 位置を入力する代わりに、周囲をマスへ分ける

プレイヤーの近くに障害物があるとき、人間は少し待ったり横へ避けたりします。AIにも、行動を変えるための情報が必要です。企画に合わせ、周辺を3×3のマスに分け、各マスに壁と障害物があるかを入力します。

1時点では`3×3×2 = 18`個、前回の観測と合わせると36個です。現在のマスも過去のマスも、それぞれの時点のプレイヤーが中心です。配置の変化にはプレイヤー自身の移動も混ざるので、この差だけで障害物の速度を一意に復元できるわけではありません。

### 6.2 入力を行動へ変換する計算

ニューラルネットワークの各ユニットは、入力に重みを掛けて足し、バイアスを加えます。隠れ層では`tanh`を通して値を−1〜1へ収めます。最後の9個の出力のうち最大のものを行動にします。

重みを変えると、同じ観測から違う行動が選ばれます。この重みとバイアス745個を、次の章で遺伝的アルゴリズムが変更します。ここでは逆伝播や勾配計算は使いません。

**完全例：`src/ai/Policy.hpp`**

```cpp
#pragma once
#include "core/Simulation.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>

namespace sil {
inline constexpr float cellWidth = 1.5f;
inline constexpr int decisionSteps = 6;
using Frame = std::array<float, 18>;
using Input = std::array<float, 36>;
using Genome = std::array<float, 745>;

inline Frame observe(const Environment& env) {
    Frame values{};
    const auto& s = env.stage();
    const auto& state = env.state();
    std::size_t index = 0;
    for (int row = 1; row >= -1; --row) {
        for (int col = -1; col <= 1; ++col) {
            const Rect cell{{state.player.x + col * cellWidth, state.player.z + row * cellWidth},
                            {cellWidth / 2, cellWidth / 2}};
            const auto b = s.bounds;
            bool wall = cell.center.x - cell.half.x < b.center.x - b.half.x
                || cell.center.x + cell.half.x > b.center.x + b.half.x
                || cell.center.z - cell.half.z < b.center.z - b.half.z
                || cell.center.z + cell.half.z > b.center.z + b.half.z;
            for (const auto& w : s.walls) wall = wall || overlaps(cell, w);
            bool obstacle = false;
            for (const auto& o : state.obstacles) obstacle = obstacle || overlaps(cell, o);
            values[index++] = wall ? 1.0f : 0.0f;
            values[index++] = obstacle ? 1.0f : 0.0f;
        }
    }
    return values;
}

inline Action choose(const Genome& genes, const Input& input) {
    std::array<float, 16> hidden{};
    std::size_t k = 0;
    for (auto& value : hidden) {
        float sum = 0;
        for (float x : input) sum += genes[k++] * x;
        sum += genes[k++];
        value = std::tanh(sum);
    }
    std::array<float, 9> output{};
    for (auto& value : output) {
        for (float h : hidden) value += genes[k++] * h;
        value += genes[k++];
    }
    // 同点なら先に並ぶ行動を選ぶ。保存形式にもこの順序を記録する。
    constexpr std::array actions{Action::Stop, Action::Up, Action::Down, Action::Left,
        Action::Right, Action::UpRight, Action::DownRight, Action::UpLeft, Action::DownLeft};
    const auto best = std::max_element(output.begin(), output.end());
    return actions[static_cast<std::size_t>(best - output.begin())];
}

class Controller {
public:
    explicit Controller(Genome genes) : genes_(genes) {}
    void reset(const Environment& env) {
        previous_ = observe(env);
        count_ = 0;
        held_ = Action::Stop;
    }
    Action next(const Environment& env) {
        if (count_ % decisionSteps == 0) {
            const Frame current = observe(env);
            Input input{};
            std::copy(current.begin(), current.end(), input.begin());
            std::copy(previous_.begin(), previous_.end(), input.begin() + 18);
            held_ = choose(genes_, input);
            previous_ = current;
        }
        ++count_;
        return held_;
    }
private:
    Genome genes_;
    Frame previous_{};
    int count_{};
    Action held_{Action::Stop};
};

struct Metrics {
    float fitness{};
    float distance{};
    float survival{};
    float lateral{};
    End end{End::Running};
};

inline Metrics evaluate(const Genome& genes, std::uint32_t environmentSeed) {
    Environment env(makeStage());
    env.reset(environmentSeed);
    Controller controller(genes);
    controller.reset(env);
    Metrics m;
    float bestZ = env.stage().start.z;
    while (env.state().end == End::Running) {
        const float oldX = env.state().player.x;
        const auto result = env.step(controller.next(env));
        m.fitness += result.reward;
        bestZ = std::max(bestZ, env.state().player.z);
        m.lateral += std::abs(env.state().player.x - oldX);
    }
    m.distance = bestZ - env.stage().start.z;
    m.survival = static_cast<float>(env.state().steps * Environment::dt);
    m.end = env.state().end;
    return m;
}
}
```

`observe()`の並びは、上の行から下の行へ、各行は左から右へ、各マスでは壁・障害物の順です。境界への接触も長方形同士の重なりとして数える保守的な観測です。プレイヤー自身とゴールは入力に含めません。

パラメーターは隠れユニットごとの「36個の重み、1個のバイアス」、出力ユニットごとの「16個の重み、1個のバイアス」の順です。`16×37 + 9×17 = 745`になります。この並びを変更すると、古い保存個体の意味も変わります。

`Controller`は6更新ごと、つまり0.1秒ごとに行動を決めます。`next()`はシミュレーションの更新直前に1回だけ呼びます。描画のたびに呼ぶと、画面の速さでAIの判断頻度が変わります。自動再開時には環境とコントローラーの両方をリセットしてください。

### 6.3 観測と行動をテストする

**完全例：`tests/PolicyTests.cpp`**

```cpp
#include "ai/Policy.hpp"
#include <iostream>
#include <stdexcept>

void check(bool ok, const char* why) { if (!ok) throw std::runtime_error(why); }
int main() {
    try {
        auto stage = sil::makeStage();
        stage.walls.clear(); stage.obstacles.clear();
        stage.obstacles.push_back({{{0, 0}, {0.1f, 0.1f}}, sil::Axis::X, 0, 1, 0, 0});
        sil::Environment env(stage);
        const auto frame = sil::observe(env);
        check(frame[9] == 1, "center cell obstacle channel");
        check(frame[8] == 0, "player is not wall");
        sil::Genome zero{};
        sil::Input input{};
        check(sil::choose(zero, input) == sil::Action::Stop, "tie chooses stop");
        // 隠れ層592個の後、上方向出力のバイアスだけを1にする。
        zero[592 + 17 + 16] = 1;
        check(sil::choose(zero, input) == sil::Action::Up, "output bias selects up");
        stage.obstacles.clear();
        sil::Environment playable(stage);
        sil::Controller controller(zero);
        controller.reset(playable);
        playable.step(controller.next(playable));
        check(playable.state().player.z > 0, "controller applies action");
        controller.reset(playable);
        check(controller.next(playable) == sil::Action::Up, "controller resets");
        std::cout << "policy tests passed\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
```

**差し込み例：`CMakeLists.txt`の末尾に追加。**

```cmake
if(BUILD_TESTING)
    add_executable(policy_tests tests/PolicyTests.cpp)
    target_link_libraries(policy_tests PRIVATE sphere_core)
    add_test(NAME policy COMMAND policy_tests)
endif()
```

確認課題は、フィールド外のマス、壁と障害物の両方が存在するマス、前試行の履歴が残らないことです。履歴の確認には、2つの環境を使い、途中まで動かしたコントローラーをリセットした場合と、新しいコントローラーの場合の行動列を比較します。

<a id="chapter-7"></a>

## 第7章 遺伝的アルゴリズムで方策を育てる

### 7.1 1個体の評価から1世代へ

ランダムな重みでは、球はゴールへ進むとは限りません。まず多数の個体を同じ条件で試し、良い成績の個体から次の世代を作ります。1世代で必ず改善するわけではないため、最良値だけでなく平均値と実際の到達距離も記録します。

親の選択は、無作為に選んだ3個体の中から成績が最も良いものを選ぶ方式です。適応度が負でも、その大小を比較できます。交叉では各パラメーターを2親のどちらかから取り、突然変異では一部の値へ小さな乱数を加えます。

**完全例：`src/ai/Training.hpp`**

```cpp
#pragma once
#include "ai/Policy.hpp"
#include <algorithm>
#include <random>
#include <stdexcept>
#include <vector>

namespace sil {
struct Individual { Genome genes{}; Metrics metrics{}; };

inline std::vector<Individual> initialPopulation(std::mt19937& random, int count = 64) {
    if (count < 4) throw std::invalid_argument("population must be at least four");
    std::normal_distribution<float> weight(0.0f, 0.2f);
    std::vector<Individual> population(static_cast<std::size_t>(count));
    for (auto& individual : population) {
        for (float& gene : individual.genes) gene = weight(random);
    }
    return population;
}

inline void evaluatePopulation(std::vector<Individual>& population, std::uint32_t seed) {
    for (auto& i : population) i.metrics = evaluate(i.genes, seed);
    std::stable_sort(population.begin(), population.end(), [](const auto& a, const auto& b) {
        return a.metrics.fitness > b.metrics.fitness;
    });
}

inline std::vector<Individual> nextGeneration(const std::vector<Individual>& population, std::mt19937& random) {
    if (population.size() < 4) throw std::invalid_argument("population too small");
    std::uniform_int_distribution<std::size_t> pick(0, population.size() - 1);
    std::bernoulli_distribution parentChoice(0.5);
    std::bernoulli_distribution mutate(0.1);
    std::normal_distribution<float> noise(0.0f, 0.1f);
    const auto tournament = [&]() -> const Genome& {
        std::size_t best = pick(random);
        for (int i = 1; i < 3; ++i) {
            const auto candidate = pick(random);
            if (population[candidate].metrics.fitness > population[best].metrics.fitness) best = candidate;
        }
        return population[best].genes;
    };
    std::vector<Individual> next;
    next.reserve(population.size());
    for (std::size_t i = 0; i < 4; ++i) next.push_back(population[i]);
    while (next.size() < population.size()) {
        const auto& a = tournament();
        const auto& b = tournament();
        Individual child;
        for (std::size_t k = 0; k < child.genes.size(); ++k) {
            child.genes[k] = parentChoice(random) ? a[k] : b[k];
            if (mutate(random)) child.genes[k] += noise(random);
        }
        next.push_back(child);
    }
    return next;
}
}
```

この関数は、入力集団が適応度順に評価・整列済みであることを前提にします。先頭4個体は変更せず残します。子に前世代の評価値を使わず、次の世代で必ず全個体を評価し直します。

同じ環境シードを全個体に渡しますが、この教材のステージ自体には乱数がありません。決定的な方策を同じ条件で繰り返しても、多様な条件での強さを測ったことにはなりません。学習用シードを変えた独立実験と、同じ個体の評価回数を増やすことを区別してください。

### 7.2 遺伝操作の確認

初回の動作確認では4個体に減らし、エリートが変更されないことを確かめます。交叉と突然変異を調べる場合は8個体以上に戻します。4個体だと全個体がエリートになり、探索は進みません。

**確認用の差し込み例：第6章のテストで`Training.hpp`をincludeし、`try`内へ追加。**

```cpp
std::mt19937 randomA(123), randomB(123);
auto populationA = sil::initialPopulation(randomA, 8);
auto populationB = sil::initialPopulation(randomB, 8);
check(populationA[0].genes == populationB[0].genes, "seeded initial population");
sil::evaluatePopulation(populationA, 42);
auto next = sil::nextGeneration(populationA, randomA);
check(next.size() == populationA.size(), "population size preserved");
check(next[0].genes == populationA[0].genes, "elite preserved");
```

乱数生成器の状態が同じなら同じ列を生成できますが、正規分布の変換方法や浮動小数点の計算が実装によって違うため、異なる標準ライブラリ・OS間で同じ学習結果になるとは主張しません。

<a id="chapter-8"></a>

## 第8章 保存・読み込み・CLIを作る

### 8.1 重みだけでは再実行条件が足りない

同じ重みでも、観測のマス幅や行動の並びが違えば行動は変わります。ステージの障害物速度や制限時間が違う場合も、学習時とは別条件です。重みと一緒に、その意味を決める条件を保存します。

教材では読めるテキスト形式を使い、構成を固定したバージョン1とします。ステージ設定は文字列化して保存し、現在の`makeStage()`と一致する場合だけ受け入れます。対応する設定を現在のコードから復元する方式なので、異なるステージの保存ファイルを読む汎用形式ではありません。条件を変えたら読み込みで明示的に失敗し、黙って別条件で実行しないことを優先します。

### 8.2 保存形式の完全例

**完全例：`src/ai/AgentFile.hpp`**

```cpp
#pragma once
#include "ai/Training.hpp"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

namespace sil {
inline std::string stageText() {
    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<float>::max_digits10);
    const auto vec = [&](Vec2 v) { out << v.x << ' ' << v.z << ' '; };
    const auto rect = [&](Rect r) { vec(r.center); vec(r.half); };
    const auto s = makeStage();
    rect(s.bounds); vec(s.start);
    out << s.radius << ' ' << s.speed << ' ' << s.maxSteps << ' ';
    rect(s.goal);
    out << s.walls.size() << ' ';
    for (const auto& w : s.walls) rect(w);
    out << s.obstacles.size() << ' ';
    for (const auto& o : s.obstacles) {
        rect(o.box);
        out << static_cast<int>(o.axis) << ' ' << o.low << ' ' << o.high << ' '
            << o.speed << ' ' << o.phase << ' ';
    }
    return out.str();
}

inline std::string modelText() {
    // 行動順、パラメーター順、観測の並び・占有規則も形式の一部。
    return "3x3-cell1.5-wall-obstacle-current-previous-rowTopLeft-inclusive;"
        "36-16tanh-9argmax;hidden-unit-weights-bias-output-unit-weights-bias;"
        "Stop-Up-Down-Left-Right-UpRight-DownRight-UpLeft-DownLeft;"
        "decision6-dt1/60-substeps4;reward-recordZ1-time0.001-success10-hit-1";
}
struct SavedAgent {
    Genome genes{};
    int generation{};
    std::uint32_t environmentSeed{}, trainingSeed{};
    Metrics metrics{};
};

inline void saveAgent(const std::string& path, const SavedAgent& agent) {
    std::ofstream out(path);
    if (!out) throw std::runtime_error("cannot open agent output: " + path);
    out << std::setprecision(std::numeric_limits<float>::max_digits10);
    out << "SIL_AGENT_V1\n" << std::quoted(stageText()) << '\n' << std::quoted(modelText()) << '\n';
    out << agent.generation << ' ' << agent.environmentSeed << ' ' << agent.trainingSeed << '\n';
    const auto m = agent.metrics;
    out << m.fitness << ' ' << m.distance << ' ' << m.survival << ' ' << m.lateral << ' '
        << static_cast<int>(m.end) << '\n';
    out << agent.genes.size() << '\n';
    for (float gene : agent.genes) out << gene << '\n';
    out.close();
    if (!out) throw std::runtime_error("agent write failed: " + path);
}

inline SavedAgent loadAgent(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("cannot open agent input: " + path);
    std::string magic, stage, model;
    if (!(in >> magic >> std::quoted(stage) >> std::quoted(model))
        || magic != "SIL_AGENT_V1" || stage != stageText() || model != modelText()) {
        throw std::runtime_error("unsupported agent format or configuration");
    }
    SavedAgent agent;
    int end = 0;
    std::size_t count = 0;
    auto& m = agent.metrics;
    if (!(in >> agent.generation >> agent.environmentSeed >> agent.trainingSeed
        >> m.fitness >> m.distance >> m.survival >> m.lateral >> end >> count)
        || agent.generation < 0 || end < 1 || end > 3 || count != agent.genes.size()
        || !std::isfinite(m.fitness) || !std::isfinite(m.distance)
        || !std::isfinite(m.survival) || !std::isfinite(m.lateral)) {
        throw std::runtime_error("invalid agent metadata");
    }
    m.end = static_cast<End>(end);
    for (float& gene : agent.genes) {
        if (!(in >> gene) || !std::isfinite(gene)) throw std::runtime_error("invalid agent weights");
    }
    in >> std::ws;
    if (!in.eof()) throw std::runtime_error("unexpected agent trailing data");
    return agent;
}
}
```

`max_digits10`は、`float`を文字列にして読み戻すために十分な桁数です。既定の少ない桁数で保存すると、同じ重みへ戻らない場合があります。`std::quoted`は空白を含む設定文字列を1項目として読み書きします。

この形式は方策の保存です。集団全体、途中の評価状態、乱数生成器の内部状態は保存していないため、学習途中から同じ乱数列で再開する機能にはなりません。また、保存は指定先を上書きします。実験ごとに別の出力先を使ってください。途中書き込みによる破損への対策は、一時ファイルへ書いて成功後に置換する方法を追加課題とします。

### 8.3 コマンドラインから実験する

以下は教材用CLIの完全例です。新しい依存関係を増やさず、`std::from_chars`で整数を読みます。文字列の一部だけが数値だった場合も、入力末尾まで消費したかを調べて拒否します。

**完全例：`src/app/ExperimentMain.cpp`**

```cpp
#include "ai/AgentFile.hpp"
#include <charconv>
#include <iostream>
#include <string_view>
#include <system_error>

namespace {
struct Options {
    std::string mode;
    std::string stage{"demo"}, output{"agent"}, input;
    std::uint32_t seed{42}, trainingSeed{123};
    int generations{20}, evaluations{1};
};
void help() {
    std::cout << "sphere_experiment train|evaluate [options]\n"
        << "  --stage demo          fixed stage (default demo)\n"
        << "  --seed 0..4294967295  environment seed (default 42)\n"
        << "  --training-seed N     training seed (default 123; train only)\n"
        << "  --generations 0..100000  last generation (default 20; train only)\n"
        << "  --output PREFIX       save prefix (default agent; train only)\n"
        << "  --input FILE          saved agent (required for evaluate)\n"
        << "  --evaluations 1..100000  trials (default 1; evaluate only)\n"
        << "  --help                show help\n";
}
template<class T> T number(std::string_view text) {
    T value{};
    const auto [last, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc{} || last != text.data() + text.size()) throw std::invalid_argument("invalid integer");
    return value;
}
Options parse(int argc, char** argv) {
    if (argc < 2) throw std::invalid_argument("specify train or evaluate");
    Options o;
    o.mode = argv[1];
    if (o.mode != "train" && o.mode != "evaluate") throw std::invalid_argument("unknown mode");
    for (int i = 2; i < argc; ++i) {
        const std::string key = argv[i];
        if (i + 1 >= argc) throw std::invalid_argument("missing option value: " + key);
        const std::string value = argv[++i];
        if (key == "--stage") o.stage = value;
        else if (key == "--seed") o.seed = number<std::uint32_t>(value);
        else if (key == "--training-seed" && o.mode == "train") o.trainingSeed = number<std::uint32_t>(value);
        else if (key == "--generations" && o.mode == "train") o.generations = number<int>(value);
        else if (key == "--output" && o.mode == "train") o.output = value;
        else if (key == "--input" && o.mode == "evaluate") o.input = value;
        else if (key == "--evaluations" && o.mode == "evaluate") o.evaluations = number<int>(value);
        else throw std::invalid_argument("unknown or inapplicable option: " + key);
    }
    if (o.stage != "demo" || o.generations < 0 || o.generations > 100000
        || o.evaluations < 1 || o.evaluations > 100000 || o.output.empty()
        || (o.mode == "evaluate" && o.input.empty())) throw std::invalid_argument("option out of range");
    return o;
}
void train(const Options& o) {
    std::mt19937 random(o.trainingSeed);
    auto population = sil::initialPopulation(random);
    std::cout << "generation,best_fitness,mean_fitness,best_distance,survival,lateral,end\n";
    for (int generation = 0; generation <= o.generations; ++generation) {
        sil::evaluatePopulation(population, o.seed);
        float mean = 0;
        for (const auto& i : population) mean += i.metrics.fitness;
        mean /= static_cast<float>(population.size());
        const auto& best = population.front();
        const auto farthest = std::max_element(population.begin(), population.end(), [](const auto& a, const auto& b) {
            return a.metrics.distance < b.metrics.distance;
        });
        const auto m = best.metrics;
        std::cout << generation << ',' << m.fitness << ',' << mean << ',' << farthest->metrics.distance
            << ',' << m.survival << ',' << m.lateral << ',' << static_cast<int>(m.end) << '\n';
        if (generation == 0 || generation == o.generations / 2 || generation == o.generations) {
            const std::string prefix = o.output + "_g" + std::to_string(generation);
            sil::saveAgent(prefix + "_fitness.sil", {best.genes, generation, o.seed, o.trainingSeed, best.metrics});
            sil::saveAgent(prefix + "_distance.sil", {farthest->genes, generation, o.seed, o.trainingSeed, farthest->metrics});
        }
        if (generation != o.generations) population = sil::nextGeneration(population, random);
    }
}
void evaluate(const Options& o) {
    const auto agent = sil::loadAgent(o.input);
    std::cout << "trial,fitness,distance,survival,lateral,end\n";
    for (int trial = 0; trial < o.evaluations; ++trial) {
        const auto m = sil::evaluate(agent.genes, o.seed);
        std::cout << trial << ',' << m.fitness << ',' << m.distance << ',' << m.survival << ',' << m.lateral
            << ',' << static_cast<int>(m.end) << '\n';
    }
}
}
int main(int argc, char** argv) {
    try {
        for (int i = 1; i < argc; ++i) {
            if (std::string_view(argv[i]) == "--help") { help(); return 0; }
        }
        const auto options = parse(argc, argv);
        if (options.mode == "train") train(options);
        else evaluate(options);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
```

**差し込み例：`CMakeLists.txt`の末尾へ追加。**

```cmake
add_executable(sphere_experiment src/app/ExperimentMain.cpp)
target_link_libraries(sphere_experiment PRIVATE sphere_core)
```

まず世代更新を行わず、初期集団だけを評価して保存します。次に2世代で一連の処理を確認し、問題がなければ世代数を増やします。

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
.\build\Release\sphere_experiment.exe --help
.\build\Release\sphere_experiment.exe train --generations 0 --output first
.\build\Release\sphere_experiment.exe train --generations 2 --output short
.\build\Release\sphere_experiment.exe evaluate --input short_g2_distance.sil --evaluations 3 --seed 42
```

`--output`の親フォルダーは先に用意してください。ログを保存するときはPowerShellの`>`でCSVへ出力できます。学習を20世代進める例は次のとおりです。

```powershell
.\build\Release\sphere_experiment.exe train --generations 20 --seed 42 --training-seed 123 --output run123 > run123.csv
```

CSVの`best_distance`は世代内で到達距離が最大の個体の値です。他の最良値行の指標は適応度が最大の個体に属するため、同じ個体とは限りません。保存名も`_fitness`と`_distance`に分けています。`survival`は秒、`lateral`は各更新のX方向変位の絶対値の合計です。

### 8.4 保存の往復を確かめる

**確認用の差し込み例：PolicyTests.cppで`AgentFile.hpp`をincludeし、`try`内へ追加。**

```cpp
sil::SavedAgent original;
original.genes[7] = 0.123456789f;
original.metrics.end = sil::End::Timeout;
sil::saveAgent("roundtrip.sil", original);
const auto restored = sil::loadAgent("roundtrip.sil");
check(restored.genes == original.genes, "weights survive text roundtrip");
```

破損検査は、保存ファイルをコピーし、先頭のバージョン、パラメーター数、最後の重みを変更・削除して、読み込みが失敗するか確かめます。学習用の元ファイルは残してください。

### 8.5 保存済みAIを画面で動かす

第5章の人間用ループで行動を決めている場所を、次のように置き換えます。変数名は第5章の`env`へ合わせます。コードはコンストラクター直後・更新箇所・自動リセット箇所の3か所へ分けて差し込みます。

```cpp
// WindowsMain.cppのincludeへ追加。
#include "ai/AgentFile.hpp"

// envを構築した後。パスは実行時の作業フォルダーからの相対位置。
const auto saved = sil::loadAgent("run123_g20_distance.sil");
env.reset(saved.environmentSeed);
sil::Controller controller(saved.genes);
controller.reset(env);
```

```cpp
// 固定時間刻みの更新内で、人間の入力から行動を作る部分を置換。
const auto result = env.step(controller.next(env));
if (result.end != sil::End::Running) {
    // ここで終了結果を画面表示用の変数へ記録する。
    env.reset(saved.environmentSeed);
    controller.reset(env);
}
```

第5章のもとの`env.step()`とリセット処理は残して二重に呼ばず、この処理へ置換してください。ImGui版では`if (playing)`の内側を置換し、開始ボタンで環境をリセットする際にも`saved.environmentSeed`を使って`controller.reset(env)`を呼びます。`Controller::next()`の呼び出しも1更新に1回です。モデル選択GUIを追加する場合は、読込失敗を画面に表示し、成功したファイルだけを実行します。教材の最初の到達点はファイル名をコード中で指定する方法です。

<a id="chapter-9"></a>

## 第9章 検証と作品への仕上げ

### 9.1 速さを測る

学習が遅いと感じても、何に時間が掛かるかは測るまで分かりません。まずReleaseビルドで同じ個体の評価時間を測ります。

**確認用の差し込み例：ExperimentMain.cppなどの一時的な計測箇所。`<chrono>`をincludeする。**

```cpp
const auto begin = std::chrono::steady_clock::now();
sil::Genome sample{};
const auto metrics = sil::evaluate(sample, 42);
const auto finish = std::chrono::steady_clock::now();
const double seconds = std::chrono::duration<double>(finish - begin).count();
std::cerr << "evaluation_seconds=" << seconds << " fitness=" << metrics.fitness << '\n';
```

1回の測定は短すぎる場合があります。複数回実行して合計時間を測り、回数、CPU、構成、ステージ、ステップ数を記録します。終了までのステップ数が違う個体の評価時間を、そのままシミュレーション性能の差とは扱いません。速度改善の主張には、同じ条件の変更前後の測定が必要です。

### 9.2 画面の最終確認

| 操作・状況 | 確認すること |
| --- | --- |
| WASDと斜め移動 | 同じ速さで進み、キーを離すと停止する |
| WとS、AとD | 相反する軸が相殺される |
| 壁沿い・角・境界 | 外へ出ず、仕様どおりに動く |
| 高速・最小厚さ | 壁や障害物をすり抜けないか確認する |
| 成功・衝突・時間切れ | 正しい理由を記録し、即時再開する |
| サイズ変更・最小化・復帰 | 画面と深度が更新され、復帰時に時間が飛ばない |
| 別アプリへ切り替える | 入力が残らず、復帰時の扱いが一定である |
| GUI操作中 | 移動キーがゲームに混ざらない |
| 保存済みAI | 学習時と同じ固定時間刻み・行動間隔で動く |

### 9.3 上達を説明できる比較を作る

初期・途中・最終世代の到達距離を、同じステージと同じ実行条件で比べます。適応度だけが増え、到達距離が伸びていない場合は、報酬を稼ぐだけの行動になっていないか実際のプレイを見ます。

教材のコードが動くことと、学習によって十分に改善することは別の確認です。シードを変えた実験も行い、改善した実験だけを一般的な性能と表現しないようにします。ステージクリアに届かなくても、どこまで進むようになったかを動画と数値で示せます。

READMEには、ビルド方法、操作、保存ファイルの配置、学習コマンド、比較条件、既知の制約を記載します。Direct3Dの初期化、固定時間刻み、ゲームと描画の分離、遺伝操作を自分の言葉で説明できれば、提出用の技術説明へつながります。

<a id="cli"></a>

## 付録 サンプルCLI仕様

この仕様は本文のサンプルに対応します。現在のリポジトリに実装済みのCLIを示すものではありません。採用・変更時は実際の引数定義と`--help`を照合します。重複オプションは後の値を採用し、列挙にないオプションと他モード用のオプションはエラーです。`--help`があれば他の引数を解釈せず正常終了します。

### `sphere_headless`

第3章の固定動作確認用です。ステージと行動はコード内で固定し、CLI引数は定義していません。引数を使う実験は次のコマンドを使用します。

| オプション名 | 設定可能な値（型・列挙値・既定値） | 意味 |
| --- | --- | --- |
| なし | 引数を指定せず実行 | demoステージで上へ進み続ける |

### `sphere_experiment train`

| オプション名 | 設定可能な値（型・列挙値・既定値） | 意味 |
| --- | --- | --- |
| `--stage` | 文字列。列挙値`demo`、既定`demo` | 教材の1ステージ。外部ファイルパスではない |
| `--seed` | 符号なし32ビット整数、0〜4294967295、既定42 | 環境シード。教材の固定ステージでは配置を変えない |
| `--training-seed` | 符号なし32ビット整数、0〜4294967295、既定123 | 初期集団と遺伝操作の乱数シード |
| `--generations` | 整数、0〜100000、既定20 | 最終世代番号。0は初期集団のみ。集団の評価回数は世代数＋1 |
| `--output` | 空でないパス接頭辞、既定`agent` | 世代番号・種類・拡張子を付けて保存。親フォルダーは作成済みとする。同名は上書き |
| `--help` | 値なしの真偽フラグ、既定は非指定 | ヘルプを表示して終了 |

個体数64、エリート4、トーナメント3、突然変異確率0.1・標準偏差0.1はコード内の教材設定です。CLIで変更するオプションは実装していません。保存は初期、中間、最終世代に行い、適応度最大と到達距離最大を別名で保存します。

### `sphere_experiment evaluate`

| オプション名 | 設定可能な値（型・列挙値・既定値） | 意味 |
| --- | --- | --- |
| `--stage` | 文字列。列挙値`demo`、既定`demo` | 対応する固定ステージ。保存条件との整合を読込時に確認 |
| `--input` | 空でないファイルパス、必須、既定なし | 保存済み個体。1ファイルのみ。空白を含むパスは引用符で囲む |
| `--seed` | 符号なし32ビット整数、0〜4294967295、既定42 | 評価時の環境シード。保存値の自動採用ではなく明示したCLI値を使う |
| `--evaluations` | 整数、1〜100000、既定1 | 同一シードでの試行回数。繰り返しを異なる条件の評価とは扱わない |
| `--help` | 値なしの真偽フラグ、既定は非指定 | ヘルプを表示して終了 |

引数不正や読込・保存失敗では終了コード1、成功では0です。標準出力へCSV、標準エラー出力へエラーを出します。学習中断・再開、外部ステージ読込、複数ファイルの一括評価は未実装です。

<a id="references"></a>

## 参考資料と検証記録

### 公式資料

- [CMake：add_test](https://cmake.org/cmake/help/latest/command/add_test.html) — CTestへのテスト登録。
- [Microsoft Learn：D3D11CreateDeviceAndSwapChain](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11createdeviceandswapchain) — 描画デバイスと表示用バッファの作成。
- [Microsoft Learn：Direct3D 11の初期化](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-initialize) — デバイス、描画先、ビューポートの役割。
- [DirectXTK：GeometricPrimitive](https://github.com/microsoft/DirectXTK/wiki/GeometricPrimitive) — 球体・立方体の生成と描画。
- [DirectXTK：公開ヘッダー](https://github.com/microsoft/DirectXTK/blob/main/Inc/GeometricPrimitive.h) — 引数と型の定義。
- [DirectXTK：CMake構成](https://github.com/microsoft/DirectXTK/blob/main/CMakeLists.txt) — ソース組込時のターゲットとオプション。
- [Dear ImGui：Win32・DirectX 11公式例](https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx11/main.cpp) — 初期化、フレーム開始、描画、終了の順序。

### 実際に行った確認

2026年10月20日、本文の完全例を一時ディレクトリへ取り出し、macOS・Apple Clang 15・C++20で以下を実行した。プロジェクト本体へ実装ファイルを作成したものではない。

- CMakeの構成とDebugビルド：成功。
- CTest：シミュレーションと方策の2テストが成功。本文の遺伝操作・保存往復の追加例も組み込んで確認。
- ヘッドレス実行、初期集団のみの評価、2世代の学習、保存済み個体の3回評価：成功。
- 不正なCLI入力10例、破損・未対応設定の保存ファイル4例：終了コード1で拒否。
- 同一実行環境・同じシードで2世代の学習を繰り返し、CSVと保存した代表個体が一致することを確認。

教材のステージでは、上へ進むだけの方策や初期集団にも成功する個体があった。この実行結果はコードの動作確認であり、学習による十分な改善を示す実験結果ではない。提出用の学習デモでは障害物の位相・配置・速度を調整し、初期・途中・最終世代を比較する必要がある。

WindowsのMSVCビルド、Direct3D 11の表示、DirectXTKとImGuiのリンク・実行は、この環境では未確認。第5章の完了条件をWindows実機で確認する。DirectXTKやImGuiのリビジョンを固定した後は、その版でビルドし直す。OSをまたぐ浮動小数点結果や学習結果の一致、学習性能、任意の高速設定での非貫通は保証していない。
