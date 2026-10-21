# sphere is learning — C++とDirect3D 11で作るゲーム開発入門

## 目次

- [この教材の使い方](#guide)
- [10/26までに手動プレイを完成させる日別計画](#manual-play-plan)
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

このファイルは教材です。リポジトリの`src/`、`tests/`、`CMakeLists.txt`へ実装を配置したものではありません。本文で「完全例」としたコードブロックは、直前のファイル名へ保存する内容です。「差し込み例」は指定した場所へ追加・置換します。第5章はワークブック形式で、概念・APIの説明を読み、骨組みの空欄と応用課題を自分で実装します。第5章の未完成コードはそのままではビルドできません。

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

<a id="manual-play-plan"></a>

## 10/26までに手動プレイを完成させる日別計画

2026年10月21日（水）から10月26日（月）までの6日間で、**Direct3D 11の画面を見ながら、WASDで1ステージを遊べる状態**を目指します。対象は第1〜5章と第9章9.2の人間操作に関する確認です。第6〜8章のAI観測・学習・保存は10/27以降に進めます。

### 必要な時間の目安

**合計24時間、1日平均4時間**を確保する計画です。内訳は実装・理解・動作確認に約20時間、環境構築や不具合対応の予備に約4時間です。従来の週15時間より多く、この6日間は約9時間の上積みが必要になります。週15時間のペースでは6日間で約13時間のため、今回の期限に向けた余裕は小さくなります。

これは、既存のシミュレーションを使い、教材のサンプルを読みながら自分の実装へ合わせる場合の計画上の見積もりです。実測した所要時間や完成の保証ではありません。第5章は現在、概念理解と自力実装を含むワークブックへ改稿しています。旧形式を前提とした24時間の見積もりは再検証していないため、演習の実績を記録して残り時間を見直してください。Direct3DやCMakeの問題が長引いた場合は、下記の到達点で遅れを判断します。

10/21のクラウド環境で確認した時点では、`Simulation.hpp/.cpp`とヘッドレス実行のコードがあり、描画は作成途中です。`tests/SimulationTests.cpp`は存在せず、CMake構成はその参照で失敗しました。GCCでの直接ビルドとヘッドレス実行は成功しました。Windowsのビルドと画面動作は未確認です。次の違いを調整する時間も計画に含めています。

- 現在のコアでは`Direc`、`GameState`、`state.player.pos`などを使っています。第5章の演習は現在のコアの名前を使います。第3章や第6章以降の教材例との違いは第5.2節の対応表で確認します。
- 描画ファイルの現状の配置は`src/renderer/directX11/`で、教材の`src/renderer/d3d11/`と異なります。実ファイル、`#include`、CMakeの参照先を一致させます。
- 確認時のCMakeでは、コアに`HeadlessMain.cpp`、`sphere_hello`に複数の入口ソースと未存在のテストソースが含まれています。第1章1.2と第5章5.6を参考に、コア、各実行ファイル、Windows専用描画の所属を整理します。描画の修正中もヘッドレスと、実装済みのテストを個別に確認できる構成を目指します。

### 日ごとの作業と、その日の完了条件

各日の時間は、教材を読む時間、実装、ビルド、確認、予備時間をすべて含みます。予備時間を使わなかった日は、翌日の課題へ進むか、実装した処理を説明できるか確かめます。

| 日付 | 確保する時間 | 教材・作業内容 | その日の完了条件 |
| --- | --- | --- | --- |
| 10/21（水） | 3時間（うち予備30分） | 第1〜4章の既存実装を確認。CMakeのターゲットと描画パスを整理し、現在のコアAPIを把握する。第2章は分からない文法を中心に読む | ヘッドレス実行とシミュレーションテストがWindowsで通る。描画用コードがコアへ混入していない |
| 10/22（木） | 3時間（うち予備30分） | 第5章5.2〜5.6・5.10。DirectXTKを用意し、Win32ウィンドウ、Direct3Dデバイス、スワップチェーン、描画先をつなぐ。まず背景色だけ表示する。5.10の進行表に従い5.5のウィンドウ作成も行う | ゲーム用実行ファイルがビルドでき、背景色のあるウィンドウを開閉できる |
| 10/23（金） | 4時間（うち予備30分） | 第5章5.3〜5.6。深度バッファ、カメラ、球体・直方体の表示を完成させる。Rendererを現在の`Stage`・`State`へ合わせる | 床・壁・ゴール・プレイヤー・障害物が、意図した位置と色で見える |
| 10/24（土） | 5時間（うち予備1時間） | 第5章5.5。WASD入力、固定時間刻み、シミュレーション、描画を接続。終了結果を記録して即座にリセットする | 8方向移動と停止ができる。往復障害物を避けて進め、壁・衝突・ゴール・時間切れ・自動再開が働く |
| 10/25（日） | 5時間（うち予備1時間） | 第5章5.7。ImGuiの最小限のステージ選択・開始操作と状態表示を追加。1ステージの配置・速度を調整し、第5章5.9を確認する | 1ステージを選んで開始でき、終了理由を確認しながら繰り返し遊べる。手動プレイを一通り通す |
| 10/26（月） | 4時間（うち予備30分） | 第9章9.2の人間操作の確認。サイズ変更、最小化・復帰、フォーカス切替、GUI操作中の入力を確認。不具合修正とDebug・Releaseの確認に使う | 下記の完成チェックを満たし、起動方法・操作・既知の問題を記録できる |

**10/25中に一通り遊べる状態を作り、10/26を確認と修正に使う**のが目標です。背景色だけの表示は第5章5.10を先に使い、DirectXTKの図形表示まで一度に完成させようとせず、小さく動作を確かめます。

1日の進め方は、最初の15分で前日のビルドと今日の完了条件を確認し、その後は45〜60分単位で「1機能を書く→ビルド→確認」を繰り返します。最後の15分で、動いたこと・未解決のエラー・翌日の最初の作業をメモします。休憩は表の作業時間とは別に取ります。

### 10/26の完成チェック

- [ ] Windows実機でDirect3D 11の3D画面が表示される。
- [ ] 1ステージを画面から選んで開始できる。選択肢は1つでよい。
- [ ] WASDの8方向移動と停止ができ、斜めでも速くならず、相反するキーは軸ごとに相殺される。
- [ ] 壁の外へ出ず、床面内を水平・垂直に往復する障害物が動く。
- [ ] ゴール・障害物への接触・時間切れを判定し、同時成立時は成功、衝突、時間切れの順に優先する。
- [ ] 成功・失敗の結果を表示用に記録し、待ち時間なしでプレイヤー・障害物・経過時間を初期化して再開する。
- [ ] サイズ変更・最小化・別アプリからの復帰で破綻せず、非アクティブ時やGUI操作中の入力を適切に扱う。
- [ ] ヘッドレス実行とシミュレーションテストが引き続き通り、ゲーム用実行ファイルがDebug・Releaseでビルド・起動できる。

### 遅れたときの調整

10/22に背景色が出ない場合は、10/23の冒頭から初期化・リンク・ウィンドウ生成の問題解消を優先します。10/23終了時点で図形が表示できない場合は期限への余裕が少ないため、週末の予備時間を描画の復旧へ回します。10/24終了時点で操作できない場合は、GUIを増やす前に入力から`step()`、描画までを通すことを優先します。

装飾、ゴールテープの凝った表現、テクスチャ、複数ステージ、外部ステージ読込、新しい障害物、音、独自シェーダー、AI関連はこの6日間には追加しません。GUIは1ステージの選択・開始と状態確認に絞ります。シミュレーションと描画の分離、固定時間刻み、衝突・終了・リセットの確認は維持します。

10/25終了時点で手動プレイが通らなければ、残作業を10/26の4時間で解消できるか見直します。難しい場合は「画面表示」「操作」「ゲームルール」のどこまで動いたかと未解決事項を記録し、完成扱いにせず時間追加または期限調整を判断します。この短期計画は、11/16に向けたAI学習・保存・再実行の目標を置き換えるものではありません。

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

## 第5章 WindowsとDirect3D 11 — 描画を組み立てるワークブック

この章では、Direct3D 11の仕組みを理解しながら、自分で描画アプリを書きます。完成済みファイルの転記ではなく、背景色、直方体、球体、ゲーム、GUIの順に、一つずつ動く範囲を広げます。第5.6節のビルド準備を確認し、第5.1節から取り組んでください。最初の実行までの道順は第5.10節にまとめています。

対象はWindows 11 x64、MSVC、C++20、Windows SDK、Direct3D 11、DirectXTKです。macOSやLinuxではこの描画アプリを検証できません。古いDirectX SDKやD3DXは使用しません。

**取り組み方：** 各課題で、先に期待する結果を書き、コードを書いてから確認します。ヒントは折りたたんであります。空欄の`TODO`はあなたが実装する部分です。`#error`付きの骨組みは未完成で、課題を終えてからその行を削除します。空欄を消すだけで完了にはせず、各節の観察項目を満たしてください。コード断片は指定した関数に組み込み、ファイル全体への無条件な置換はしません。

記録欄は各課題で繰り返し使います。本文へ記入しても、別のノートへ書いても構いません。

```text
予想：
自分で書いた処理と、その理由：
実際の表示／エラー：
予想との違いと、次に調べること：
```

### 5.1 Direct3D 11が担当すること

#### ゲームの状態と画像を分ける

DirectXは描画や音声などのAPI群の総称で、Direct3Dはそのうち3D描画を担当します。本章はDirect3D 11を使います。DirectXTKは球・立方体などの描画を助けるライブラリであり、ゲームの移動や衝突を自動で実装するものではありません。

```text
WASDまたはAIの行動
  → Environment::step()：ゲーム規則を適用
  → State：更新後の座標と終了状態
  → Renderer：状態を読むだけで画像を作る
  → Present()：表示を要求
```

`core/Simulation.hpp`にはWindowsやDirect3Dのヘッダーを追加しません。画面を描かずにゲームを実行できることが、後の高速なAI学習につながります。

**課題5-A：境界を説明する。** 次の処理の担当を、シミュレーション、Windowsアプリ、Rendererに分けてください。

- 障害物に接触したか判定する。
- Wキーを押しているか調べる。
- 現在のプレイヤー位置に球を描く。
- 終了結果を記録して次の試行を開始する。

**確認：** 描画を止めても衝突判定が残る設計になっていますか。

#### GPUへ何を渡すのか

画面の球も床も、基本的には三角形の集まりです。CPUが形状、座標変換、色などを設定し、GPUが画像を作ります。標準的な描画の流れを、まず次の粒度で理解してください。

| 段階 | 役割 |
| --- | --- |
| 頂点・インデックスバッファ | 点の位置と、どの点を結んで三角形にするかを保持する |
| 頂点シェーダー | 頂点を物体の座標から画面へ向かう座標に変換するGPUプログラム |
| ラスタライズ | 三角形が画面上のどの部分を覆うかを求める |
| ピクセルシェーダー | 描画する部分の色などを計算するGPUプログラム |
| 深度判定・出力 | 奥行きなどを比較して描画先へ書き込む |

今回はDirectXTKの`GeometricPrimitive`が形状と基本的な描画処理を用意します。自作シェーダーは後の課題です。処理が存在することと、今回どこまで自分が書くかを区別しましょう。

#### 1枚の画像を表示するための部品

| 名前 | 役割と名前の読み方 |
| --- | --- |
| `HWND` | Windowsがウィンドウを識別するハンドル。画像そのものではない |
| `ID3D11Device` | GPU資源などを作る窓口。`I`はインターフェース、`D3D11`はDirect3D 11を表す |
| `ID3D11DeviceContext` | 資源を設定して描画命令を発行する窓口。本章は即時コンテキストを使う |
| `IDXGISwapChain` | 表示用バッファを管理する。DXGIは表示やGPUの選択などを扱う基盤 |
| バックバッファ | 表示する画像を描くための資源 |
| `ID3D11RenderTargetView` | 画像を描画先として利用するためのビュー。画像本体と利用方法を分ける |
| `ID3D11Texture2D` | 2次元画像の資源。模様だけでなく深度の保存にも使う |
| `ID3D11DepthStencilView` | 深度・ステンシル用に画像を利用するためのビュー |
| `D3D11_VIEWPORT` | 描画先のどの範囲へ座標を対応させるかを指定する構造体 |

深度バッファには画素ごとの奥行きを保持します。適切な深度判定を行うことで、奥の物体が後から描かれても、手前の物体を覆わずに済みます。ステンシルは領域ごとに描画を制御する別の機能で、本章では使用しません。

**課題5-B：言葉で追跡する。** 「背景を青にする」「球を描く」「描いた画像を表示する」は、それぞれどの部品への操作ですか。バックバッファとビューの違いも、自分の言葉で2文にまとめてください。

### 5.2 ファイルと責務を決める

`WindowsMain.cpp`にウィンドウ、メッセージ処理、入力、固定時間更新を置きます。`Renderer`にはGPU資源と描画をまとめます。RendererはEnvironmentを所有せず、`const Stage&`と`const State&`を受け取ります。

教材の標準配置は次のとおりです。

```text
src/app/WindowsMain.cpp
src/renderer/d3d11/Renderer.hpp
src/renderer/d3d11/Renderer.cpp
```

既存の`src/renderer/directX11/`を使い続けても構いません。その場合は、以後の`#include`とCMakeのパスをすべて実際の配置へ読み替えてください。両方の場所に別のRendererを作らないでください。

#### 自分のコアAPIを先に確認する

第3章の例と、実装済みのコアでは名前が異なる場合があります。本章の演習は、このリポジトリで確認した右列を使います。左列で実装している場合は、意味を確認して読み替えます。コアの名前を描画のために変更する必要はありません。

| 教材の第3章の例 | 既存コアの名前 |
| --- | --- |
| `Action` | `Direc` |
| `End` | `GameState` |
| `state.player.x`、`state.player.z` | `state.player.pos.x`、`state.player.pos.z` |
| `stage.radius` | `stage.player.radius` |
| `state.steps` | `state.currentStep` |
| `state.end` | `state.gameState` |

`StepResult::end`は既存コアでも`end`です。`State`の終了状態とはメンバー名が異なります。第8.5節でAIを接続するときにも、この対応を使ってください。

**課題5-C：接続の準備。** 自分のヘッダーで、現在位置、半径、経過ステップ、終了状態を取得する式を書いてください。`env.stage()`と`env.state()`が返す型も調べます。先にヘッドレス実行と、実装済みのシミュレーションテストを確認し、テストが存在しない場合は「未実装」と記録してください。

### 5.3 Rendererの宣言と資源の寿命

#### なぜ`ComPtr`を使うのか

Direct3DのインターフェースはCOMの参照カウントで寿命を管理します。`Microsoft::WRL::ComPtr<T>`は参照の解放を自動化するスマートポインタです。通常のC++オブジェクト用の`std::unique_ptr`とは用途が異なります。

| 式 | 意味 |
| --- | --- |
| `device_.Get()` | 所有権を渡さず、生ポインタを借りる |
| `device_.GetAddressOf()` | APIが生成結果を書き込むためのポインタ格納先を渡す。ここでは空のComPtrに使う |
| `device_.Reset()` | 所有している参照を解放する |
| `device_.ReleaseAndGetAddressOf()` | 既存の参照を解放してから出力先を得る。再生成時に使える |

`GetAddressOf()`を繰り返し呼ぶだけでは、既存の参照は解放されません。本章ではサイズ変更時にビューを`Reset()`してから再生成します。`ComPtr`が持つポインタへ手動で`Release()`しないでください。

#### 宣言の骨組みを書く

`Renderer.hpp`に次の宣言を土台として書き、メンバーを自分で追加してください。関数名の小文字始まりは、このプロジェクトで定義するメソッドです。`CreateTexture2D`など大文字始まりの名前は後で呼ぶAPIです。

```cpp
#pragma once
#include <cstdint>
#include <memory>
#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <GeometricPrimitive.h>

namespace sil { struct Stage; struct State; }

namespace sil::renderer {
class Renderer final {
public:
    Renderer(HWND window, std::uint32_t width, std::uint32_t height);
    ~Renderer() = default;
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void resize(std::uint32_t width, std::uint32_t height);
    void render(const Stage& stage, const State& state);
    void present();
    ID3D11Device* device() const noexcept { return device_.Get(); }
    ID3D11DeviceContext* context() const noexcept { return context_.Get(); }

private:
    void createDeviceAndSwapChain(HWND window);
    void createSizeDependentResources(std::uint32_t width, std::uint32_t height);

#error 課題5-Dのメンバー宣言を完成させてから削除する
    // TODO: 所有する資源、ビューポート、幅と高さ、図形を宣言する。
};
}
```

**課題5-D：所有するものを宣言する。** 第5.1節の型を使って、`device_`、`context_`、`swapChain_`、`renderTargetView_`、`depthTexture_`、`depthStencilView_`を宣言してください。`viewport_`、初期値0の`width_`と`height_`、図形の`cube_`と`sphere_`も追加します。図形の型は`std::unique_ptr<DirectX::GeometricPrimitive>`です。

<details>
<summary>ヒント：まず一つだけ宣言する</summary>

デバイスは`Microsoft::WRL::ComPtr<ID3D11Device> device_;`です。他のCOM資源も「所有方法は同じで、インターフェース型が違う」と考えてください。ビューポートはポインタではなく構造体を値で保持します。

</details>

**説明課題：** `render()`を呼んでもゲーム状態が変わらない理由、Rendererのコピーを禁止する理由、`render()`と`present()`を分ける理由を記入してください。最後の問いは「3Dの後にGUIを描く」と考えると解けます。

### 5.4 Direct3Dの初期化から図形表示まで

`Renderer.cpp`に実装します。自分のRendererヘッダー、`core/Simulation.hpp`、`<stdexcept>`、`<sstream>`、`<iomanip>`、`<iterator>`、`<DirectXMath.h>`、`<DirectXColors.h>`を必要に応じてincludeしてください。この節は一度に完成させず、背景色まで書いたら第5.5節のウィンドウと接続して実行します。

ヘッダーの宣言に対応する定義は、`namespace sil::renderer { ... }`内で、たとえば`void Renderer::present() { ... }`の形で書きます。`Renderer::`は「このクラスのメソッドの定義」という意味です。無名名前空間の補助関数は、その外に置けます。`DirectX::`を毎回書くか、関数内で`using namespace DirectX;`を使うかも統一してください。

#### 実習1：失敗を見えるようにする

`HRESULT`は多くのAPIが返す結果コードです。`SUCCEEDED(hr)`と`FAILED(hr)`で成否を判断します。`hr == 0`だけで成功を判断するものではありません。

**課題5-E：** 無名名前空間に`void throwIfFailed(HRESULT result, const char* operation)`を書きます。成功なら戻り、失敗なら処理名とHRESULTを含む`std::runtime_error`を投げてください。

<details>
<summary>ヒント：ここは既習のC++で書く</summary>

分岐には`FAILED(result)`、メッセージの組み立てには`std::ostringstream`、16進表記には`std::hex`を使えます。HRESULTを`unsigned long`へ変換して出力すると、ビットパターンを読みやすくできます。後のAPI呼び出しは、この関数で戻り値を確認します。

</details>

#### 実習2：デバイスとスワップチェーンを作る

`DXGI_SWAP_CHAIN_DESC`の`DESC`はdescription、つまり設定を表す構造体です。`{}`で初期化してから、必要なフィールドを設定します。本章は初期化の理解を優先した従来型のスワップチェーンを使います。flip modelなどの別方式は、まず表示できてから学びます。

| フィールド | 今回の設定 | 意味 |
| --- | --- | --- |
| `BufferDesc.Format` | `DXGI_FORMAT_R8G8B8A8_UNORM` | RGBA各8ビットの色 |
| `SampleDesc.Count` | `1` | マルチサンプルを行わない |
| `BufferUsage` | `DXGI_USAGE_RENDER_TARGET_OUTPUT` | 描画先として使う |
| `BufferCount` | `1` | 今回の方式でのバックバッファ数 |
| `OutputWindow` | 引数の`window` | 表示先のウィンドウ |
| `Windowed` | `TRUE` | ウィンドウ表示 |
| `SwapEffect` | `DXGI_SWAP_EFFECT_DISCARD` | Present後の内容保持を前提としない |

次は**呼び出し方の参考**です。前段の設定と、呼び出し前後の処理は自分で書きます。

```cpp
// description、flags、levelsは、下記の課題で先に宣言・設定する。
D3D_FEATURE_LEVEL actualLevel{};
throwIfFailed(D3D11CreateDeviceAndSwapChain(
    nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags,
    levels, static_cast<UINT>(std::size(levels)), D3D11_SDK_VERSION,
    &description, swapChain_.GetAddressOf(), device_.GetAddressOf(),
    &actualLevel, context_.GetAddressOf()), "D3D11CreateDeviceAndSwapChain");
```

引数は左から、GPUの選択（`nullptr`で既定）、ハードウェア描画、ソフトウェア実装用のモジュール（今回はなし）、作成フラグ、要求する機能レベル配列、その要素数、SDKのAPIバージョン定数、表示設定、スワップチェーンの出力先、デバイスの出力先、実際の機能レベルの出力先、コンテキストの出力先です。機能レベルはGPUが提供する機能の水準であり、Windowsのバージョンではありません。

**課題5-F：** `createDeviceAndSwapChain()`を実装してください。設定表を`description`へ反映し、`levels`には`D3D_FEATURE_LEVEL_11_1`、`D3D_FEATURE_LEVEL_11_0`の順で指定します。`flags`の初期値を`D3D11_CREATE_DEVICE_BGRA_SUPPORT`とし、Debug構成では`D3D11_CREATE_DEVICE_DEBUG`をビットORで加えてください。

**観察：** デバイス、コンテキスト、スワップチェーンが生成されたかをデバッガで確認します。失敗時には処理名とコードを記録します。デバッグレイヤーがないというエラーならWindowsのオプション機能「Graphics Tools」を確認してください。`SUCCEEDED`を無視して先へ進まないでください。

#### 実習3：画像を描画先として使う

資源の作成は`device_`、命令の設定・発行は`context_`です。この違いを、これから呼ぶ関数でも確認します。

```cpp
// ここはCOMの出力引数の書き方の参考。関数全体は自分で組み立てる。
Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
throwIfFailed(swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf())),
              "GetBuffer");
```

`0`は先頭のバックバッファです。`IID_PPV_ARGS`は要求するインターフェースの識別子と出力先を型に合わせて渡す補助マクロです。

**課題5-G：** `createSizeDependentResources(width, height)`へ、次の順序で処理を書いてください。

1. 上の方法でバックバッファを取得する。
2. `CreateRenderTargetView`で、その画像用の`renderTargetView_`を作る。
3. `viewport_`の左上を`(0, 0)`、幅と高さを引数の値、深度範囲を`[0, 1]`にする。

<details>
<summary>ヒント：初めてのAPIの引数を読む</summary>

`device_->CreateRenderTargetView(resource, description, output)`の順です。画像は`backBuffer.Get()`、標準のビュー設定は`nullptr`、出力先は`renderTargetView_.GetAddressOf()`です。戻り値を`throwIfFailed`へ渡します。ビューポートのフィールドは`TopLeftX`、`TopLeftY`、`Width`、`Height`、`MinDepth`、`MaxDepth`です。

</details>

最初のコンストラクターでは、デバイスを作り、この関数を呼び、幅と高さを保存してください。深度と図形は後で追加します。`render()`も次の実習の背景色だけから始めます。

#### 実習4：背景色を出す

| 関数 | 今回使う引数と役割 |
| --- | --- |
| `context_->OMSetRenderTargets(count, views, depth)` | `1`、描画先ビュー配列の先頭、深度ビュー（最初は`nullptr`）。OMはOutput Mergerという出力段階 |
| `context_->RSSetViewports(count, viewports)` | `1`、`&viewport_`。RSはRasterizer Stage |
| `context_->ClearRenderTargetView(view, color)` | ビューの生ポインタ、RGBAの`float[4]`。各成分は今回は0〜1 |
| `swapChain_->Present(syncInterval, flags)` | 今回は`1, 0`。表示のタイミングを垂直同期に合わせる設定 |

**課題5-H：** `render()`に描画先とビューポートの設定、背景色のクリアを書き、`present()`に表示要求を書いてください。幅または高さが0なら描画を行わないガードも書きます。`Present`はHRESULTを返すので確認します。クリアや設定の上記Contextメソッドは`void`です。

`OMSetRenderTargets`の第2引数は「ビュー一つ」ではなく「ビューのポインタの配列」です。`renderTargetView_.GetAddressOf()`は1要素の配列の先頭として渡せます。ここでは出力先としてではなく入力として使っている点に注意してください。

**ここで第5.5節の実習1〜2へ進み、ウィンドウ上に背景色を出します。** `Stage`と`State`はまだ描画に使用しなくて構いません。未使用引数の警告は引数名の省略などで意図を明示します。

**確認：** 赤成分だけ変えて再ビルドし、予想した色になることを確かめてください。図形もカメラもない段階で、表示経路が動くことを確認します。

#### 実習5：深度とサイズ変更を扱う

深度の資源はバックバッファとは別に作ります。`D3D11_TEXTURE2D_DESC`を初期化し、次を設定してください。

| フィールド | 値 |
| --- | --- |
| `Width`、`Height` | 描画領域の幅と高さ |
| `MipLevels`、`ArraySize` | それぞれ`1` |
| `Format` | `DXGI_FORMAT_D24_UNORM_S8_UINT` |
| `SampleDesc.Count` | `1`（描画先と一致させる） |
| `Usage` | `D3D11_USAGE_DEFAULT` |
| `BindFlags` | `D3D11_BIND_DEPTH_STENCIL` |

`device_->CreateTexture2D(&description, nullptr, output)`で画像を作り、`CreateDepthStencilView(resource, nullptr, output)`でビューを作ります。HRESULTを両方確認してください。`render()`では深度ビューも描画先設定へ渡し、毎フレーム`ClearDepthStencilView(view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0F, 0)`を呼びます。本章ではD3D11の既定の深度ステート（有効、比較LESS、書き込みあり）を使います。

**課題5-I：** 深度作成を`createSizeDependentResources()`へ追加し、`resize()`を次の仕様で実装してください。

1. 幅または高さが0なら戻る。
2. 初回ならそのまま資源を作る。
3. 2回目以降は`OMSetRenderTargets(0, nullptr, nullptr)`で古い描画先のバインドを外す。
4. 古い描画先ビュー・深度ビュー・深度画像を解放する。
5. `swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0)`を成功確認付きで呼ぶ。
6. 新しいサイズで資源を作り、幅と高さを保存する。

コンストラクターのサイズ依存資源の作成も`resize()`経由へまとめます。初回の判定には初期値0の`width_`を使えます。バックバッファへの余分な参照をメンバーに残すと、`ResizeBuffers`が失敗する原因になります。

**応用課題：** 新しい深度画像に古い幅を使った場合、何と何が不一致になりますか。サイズ変更直後の`viewport_`と幅・高さをデバッガで確認してください。

#### 実習6：カメラと座標変換を理解する

| 変換 | 問い | 主な関数 |
| --- | --- | --- |
| ワールド | 物体をどの大きさで、どこに置くか | `XMMatrixScaling`、`XMMatrixTranslation` |
| ビュー | どこから、どこを見るか | `XMMatrixLookAtLH` |
| 射影 | 画角と遠近感をどうするか | `XMMatrixPerspectiveFovLH` |

`LH`は左手系を意味します。Direct3Dで必ず左手系しか使えないわけではありませんが、本章は左手系で統一します。Xは右、Yは高さ、Zはゴール方向です。

カメラの例として位置`(0, 16, -18)`から`(0, 0, 0)`を見て、上方向を`(0, 1, 0)`にします。`XMMatrixLookAtLH(eye, target, up)`へ各ベクトルを渡してください。ベクトルは`XMVectorSet(x, y, z, w)`で作り、ここでは`w = 0`で構いません。

射影は`XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ)`です。画角はラジアンなので`XMConvertToRadians(50.0F)`、縦横比は`float(width_) / float(height_)`、手前と奥の範囲は`0.1F`と`100.0F`を使います。整数同士の割り算にしないでください。

**課題5-J：** `render()`に`view`と`projection`を作ってください。その前に「画角を狭くしたら同じ位置の球は大きく見えるか」「横長のウィンドウでも球を円らしく映すには何を更新するか」を予想します。図形を出してから確かめます。

行列の掛け算は順序を入れ替えられません。本章のDirectXMathとDirectXTKの組み合わせでは、`Scaling * Translation`で、形状を拡大してから所定の位置へ移します。行列の要素を暗記するより、数値を一つずつ変えて結果を確認してください。

#### 実習7：直方体からステージへ広げる

DirectXTKの`GeometricPrimitive`は形状と描画処理をまとめたクラスです。コンストラクターでデバイス作成後に図形を作り、毎フレームは使い回します。

```cpp
// 初めて使うライブラリの呼び出し例。直方体の描画全体の解答ではない。
cube_ = DirectX::GeometricPrimitive::CreateCube(context_.Get(), 1.0F, false);
```

引数はコンテキスト、一辺の長さ、右手系で生成するかです。`false`で本章の左手系へ合わせます。球は`CreateSphere(context, diameter, tessellation, rhcoords)`で作ります。直径`1.0F`、分割数`24`、右手系指定`false`を使ってください。

描画呼び出しは`cube_->Draw(world, view, projection, color)`です。色は`DirectX::Colors::SlateGray`などを使えます。

**課題5-K：** まず幅2、高さ1、奥行き2の直方体を、中心`(0, 0.5, 0)`へ描いてください。次に、これを汎用の補助関数へまとめます。

```cpp
// Renderer.cppの無名名前空間に置く骨組み。FXMATRIX等は行列引数用の型。
void drawRect(DirectX::GeometricPrimitive& cube, const sil::Rect& rect,
              float centerY, float height, DirectX::FXMVECTOR color,
              DirectX::FXMMATRIX view, DirectX::FXMMATRIX projection) {
#error Rectの半サイズを実寸へ変換し、worldを作ってDrawする
    // TODO: 拡大する。rect.halfは「半分の長さ」。
    // TODO: 移動する。rect.centerの2成分はXとZ。
    // TODO: cube.Draw(...)を呼ぶ。
}
```

<details>
<summary>ヒント1：サイズを紙に書く</summary>

幅は`rect.half.x * 2`、奥行きは`rect.half.z * 2`です。高さは引数をそのまま使います。中心座標は`(rect.center.x, centerY, rect.center.z)`です。

</details>

<details>
<summary>ヒント2：既習の変換を組み合わせる</summary>

`XMMatrixScaling(幅, 高さ, 奥行き) * XMMatrixTranslation(中心X, 中心Y, 中心Z)`をワールド行列にします。課題5-Jで作った二つのカメラ行列も、Drawへ渡してください。

</details>

**課題5-L：応用部分は自分で書く。** 次の表を仕様として、`render()`の残りを実装してください。ここには完成コードを置きません。

| 対象 | 読むデータ | 描画の条件 |
| --- | --- | --- |
| 床 | `stage.bounds` | 厚さ0.2、中心Yは-0.1。上面がY=0になる |
| ゴール | `stage.goal` | 厚さ0.05、中心Yは0.025、緑色 |
| 壁 | `stage.walls`の各Rect | 高さ1、中心Yは0.5、灰色 |
| 障害物 | **`state.obstacles`**の各Rect | 高さ0.75、中心Yは0.375、赤色 |
| プレイヤー | `state.player.pos`、`stage.player.radius` | 球の直径を半径の2倍にし、中心Yを半径にする |

プレイヤーには、直径1の球用のワールド行列を別に作ります。壁と障害物のループは、既習のコンテナ操作と`drawRect()`の応用です。`stage.obstacles`は初期設定なので、現在の障害物表示には使いません。

**観察：** 球のYを0にすると何が起きるか予想してから試し、半径へ戻してください。視点を変えて奥行きも確認します。図形は出るのにゲーム配置が見えないときは、まずカメラの注視点と配置範囲を調べます。

### 5.5 Windowsの入口、入力、固定時間更新

Direct3Dはウィンドウを作りません。Win32が作ったウィンドウの`HWND`を受け取って描画します。この節の前半だけを先に書けば、第5.4節の背景色を検証できます。

`WindowsMain.cpp`には`<windows.h>`、自分のRendererヘッダー、`core/Simulation.hpp`をincludeします。時間計測には`<chrono>`、時間の上限設定には`<algorithm>`、整数型には`<cstdint>`、例外処理には`<exception>`を追加します。`WindowState`や入力の補助関数は無名名前空間へ置き、入口の`wWinMain`はグローバルスコープへ定義してください。

#### 実習1：Win32の名前を読む

| 名前 | 意味と、今回の使い方 |
| --- | --- |
| `wWinMain` | Unicode版のWindows GUIアプリの入口。コンソールの`main`に相当 |
| `WNDCLASSEXW` | メッセージの処理方法などを登録する構造体。「C++のclass」とは別 |
| `RegisterClassExW` | ウィンドウクラスを登録する。失敗時は0 |
| `CreateWindowExW` | ウィンドウを作り、HWNDを返す。失敗時は`nullptr` |
| `ShowWindow` | 作ったウィンドウを表示する |
| `GetClientRect` | 枠やタイトルバーを除く、描画領域の大きさを取得する |
| `PeekMessageW` | 待ち続けずにメッセージを取得する。描画ループと共存できる |
| `DispatchMessageW` | メッセージをウィンドウプロシージャへ送る |
| `DefWindowProcW` | 自分で扱わないメッセージを既定の処理へ渡す |

`W`付きAPIへは`L"文字列"`を渡します。`HINSTANCE`はアプリのインスタンスを識別するハンドルです。`WPARAM`と`LPARAM`はメッセージの付加情報、`LRESULT`は処理結果、`CALLBACK`と`WINAPI`は呼び出し規約の指定です。独自の型や関数名だと思って読み飛ばさず、まずこの役割を押さえます。

**課題5-M：** `WindowsMain.cpp`に次の二つの関数を定義してください。入口全体の解答は用意しません。以下の手順と表を使って組み立てます。

```cpp
LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand);
```

最初の`windowProcedure`は`WM_CLOSE`で`DestroyWindow(window)`、`WM_DESTROY`で`PostQuitMessage(0)`を呼び、それぞれ0を返します。それ以外は`DefWindowProcW(window, message, wParam, lParam)`へ渡します。`WM_QUIT`はこの関数へ配送されず、後述のメッセージループで扱います。

`wWinMain`は次の順序で書きます。

1. `constexpr wchar_t className[]`に固有のクラス名を用意する。
2. `WNDCLASSEXW`をゼロ初期化し、`cbSize`に自身の`sizeof`、`hInstance`に`instance`、`lpszClassName`にクラス名、`lpfnWndProc`に上の関数を設定する。`hCursor`は`LoadCursorW(nullptr, IDC_ARROW)`、背景ブラシはnullptrとする。
3. `RegisterClassExW`へ構造体のアドレスを渡す。失敗なら`GetLastError()`の値を記録して終了する。
4. `CreateWindowExW`でウィンドウを作る。初期ウィンドウサイズは1280×720を使い、失敗を確認する。
5. `ShowWindow(window, showCommand)`で表示する。
6. `GetClientRect`で実際の描画領域を測り、その幅と高さでRendererを構築する。

<details>
<summary>ヒント：引数の多いCreateWindowExWを分けて読む</summary>

順番は、拡張スタイル`0`、登録したクラス名、タイトル`L"sphere is learning"`、スタイル`WS_OVERLAPPEDWINDOW`、XとY（各`CW_USEDEFAULT`）、幅と高さ、親ウィンドウ`nullptr`、メニュー`nullptr`、`instance`、追加データ（最初は`nullptr`）です。指定した1280×720は枠を含むため、Rendererには必ずGetClientRectの結果を渡します。

</details>

#### 実習2：メッセージと描画を繰り返す

メッセージ処理は、OSから「閉じる」「サイズが変わった」などの通知を受け取る処理です。ゲームの更新関数とは別です。

**課題5-N：** `bool running = true`とループを用意し、次の順序をC++で実装してください。まだEnvironmentは不要です。

```text
runningの間：
    キューにメッセージがある間、PeekMessageW(..., PM_REMOVE)で取り出す
        WM_QUITならrunningをfalseにし、そのメッセージは配送しない
        それ以外はTranslateMessageとDispatchMessageWへ渡す
    runningがfalseならループを抜ける
    renderer.render(...)で背景をクリアする
    renderer.present()で表示する
```

`MSG message{}`を受信先にし、`PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)`を使います。`TranslateMessage`は文字入力メッセージの生成を助けます。`WM_QUIT`の検出後は、そのフレームの描画へ進まないでください。

この段階で`render(const Stage&, const State&)`へ渡す値には、`makeStage()`で作ったStageと値初期化したStateを使えます。まだ中身は描画に使いません。`render()`の引数を何度も変更する必要はありません。

Rendererの構築とループを`try`で囲み、`std::exception`を捕まえて`MessageBoxA(window, error.what(), "Rendering error", MB_OK | MB_ICONERROR)`などで通知してください。終了コードは失敗時1、正常時0です。`GetLastError`はWin32の失敗直後に確認する情報で、Direct3DのHRESULTとは別物です。

**ここで背景色の表示・変更・閉じる操作を確認します。** 最初の確認後、最終版のサイズ変更と一時停止に必要な状態を追加してください。

#### 実習3：通知を状態へ変換する

アプリが使う`WindowState`構造体を作ります。次の項目を持たせてください。

| メンバー | 初期値・意味 |
| --- | --- |
| `width`、`height` | 1280、720。描画領域の幅と高さ |
| `resizePending` | false。メインループで資源を作り直す印 |
| `minimized` | false。最小化中か |
| `hasFocus` | true。アプリがアクティブか |
| `resetClock` | false。復帰後に停止時間を加算しないための印 |

ウィンドウプロシージャへ渡すアプリ状態は、`GWLP_USERDATA`というウィンドウごとの保存場所を使います。これは初見のWin32手続きなので、接続部分だけを参考に示します。グローバルなRendererを作る代わりに状態を渡します。

```cpp
// windowProcedureの先頭。
auto* state = reinterpret_cast<WindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
// switch内。CreateWindowExWの最後の引数を&windowStateへ変更して使う。
case WM_NCCREATE: {
    const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
    SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
    return TRUE;
}
```

`WindowState windowState`は`CreateWindowExW`の前に作り、ウィンドウが生きている間は存続させます。生成途中にもメッセージが来るので、`state`がnullptrの可能性を確認してください。

**課題5-O：** 次のメッセージ処理を自分で追加してください。

- `WM_SIZE`：`wParam == SIZE_MINIMIZED`を判定する。`LOWORD(lParam)`が幅、`HIWORD(lParam)`が高さ。最小化中以外は`resizePending`を立て、`resetClock`も立てる。
- `WM_ACTIVATEAPP`：`wParam != FALSE`を`hasFocus`へ保存し、`resetClock`を立てる。
- メインループ：保留されたサイズ変更を`renderer.resize()`へ渡して印を戻す。最小化中や非アクティブ時は更新・描画を休み、`Sleep(10)`などで空回りを避ける。

作成直後は`GetClientRect`の値で状態を更新し、`resizePending`をfalseへ戻します。メッセージ内でGPU資源を直接作り直さず、メインループへ依頼する形にしてください。

#### 実習4：WASDから既存の行動へ変換する

`GetAsyncKeyState(key)`の戻り値の上位ビット`0x8000`は、現在押されていることを表します。押した出来事である`WM_KEYDOWN`とは用途が異なります。

**課題5-P：** `bool isKeyDown(int key)`を書き、次の空欄を埋めてください。`sil::direcFromAxes`は既存コアにある関数です。

```cpp
sil::Direc readAction() {
#error 相反するキーが軸ごとに打ち消される入力を書く
    // TODO: horizontal = Dの押下値 - Aの押下値
    // TODO: vertical   = Wの押下値 - Sの押下値
    // TODO: 既存関数で二つの軸を9行動へ変換して返す。
}
```

**応用確認：** Wのみ、W+D、W+S、W+A+Sを押した場合の軸と行動を、実行前に表へ書いてください。斜め移動の速度補正はシミュレーション側に残します。AIから斜め行動を渡した場合にも同じ規則が適用されるためです。

#### 実習5：描画の速さからゲーム時間を独立させる

固定更新には`std::chrono::steady_clock`を使います。`now - previousTime`を`std::chrono::duration<double>`へ変換して`count()`を呼ぶと、秒数を得られます。`accumulator`は、次の1ステップに足りない時間を持ち越す変数です。

**課題5-Q：** `sil::Environment env{sil::makeStage()}`、固定の`initialSeed`、終了結果用の`lastEnd`、`accumulator = 0.0`、前回時刻を用意し、次の設計をコードにしてください。

```text
各フレームでメッセージとサイズ変更を処理する
nowを取得する
resetClockが立っていたら、前回時刻をnowにし、蓄積時間を0へ戻す
最小化中または非アクティブなら、同様に時計をリセットして休む
elapsed = 前回からの秒数（上限0.25秒）
前回時刻 = now
accumulatorへelapsedを加算する
accumulator >= Environment::dt の間：
    入力から行動を得てenv.step()を1回呼ぶ
    終了なら結果をlastEndへ保存してからenv.reset(initialSeed)
    accumulatorからdtを引く
renderer.render(env.stage(), env.state())
renderer.present()
```

既存コアでは`lastEnd`の型は`sil::GameState`、更新結果は`result.end`です。終了時も待たずにリセットし、直前の結果は別変数に保持してください。`if`で1回だけ更新するループでは、描画が遅いとゲームも遅くなります。

<details>
<summary>ヒント：時間の計算だけ確認する</summary>

`std::min(std::chrono::duration<double>(now - previousTime).count(), 0.25)`で上限を付けられます。停止中は`previousTime = now`と`accumulator = 0.0`の両方が必要です。`resetClock`も処理後にfalseへ戻します。

</details>

**机上課題：** dtが1/60秒、accumulatorが0なら、1/120秒が経過したとき更新は何回ですか。次のフレームでも1/120秒が経過したら何回になりますか。実時間0.25秒超の切り捨ては大量更新を避ける教材上の方針であり、その場合は現実の時計よりゲーム内時間が遅れます。

**実機確認：** 描画の負荷を変えても通常時の移動速度が大きく変わらないこと、フォーカスを外して戻しても即座に時間切れにならないこと、障害物の現在位置が動くことを確認します。

### 5.6 依存ライブラリとCMakeを接続する

ここは第5.4節をビルドする前に準備します。既存のCMakeを丸ごと上書きせず、第1章のターゲット分離と照合してください。ソースの所属を間違えると、複数の入口関数やWindowsヘッダーの混入によるエラーになります。

- `sphere_core`にはシミュレーションの実装だけを入れる。
- `sphere_headless`と`sphere_tests`は、それぞれ自分の入口とcoreを使う。
- `sphere_game`に`WindowsMain.cpp`とRendererを入れる。
- `sphere_hello`を残す場合、その入口は`Hello.cpp`だけにする。
- テストファイルがない場合は、テストが実行できた扱いにしない。

DirectXTKとImGuiは、このリポジトリでは既にsubmoduleとして登録されています。`git submodule add`を重ねず、プロジェクト根で`git submodule update --init --recursive`を実行します。GUIのコードは第5.7節まで使いません。

**課題5-R：** 次の骨組みを完成させてください。これは未完成のCMakeです。`message(FATAL_ERROR ...)`は必要な箇所を埋めてから削除します。

```cmake
if(WIN32)
    set(BUILD_TOOLS OFF CACHE BOOL "Build DirectXTK auxiliary tools" FORCE)
    set(BUILD_XAUDIO_WIN8 OFF CACHE BOOL "Build DirectXTK audio" FORCE)
    add_subdirectory(external/DirectXTK)

    message(FATAL_ERROR "課題5-R: ソースとリンク先を自分の配置で定義する")
    # TODO: add_executable(sphere_game WIN32 ...)で描画アプリのソースを指定。
    # TODO: include先src、コンパイル定義、リンク先を指定。
    # TODO: MSVCの場合だけ/W4と/utf-8を設定。
endif()
```

必要なコンパイル定義は`WIN32_LEAN_AND_MEAN`、`NOMINMAX`、`UNICODE`、`_UNICODE`です。前二つはWindowsヘッダーの範囲を絞ることと、`min`/`max`マクロの干渉防止、後二つはUnicode設定です。リンク先は`PRIVATE`で`sphere_core`、`DirectXTK`、`user32`、`d3d11`、`dxgi`を指定します。`add_subdirectory`で取り込んだターゲット名は`DirectXTK`です。

<details>
<summary>ヒント：第1章のCMakeから応用する</summary>

ソースは`src/app/WindowsMain.cpp`と、選んだ場所の`Renderer.cpp`（必要なら`.hpp`も）です。`target_include_directories`、`target_compile_definitions`、`target_link_libraries`を使ってください。`if(WIN32)`はWindowsでだけ構成する条件、`add_executable`の`WIN32`はGUIの入口を選ぶ指定です。MSVC固有オプションは別途`if(MSVC)`で囲みます。

</details>

この章の演習は既存のWindows用`build/`と混ぜず、新しい`build-workbook/`を使います。ローカルの除外設定`.git/info/exclude`に`/build-workbook/`を追記し、生成物をコミットしないようにしてください。Visual StudioのDeveloper PowerShellでプロジェクト根から実行します。

```powershell
cmake -S . -B build-workbook -A x64
cmake --build build-workbook --config Debug --target sphere_game
.\build-workbook\Debug\sphere_game.exe
```

**確認：** ヘッダーが見つからないエラー、リンク時の未解決シンボル、実行時のHRESULT失敗を区別し、それぞれどこを調べるかを書いてください。core、ヘッドレス、実装済みテストも別にビルドして、描画の依存が混入していないか確認します。

### 5.7 Dear ImGuiで操作画面を作る

3Dと手動操作を確認してから取り組みます。ImGuiは毎フレームの関数呼び出しでUIを記述するライブラリです。Win32バックエンドが入力など、DX11バックエンドが描画などを担当します。

#### 実習1：依存関係と寿命

**課題5-S：** `if(WIN32)`内に`sil_imgui`というSTATICライブラリを作り、次の6ファイルを追加してください。CMakeの書き方は第5-Rの応用です。

```text
external/imgui/imgui.cpp
external/imgui/imgui_draw.cpp
external/imgui/imgui_tables.cpp
external/imgui/imgui_widgets.cpp
external/imgui/backends/imgui_impl_win32.cpp
external/imgui/backends/imgui_impl_dx11.cpp
```

`external/imgui`をPUBLICなinclude先にし、DX11の必要なリンク先を設定し、`sphere_game`から`sil_imgui`へリンクします。`sphere_game`に`SIL_WITH_IMGUI=1`の定義を追加します。`<imgui.h>`、`<backends/imgui_impl_win32.h>`、`<backends/imgui_impl_dx11.h>`をincludeします。本体と両バックエンドは同じsubmoduleリビジョンを使います。

| 時点 | 呼ぶ関数と注意 |
| --- | --- |
| Renderer作成後 | `IMGUI_CHECKVERSION()`、`ImGui::CreateContext()`、`ImGui::StyleColorsDark()` |
| 入力側の初期化 | `ImGui_ImplWin32_Init(window)`。boolの成功を確認 |
| 描画側の初期化 | `ImGui_ImplDX11_Init(renderer.device(), renderer.context())`。boolの成功を確認 |
| 各フレーム開始 | `ImGui_ImplDX11_NewFrame()`、`ImGui_ImplWin32_NewFrame()`、`ImGui::NewFrame()` |
| UI定義後 | `ImGui::Render()`で描画データを確定 |
| 3D描画後、Present前 | `ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData())` |
| Renderer破棄前 | `ImGui_ImplDX11_Shutdown()`、`ImGui_ImplWin32_Shutdown()`、`ImGui::DestroyContext()` |

**設計課題：** 初期化済みの資源だけを逆順に終了する小さなRAIIクラスを考えてください。途中のInitに失敗してコンストラクターが例外を投げる場合、未完成オブジェクトのデストラクターは呼ばれません。初期化途中の後始末も必要です。まず正常終了、次に途中失敗で、どの終了処理が必要か表にしてください。

#### 実習2：メッセージをバックエンドへ渡す

`WindowState`に`imguiReady = false`を追加し、両バックエンドの初期化成功後だけtrueにします。終了処理の前にfalseへ戻します。

Win32バックエンドのハンドラーは、利用する版のヘッダーで宣言方法を確認してください。利用側の宣言を求める版では、`WindowsMain.cpp`のグローバルスコープに次を置きます。

```cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND window, UINT message, WPARAM wParam, LPARAM lParam);
```

**課題5-T：** `windowProcedure`から、初期化済みの場合だけこの関数へメッセージを渡してください。戻り値が非0なら処理済みとして返せます。作成途中の`WM_NCCREATE`や、アプリが必要とするサイズ・フォーカス状態の処理を失わない順序にしてください。

#### 実習3：覚えた操作からUIを組み立てる

| 関数 | 使い方 |
| --- | --- |
| `ImGui::Begin("Stage selection")` / `End()` | UIウィンドウの開始と終了。BeginがfalseでもEndは必要 |
| `RadioButton(label, &selectedStage, 0)` | 整数の選択値を操作する。最初は選択肢が一つでよい |
| `Button(label)` | そのフレームで押されたらtrue |
| `Text("Remaining: %.1f s", seconds)` | 書式付き表示 |
| `ImGui::GetIO().WantCaptureKeyboard` | UIがキーボード入力を必要としているか |

**課題5-U：** 以下を仕様に、UIと更新ループを実装してください。表示の配置や文言は自分で決めて構いません。

- `int selectedStage = 0`と`bool playing = false`を持つ。起動時にはゲームを進めない。
- Stage 1の選択肢と開始ボタンを出す。開始時は環境と終了結果を初期化し、蓄積時間を0にする。
- プレイ中は選択へ戻る操作を出す。戻ると更新を止める。
- 残り時間は`max(0, (maxSteps - currentStep) * dt)`で求め、直前の終了結果も表示する。
- `playing`のときだけ`step()`を呼ぶ。終了したら結果を記録して即時リセットし、プレイは続ける。
- UIがキーボードを使う間は停止行動を渡す。
- 最小化・非アクティブ時の`continue`より後でNewFrameを始め、開始したフレームは必ずRenderで確定する。

<details>
<summary>ヒント：UIと描画を順番に並べる</summary>

「フレーム開始 → UI作成と入力取得 → 固定更新 → Rendererの3D描画 → ImGui::Render → ImGuiの描画 → Present」です。`WantCaptureKeyboard`はGetAsyncKeyStateを止める機能ではないので、アプリ側で`sil::Direc::Stop`に差し替えます。画面に出す残り時間は、そのフレームの更新前後のどちらを読んだかも観察してください。

</details>

**確認：** 選択中に障害物が進まないこと、UI操作時に球が動かないこと、自動リセット後も直前の終了理由が読めること、3DとGUIが同じ画面に出ることを確かめます。

### 5.8 調べ方と参考資料

API名を見たら、まず「何をするか」「どのオブジェクトの関数か」「入力と出力」「戻り値」「有効な呼び出し時点」を調べます。引数の多い関数は、入力側と出力側に分けてノートへ書いてください。

- [Direct3D 11の初期化](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-initialize)：デバイス、描画先、ビューポート。
- [D3D11CreateDeviceAndSwapChain](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11createdeviceandswapchain)：引数と戻り値。
- [DirectXTK Getting Started](https://github.com/microsoft/DirectXTK/wiki/Getting-Started)：C++で使う公式教材。
- [DirectXTK 3D shapes](https://github.com/microsoft/DirectXTK/wiki/3D-shapes)：形状と行列の使い方。
- [GeometricPrimitive](https://github.com/microsoft/DirectXTK/wiki/GeometricPrimitive)：CreateCube、CreateSphere、Drawの仕様。
- [Dear ImGuiのWin32・DX11例](https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx11/main.cpp)：初期化・メッセージ・終了の順序。丸ごと移植せず、担当箇所を照合する。

**調査課題：** `CreateRenderTargetView`と`OMSetRenderTargets`を調べ、前者はHRESULT、後者はvoidであることを確認してください。「作成」と「命令の設定」の違いも説明します。

### 5.9 この章の完了条件と振り返り

以下は実施する確認であり、本文を書いた時点での成功記録ではありません。

- [ ] Windows x64で`sphere_game`をビルドし、起動・終了できる。
- [ ] 背景色、直方体、球体の順に自分で表示した。
- [ ] デバイス、コンテキスト、画像とビューの違いを説明できる。
- [ ] 球の大きさ・位置、カメラ、画角を変えて、その効果を説明できる。
- [ ] 床、壁、動く障害物、ゴール、プレイヤーが適切な前後関係で表示される。
- [ ] RendererはStageとStateを読むだけで、stepやresetを呼ばない。
- [ ] WASDの9行動、固定時間更新、終了結果の記録、自動リセットが動く。
- [ ] サイズ変更、最小化、フォーカス切替から正常に復帰する。
- [ ] ImGuiで選択・開始・状態表示ができ、入力がゲームと競合しない。
- [ ] GUIと描画資源の寿命、初期化失敗時の後始末を確認した。
- [ ] ヘッドレスと実装済みテストが描画依存なしで動く。未実装のテストは別に記録した。
- [ ] Debugでデバッグレイヤーの警告を調べ、DebugとReleaseで手動操作を確認した。

最後に、空のノートへ「Wを押してから、球が画面で動くまで」を、関数名を交えて書いてください。見ずに説明できなかった部分は、対応する実習へ戻ります。

この最小構成ではデバイスロストの復旧は扱いません。Presentで`DXGI_ERROR_DEVICE_REMOVED`などが返ったら処理名とコードを確認し、例外として終了させます。自作シェーダー、複雑なモデル、音声は次の学習範囲です。

### 5.10 最初は背景色だけを確かめる — 進行表

第5.4節の全実習を一度に終えてから実行する必要はありません。以下の順に、動作する地点を残して進めてください。

| 区切り | 取り組む範囲 | そこで止めて確認すること |
| --- | --- | --- |
| 準備 | 5.1〜5.3、5.6 | 自分のAPI名とファイル配置、依存とターゲットの分離 |
| 背景色 | 5.4の実習1〜4、5.5の実習1〜2 | 色を変えられ、閉じる操作が働く |
| サイズ変更 | 5.4の実習5、5.5の実習3 | 描画先・深度・ビューポートが更新される |
| 直方体・球 | 5.4の実習6〜7 | 大きさ、位置、カメラ、前後関係が意図どおり |
| 手動プレイ | 5.5の実習4〜5 | 入力と固定更新を接続できる |
| 操作画面 | 5.7 | ステージ選択、状態表示、入力競合の防止 |

初期段階では未使用の図形メンバーを持っていても構いませんが、未実装の関数を呼ばないようにします。背景色の段階ではrenderに図形や深度の利用をまだ書かず、実習5を終えてから深度設定を追加します。

| 症状 | 最初に確認すること |
| --- | --- |
| ウィンドウが出ない | RegisterClassExWとCreateWindowExWの失敗、Win32のエラーコード |
| 背景が出ない | HRESULT、描画先の設定、ClearとPresent、描画領域の大きさ |
| 背景だけで図形がない | Drawの呼び出し、行列、カメラ前方と描画範囲、座標系の統一 |
| 球が半分床に埋まる | 中心Yが半径になっているか |
| 奥の物体が手前を覆う | 深度ビューの作成・設定・毎フレームのクリア |
| サイズ変更で失敗する | バックバッファへの古い参照、バインド解除とビューの解放 |
| 障害物の表示が動かない | 初期設定ではなくstate.obstaclesを読んでいるか |
| 復帰直後に時間切れ | 前回時刻とaccumulatorの両方をリセットしているか |

**最後の余白：** 見た目に関する小さな変更を一つ、自分で考えて実装してください。例は「障害物だけ背を高くする」「カメラの注視点をステージ中央へ寄せる」です。変更する責務、使う関数、予想する表示を書いてから実装し、シミュレーションの結果が変わらないことも確かめてください。


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

第5章は2026年10月21日にワークブック形式へ改稿した。章構成、演習の前提・参照・コードフェンスを静的に確認した。空欄は学習者が実装する課題であり、完成プログラムとしてのビルドやWindowsでの描画検証は行っていない。上記の2026年10月20日の検証記録は、改稿後の第5章の動作保証を表すものではない。
