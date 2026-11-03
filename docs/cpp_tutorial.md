# sphere is learning — C++とDirect3D 11で作るゲーム開発入門

## 目次

- [この教材の使い方](#guide)
- [10/26までに手動プレイを完成させる日別計画](#manual-play-plan)（完了）
- [11/4〜11/12 AI学習から提出までの日別計画](#ai-plan)
- [第1章 ビルドして動かす](#chapter-1)
- [第2章 ゲームに必要なC++の読み方](#chapter-2)
- [第3章 描画から独立したゲームを作る](#chapter-3)
- [第4章 テストでルールを確かめる](#chapter-4)
- [第5章 WindowsとDirect3D 11](#chapter-5)
- [第6章 周辺を観測して行動を決める](#chapter-6)
- [第7章 遺伝的アルゴリズムで行動表を育てる](#chapter-7)
- [第8章 保存・読み込み・CLIを作る](#chapter-8)
- [第9章 検証と作品への仕上げ](#chapter-9)
- [付録 サンプルCLI仕様](#cli)
- [参考資料と検証記録](#references)

<a id="guide"></a>

## この教材の使い方

Wキーを押すと球が進む。この短い動作にも、入力を読む処理、位置を変える処理、画面を描く処理が必要です。位置を変える処理を独立させると、キーボードの代わりにAIから同じゲームを操作できます。これが本作の出発点です。

[実装TODO](todo.md)の手順2以降を、実際に書くコードへ落とし込みます。WindowsへのcloneはTODOの手順1を先に済ませてください。C++の文法をすべて覚える必要はありません。各章のコードを動かし、値を変え、結果を予想して確かめます。

このファイルは教材です。リポジトリの`src/`、`tests/`、`CMakeLists.txt`へ実装を配置したものではありません。本文で「完全例」としたコードブロックは、直前のファイル名へ保存する内容です。「差し込み例」は指定した場所へ追加・置換します。第5章は、定型的な基盤をコピーして使い、ゲームによって変わる処理を自分で書く構成です。「コピー可」「理解して使う」「自分で書く」の指針に従ってください。自分で書く箇所の骨組みには未完成のものがあります。第6〜8章のAI部分は、関数名・型・引数・手順だけを説明し、コードは自分で書く構成です。CLIの入口、CMake、保存の確認用の差し込み例だけをコード付きで示します。

### TODOとの対応

| TODO | 教材 | 最初に確認する結果 |
| --- | --- | --- |
| 手順2 | 第1章 | コマンドから実行できる |
| 手順3〜5 | 第2〜4章 | 画面なしで移動・衝突・終了判定ができる |
| 手順6〜8 | 第5章 | Windowsで1ステージを遊べる |
| 手順9 | 第6章 | 周辺8マスを8バイトの観測と行動表の番号へ変換できる |
| 手順10 | 第7〜8章 | 世代更新と保存済みAIの再実行ができる |
| 手順11 | 第9章 | 比較条件と実測結果を説明できる |

### 最後にそろうファイル

以下は学習を進めた後の配置です。一度に全部作る必要はありません。各章で指定されたファイルだけを追加します。AIのファイルは、第6章では`src/ai/Policy`にまとめて書き、第7章7.1で役割ごとのフォルダ（`common/`、`table/`、`ga/`）へ分け直します。

```text
CMakeLists.txt
src/
  core/Simulation.hpp, Simulation.cpp
  app/HeadlessMain.cpp, WindowsMain.cpp, ExperimentMain.cpp
  renderer/d3d11/Renderer.hpp, Renderer.cpp
  ai/common/Observation.hpp, Observation.cpp, Episode.hpp
  ai/table/ActionTable.hpp, ActionTable.cpp, TableFile.hpp, TableFile.cpp
  ai/ga/GeneticTrainer.hpp, GeneticTrainer.cpp
tests/
  SimulationTests.cpp, AiTests.cpp
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
| 観測 | 1マス1.5、自分を除く周辺8マス。各マスは空き・壁・障害物・ゴールのいずれかを1バイトで表す |
| AI | 6ステップごとに行動決定。観測の番号（4⁸＝65,536通り）から行動を引く行動表を、遺伝的アルゴリズムで育てる |
| 初期ステージ | コンパイル時に作る設定。外部ステージ読込は追加課題 |

<a id="manual-play-plan"></a>

## 10/26までに手動プレイを完成させる日別計画

**この計画は10/24深夜に第5章までの実装を終えて完了しました。以降は[AI学習から提出までの日別計画](#ai-plan)に従います。** 以下は記録として残します。

2026年10月21日（水）から10月26日（月）までの6日間で、**Direct3D 11の画面を見ながら、WASDで1ステージを遊べる状態**を目指します。対象は第1〜5章と第9章9.2の人間操作に関する確認です。第6〜8章のAI観測・学習・保存は10/27以降に進めます。

### 必要な時間の目安

**合計24時間、1日平均4時間**を確保する計画です。内訳は実装・理解・動作確認に約20時間、環境構築や不具合対応の予備に約4時間です。従来の週15時間より多く、この6日間は約9時間の上積みが必要になります。週15時間のペースでは6日間で約13時間のため、今回の期限に向けた余裕は小さくなります。

これは、既存のシミュレーションを使い、教材のサンプルを読みながら自分の実装へ合わせる場合の計画上の見積もりです。実測した所要時間や完成の保証ではありません。第5章は10/22に、定型部分の自力実装を減らす構成へ改稿しました。所要時間は再計測していないため、この24時間を保証値とせず、動作した区切りで残作業を見直してください。Direct3DやCMakeの問題が長引いた場合は、下記の到達点で遅れを判断します。

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

<a id="ai-plan"></a>

## 11/4〜11/12 AI学習から提出までの日別計画

11/4の再計画です。予定の都合で実装の再開を11/4（水）とし、ES提出期限は11/12（木）へ前倒しになりました。以前の「10/25〜11/16」の計画は、第6章から8.4節までが予定より早く進んだため、この計画で置き換えます。以前の計画の内容はgitの履歴で確認できます。

### 11/4時点の進み具合

ユーザーの報告とコミット履歴から整理したものです。Windowsでの動作は、この文書の作成時には確認していません。

| 範囲 | 状態 | 根拠 |
| --- | --- | --- |
| 第6章 観測・行動 | 完了 | 10/26に完了と報告 |
| 第7章 遺伝的アルゴリズム | 完了 | 10/26「GAのロジックを実装」、同日に7.1の再構成 |
| 8.2〜8.4 保存・CLI | 完了 | `GenomeFile`、`AgentMain.cpp`の`train`・`evaluate` |
| 障害物の種類の追加（`std::variant`） | 完了（計画外の追加） | 10/28〜10/29のコミット |
| 8.6 最終世代のAIのデモ再生 | 着手 | 10/30に`aiDemoMain.cpp`を作成 |
| 第9章 計測・比較 | 未着手 | — |
| A\*による経路探索 | 未着手（追加目標） | 教材に章はない。下の「A\*の範囲」を参照 |

### 前提と時間の目安

| 項目 | 内容 |
| --- | --- |
| 期間 | 11/4（水）〜11/11（水）の8日間。11/12はES提出日で、最終確認だけを行う |
| 1日の時間 | 平日2時間、土日5時間、11/11は3時間 |
| 合計 | 約23時間 |
| 必須成果 | 1ステージで学習によりプレイが改善する様子を示す／学習結果を保存する／保存したAIを画面で動かす |
| 追加目標 | A\*で求めた経路を画面で再生する |
| 優先順位 | 必須成果 → 提出用の素材（CSV・学習曲線・録画） → A\* |
| 後回し | ステージクリア、複数ステージ、学習の中断・再開、ニューラルネット、手法の網羅的な比較 |

時間は見積もりで、実測ではありません。平日に時間が取れない日は、その日の作業を次の土日へ回します。それでも足りない場合は、優先順位の低いものから削ります。必須成果と提出素材は11/7までにそろえ、A\*で詰まっても提出物に影響しないようにします。

### 日ごとの作業と、その日の完了条件

| 日付 | 時間 | 作業（教材） | その日の完了条件 |
| --- | --- | --- | --- |
| 11/4（水） | 2時間 | 8.6：保存ファイルを読み込み、学習時と同じシードで`GAAgent`に操作させる。終了したら同じ条件で最初から繰り返す | 指定した保存ファイルのAIが画面で動き、終了後に自動で再開する |
| 11/5（木） | 2時間 | 8.6の残り：`results/`から最終世代を自動で選ぶ。ImGuiにファイル名・世代・保存時の成績・現在の到達距離を表示する。9.2の「保存済みAI」の行を確認する | **判断日①**：必須成果の3つの仕組みがそろう |
| 11/6（金） | 2時間 | 9.1：Releaseビルドで評価時間と学習時間を測る。その値から世代数を決め、`train`の出力をCSVへ保存する学習を寝る前に実行する | 計測値（CPU・構成・世代数・時間）を記録し、学習を開始できている |
| 11/7（土） | 5時間 | 前半：学習結果を確認し、改善が小さければ下の「調整の順番」に従って再実験する。9.3：初期・途中・最終世代の代表個体を決め、同じ条件で比較する。**デモ動画とGIFの素材を録画する**。後半：A\*の設計（`Environment`のコピーの確認と、下の表の各項目をメモに決める） | **判断日②**：学習曲線のCSV、代表個体の保存ファイル、録画素材がそろう |
| 11/8（日） | 5時間 | A\*をヘッドレスで実装し、ゴールまでの行動列を求める | **判断日③**：A\*の行動列をヘッドレスで再生し、ゴールに到達する |
| 11/9（月） | 2時間 | A\*の行動列をファイルに保存するか、デモの起動時に探索する。デモ画面で再生し、録画する | A\*の経路が画面で動き、録画素材がそろう |
| 11/10（火） | 2時間 | **機能凍結**。9.2の画面の最終確認、Debug・Releaseのビルド、全テスト。GAとA\*の結果を同じ条件で記録する（到達距離、ゴールの有無、ステップ数）。READMEの骨組みを書く | 以後は不具合修正と文書だけにする |
| 11/11（水） | 3時間 | ポートフォリオのまとめ：README（冒頭のGIF、一文の説明、学習曲線、工夫した点）、資料のページ、動画の編集、GitHubへのpush | ES・ポートフォリオに載せる文章と動画のリンクが確定する |
| 11/12（木） | — | ES提出。READMEのリンクと動画の再生だけを最終確認する | — |

ESの本文（作品の説明、力を入れたこと）は、11/11を待たずに11/7〜11/9の空き時間で下書きしておきます。11/11の作業を、文章の手直しと素材の配置だけにするためです。

**調整の順番（1回に1つだけ変える）。** 変えた項目と結果は、毎回CSVのファイル名とメモに残します。

1. 世代数を増やす。9.1の計測結果から実行時間を見積もってから増やす。
2. 突然変異率（0.01→0.005や0.02）。エリートの成績の変化を見て判断する。
3. ステージの難度（障害物の速度・位相・本数）。初期集団でも少し進め、途中で詰まる程度にする。
4. マス幅（1.5→1.0など）。変えると以前の保存ファイルは読めなくなる。
5. 報酬の重み。変えた実験同士の適応度は比べず、到達距離で比べる。

保存ファイルの読み込みに関わる設定を変えると、古い保存ファイルは読み込みで失敗します。これは意図した動作です。比較に使う個体は、最後に同じ設定でまとめて作り直します。

時間のかかる学習は寝る前や外出前に実行しておき、翌日の作業時間に結果を確認すると、平日の短い時間を有効に使えます。

### A\*の範囲

A\*には教材の章がありません。ここでは方針だけを示し、コードは自分で書きます。置き場所は`src/ai/search/`です。`src/ai/common/`の部品は使えますが、`src/core`から`ai/`をincludeしてはいけません。

このステージでは障害物が動き、`Chase`はプレイヤーの位置に反応します。そのため、マス目の迷路として静的に解くのではなく、**`Environment`をコピーして先読みする、時間付きのA\***にします。シミュレーターそのものを予測に使うので、衝突判定と障害物の動きをそのまま正しく扱えます。

| 項目 | 方針 |
| --- | --- |
| ノード | `Environment`のコピーと、そこへ至る行動（親ノードへの参照など） |
| 展開 | 9行動それぞれについてコピーを作り、GAと同じ6ステップの間隔で`step()`を進める。衝突・時間切れになった子は捨てる |
| コスト | 経過ステップ数 |
| ヒューリスティック | ゴール領域までの直線距離を、1ステップの移動量で割った値。実際のステップ数を超えて見積もらないようにする |
| 重複判定 | 位置をマスに丸めた値と、経過ステップの区切りの組 |
| 打ち切り | 展開したノード数の上限を決め、超えたら「見つからない」と返す |

最初に、「`Environment`をコピーして同じ行動列を与えると、元と同じ結果になる」ことをテストで確かめます。A\*全体がこの性質に依存しているためです。

A\*は、障害物の動きまで含めた全情報を使って探索します。一方のGAは、周辺8マスの観測だけで行動を決めます。そのため、2つの強弱を比べるのではなく、「全情報があれば解ける経路に、観測だけで学習したAIがどこまで近づけるか」という位置づけで結果をまとめます。

### 遅れたときの調整

| 判断日 | 未達の場合 |
| --- | --- |
| ①11/5 | 11/6も8.6に使う。最終世代の自動選択とImGuiの表示は省き、ファイル名を固定して読み込む。学習は寝る前に実行しておく |
| ②11/7 | A\*を取りやめ、11/8を学習の調整と録画に回す。それでも改善が見えなければ、障害物を減らすなどステージを簡単にし、「改善する様子を示す」ことを優先する |
| ③11/8 | A\*を打ち切る。README・資料では「今後の展望」として触れ、11/9以降は仕上げに回す |
| 11/10 | 機能追加をやめる。未完成の項目は「未実装」とREADMEに明記して提出する |

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

数が固定された8マスの観測には`std::array<Cell, 8>`、ステージによって数が違う壁には`std::vector<Rect>`を使います。どちらも自分のデータを所有し、寿命が終わると解放します。最初から`new`や`delete`を使う必要はありません。

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

## 第5章 WindowsとDirect3D 11 — 基盤を使い、ゲームの描画と動作を作る

この章の目標は、Direct3D 11を使ってゲームの見せ方と動作を実装できることです。初期化APIの全引数やWin32の手続きを、資料なしで再現することは求めません。背景色、直方体、球体、ゲーム、GUIの順に動く範囲を広げます。第5.6節のビルド準備を先に接続し、第5.10節の進行表で実行する区切りを確認してください。

対象はWindows 11 x64、MSVC、C++20、Windows SDK、Direct3D 11、DirectXTKです。macOSやLinuxではこの描画アプリを検証できません。古いDirectX SDKやD3DXは使用しません。

**実装済みの5-Hまでは、そのまま使います。** 5-A〜5-Hの本文と課題番号は残しています。追加のノートや書き直しは不要です。これから進む5-I以降では、自力実装を次のように絞ります。

| 指針 | 取り組み方 | 該当箇所 |
| --- | --- | --- |
| **コピー可** | 指定箇所へ貼り付けて使う。役割・呼ぶ時点・失敗の確認方法を押さえれば進んでよい | 5-Iの深度とサイズ変更、5-M〜5-OのWin32、5-RのCMake、5-S〜5-TのImGui連携。既習の5-D〜5-Hも再現練習は不要 |
| **理解して使う** | 完成例を使い、ゲームに影響する順序と設定を説明できるようにする。APIや定型ループをゼロから組み直さない | 図形の生成、固定時間更新の枠組み、GUIを描く順序 |
| **自分で書く** | 仕様から計算や条件分岐を書く。分からないAPIの呼び方は例や資料を見てよい | 5-J〜5-Lのカメラ・座標変換・ステージ描画、5-Pの入力変換、5-Qの1ステップと再開、5-UのUIとプレイ状態 |

「コピー可」は、表示やゲームルールに関係する変更が必要になった時に調べ直せれば十分、という範囲です。定数の暗記、COMや呼び出し規約の詳細、全引数のノート作成、複数描画先などの発展調査は必須にしません。エラーを無視することとは区別し、失敗コードとデバッグレイヤーの警告は確認します。

**取り組み方：** コピーする基盤を先につなぎ、小さく実行してから、自分で書く部分へ進みます。各課題で未知の型やAPIを推測して埋める必要はありません。`#error`付きの骨組みは未完成です。コード断片は指定した関数へ組み込み、置換と指定した箇所だけを置き換えます。

記録は各課題ではなく、5.10の動作確認の区切りごとに一度で十分です。下の詳細な記録欄は任意で、通常は「動いたこと／未解決／次の作業」の3行だけ残します。

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

#### 5-Hまでのコードを、最初の実行へつなぐ（コピー可）

既に書いた初期化・描画処理はそのまま使います。最初に背景色を表示するため、未実装なら次のコンストラクターをRenderer.cppのnamespace sil::renderer内へ追加します。ヘッダーのwidth_・height_はそれぞれ = 0 で初期化し、viewport_はComPtrではなく D3D11_VIEWPORT viewport_{}; とします。

```cpp
Renderer::Renderer(HWND window, std::uint32_t width, std::uint32_t height)
{
    createDeviceAndSwapChain(window);
    createSizeDependentResources(width, height);
    width_ = width;
    height_ = height;
}
```

present()が戻り値を無視している場合は、次の形にします。throwIfFailedの処理名の引数は const char* operation です。エラー文では文字列 "hex" ではなく std::hex を使います。

```cpp
void Renderer::present()
{
    if (width_ == 0 || height_ == 0) {
        return;
    }
    throwIfFailed(swapChain_->Present(1, 0), "Present");
}
```

**次は5.6のコピー用CMakeと、5.5の実習1〜2へ進みます。** 先に背景色の表示・変更・終了を確認し、その後に以下の5-Iへ戻ります。単体の初期化関数は、まだウィンドウから呼ばれていなければ実行確認できません。

#### 実習5：深度とサイズ変更を扱う（課題5-I：コピー可）

深度バッファは、各ピクセルでどの物体が手前かを判断するための画像です。ウィンドウサイズが変われば、描画先や深度画像の大きさも合わせます。ここは完成例を使い、「サイズ変更時に古い参照を外して作り直す」「描画時に深度を設定・クリアする」の二点を押さえます。全フィールドを資料なしで埋める課題はありません。

**差し込み例：** 5-Gで書いたcreateSizeDependentResources()の末尾へ追加します。既存のバックバッファ取得・ビュー作成・viewport_の設定は残してください。

```cpp
D3D11_TEXTURE2D_DESC depthDescription{};
depthDescription.Width = width;
depthDescription.Height = height;
depthDescription.MipLevels = 1;
depthDescription.ArraySize = 1;
depthDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
depthDescription.SampleDesc.Count = 1;
depthDescription.Usage = D3D11_USAGE_DEFAULT;
depthDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;

throwIfFailed(device_->CreateTexture2D(&depthDescription, nullptr, depthTexture_.GetAddressOf()), "Create depth texture");
throwIfFailed(device_->CreateDepthStencilView(depthTexture_.Get(), nullptr, depthStencilView_.GetAddressOf()), "Create depth view");
```

**完成例：** Renderer.cppのnamespace sil::renderer内へresize()を追加します。メッセージから直接呼ばず、5.5のメインループから呼ぶ形にします。

```cpp
void Renderer::resize(std::uint32_t width, std::uint32_t height)
{
    width_ = width;
    height_ = height;
    if (width == 0 || height == 0) {
        return;
    }

    if (renderTargetView_) {
        context_->OMSetRenderTargets(0, nullptr, nullptr);
        renderTargetView_.Reset();
        depthStencilView_.Reset();
        depthTexture_.Reset();
        throwIfFailed(swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0), "ResizeBuffers");
    }
    createSizeDependentResources(width, height);
}
```

**置換：** 最初の実行用に追加したコンストラクターを、次へ置き換えます。二つ定義しないでください。

```cpp
Renderer::Renderer(HWND window, std::uint32_t width, std::uint32_t height)
{
    createDeviceAndSwapChain(window);
    resize(width, height);
}
```

**差し替え：** render()内のOMSetRenderTargetsの第3引数を深度ビューへ変更し、背景色のクリアの隣に深度のクリアを追加します。viewsは5-Hで作った1要素の配列です。GetAddressOf()で書いている場合も、第3引数だけ同様に変更できます。

```cpp
context_->OMSetRenderTargets(1, views, depthStencilView_.Get());
context_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0F, 0);
```

D3D11の既定の深度判定を使います。深度を毎フレーム1.0へ戻し、手前の図形を優先して書き込みます。背景色のクリア・viewport_の設定・幅と高さのガードは残してください。

**必須確認は一つ：** サイズ変更・最小化・復帰を行い、背景表示へ戻れることを確かめます。深度の前後関係は、図形を出した後に5-K〜5-Lとまとめて確認します。バックバッファをComPtrのメンバーへ追加保存するとResizeBuffersを妨げるため、5-GのbackBufferはローカル変数のままにします。



#### 実習6：カメラと座標変換を理解する（課題5-J：自分で書く）

ここからはゲームの見せ方によって変わる部分です。APIの名前や行列の型は資料を使って構いません。カメラの位置・注視点・画角を選び、数値と表示の関係を考えることに時間を使います。予想は口頭や頭の中で行えばよく、解答ノートは不要です。

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

#### 実習7：直方体からステージへ広げる（課題5-K・5-L：自分で書く）

DirectXTKの`GeometricPrimitive`は形状と描画処理をまとめたクラスです。生成部分はコピー可です。5-Iのコンストラクターで`resize(width, height);`の直後に次を追加し、毎フレームは使い回します。render()の中で生成し直しません。

```cpp
cube_ = DirectX::GeometricPrimitive::CreateCube(context_.Get(), 1.0F, false);
sphere_ = DirectX::GeometricPrimitive::CreateSphere(context_.Get(), 1.0F, 24, false);
```

立方体の引数はコンテキスト、一辺の長さ、右手系で生成するかです。球はコンテキスト、直径、分割数、右手系で生成するかです。`false`で本章の左手系へ合わせます。基準の形はどちらも大きさ1で作り、実際の大きさは描画時の拡大で決めます。

描画呼び出しは`cube_->Draw(world, view, projection, color)`です。色は`DirectX::Colors::SlateGray`などを使えます。

**課題5-K：** まず幅2、高さ1、奥行き2の直方体を、中心`(0, 0.5, 0)`へ描いてください。render()の背景・深度のクリアより後へ描画を追加します。表示できたworld行列の計算を、下の補助関数へ移して使い回します。二通りの実装を残す課題ではありません。

```cpp
// Renderer.cppの無名名前空間へ置く。引数の型はそのまま使ってよい。
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

**課題5-L：応用部分は自分で書く。** 次の表を仕様として、`render()`の残りを実装してください。引数名を省略していた場合は、定義を`render(const Stage& stage, const State& state)`へ戻します。床・ゴール・壁・障害物には同じdrawRect()を使い、物体ごとに別の描画クラスを作る必要はありません。

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

この節は「Win32の基盤をコピーする」「ゲームへつなぐ二つの関数を自分で書く」に分けます。入口、メッセージの配送、ウィンドウへの状態の受け渡しを空欄から組み立てる課題はありません。

#### 実習1〜2：背景色を表示する入口（課題5-M・5-N：コピー可）

**完成例：src/app/WindowsMain.cpp。** 現在の空のmain()を含む内容を、このファイルに限って置き換えられます。既存のゲーム用入力や更新を実装済みの場合は、下の二つの関数へ移してください。Renderer.hppのパスは現在のsrc/renderer/directX11/に合わせています。

コピー前に5.6のCMake、5-H直後のコンストラクターを接続します。初回はゲームを進めない二つの仮関数を使います。render()もまだ背景色だけにし、以後はこのファイルを丸ごと書き直さず、指定箇所だけ変更します。

<details>
<summary>コピー用：WindowsMain.cppの基盤（Win32の細部は読み飛ばしてよい）</summary>

```cpp
#include <windows.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include "core/Simulation.hpp"
#include "renderer/directX11/Renderer.hpp"

namespace {
struct WindowState {
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    bool resizePending = false;
    bool minimized = false;
    bool hasFocus = true;
    bool resetClock = false;
};

// Rendererより後までウィンドウを生かし、例外時にも閉じる。
struct WindowLifetime {
    HWND window;
    ~WindowLifetime() {
        if (IsWindow(window)) {
            DestroyWindow(window);
        }
    }
};

// 5-Pで本体を置き換える。
sil::Direc readAction() {
    return sil::Direc::Stop;
}

// 5-Qで引数名と本体を補う。今は何もしないので背景確認に使える。
void advanceGame(sil::Environment&, sil::Direc, sil::GameState&, std::uint32_t) {
}

LRESULT CALLBACK windowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* state = reinterpret_cast<WindowState*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    switch (message) {
    case WM_NCCREATE: {
        const auto* create = reinterpret_cast<const CREATESTRUCTW*>(lParam);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
        return TRUE;
    }
    case WM_SIZE:
        if (state) {
            state->width = LOWORD(lParam);
            state->height = HIWORD(lParam);
            state->minimized = wParam == SIZE_MINIMIZED;
            state->resizePending = !state->minimized;
            state->resetClock = true;
        }
        return 0;
    case WM_ACTIVATEAPP:
        if (state) {
            state->hasFocus = wParam != FALSE;
            state->resetClock = true;
        }
        return 0;
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int showCommand)
{
    HWND window = nullptr;
    try {
        constexpr wchar_t className[] = L"SphereIsLearningWindow";
        WNDCLASSEXW windowClass{};
        windowClass.cbSize = sizeof(windowClass);
        windowClass.hInstance = instance;
        windowClass.lpszClassName = className;
        windowClass.lpfnWndProc = windowProcedure;
        windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        if (!RegisterClassExW(&windowClass)) {
            const DWORD error = GetLastError();
            throw std::runtime_error("RegisterClassExW: " + std::to_string(error));
        }

        WindowState windowState;
        window = CreateWindowExW(0, className, L"sphere is learning", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, nullptr, nullptr, instance, &windowState);
        if (!window) {
            const DWORD error = GetLastError();
            throw std::runtime_error("CreateWindowExW: " + std::to_string(error));
        }
        WindowLifetime windowLifetime{window};

        RECT client{};
        if (!GetClientRect(window, &client)) {
            const DWORD error = GetLastError();
            throw std::runtime_error("GetClientRect: " + std::to_string(error));
        }
        windowState.width = static_cast<std::uint32_t>(client.right - client.left);
        windowState.height = static_cast<std::uint32_t>(client.bottom - client.top);
        windowState.resizePending = false;
        sil::renderer::Renderer renderer(window, windowState.width, windowState.height);

        constexpr std::uint32_t initialSeed = 42;
        sil::Environment env{sil::makeStage()};
        env.reset(initialSeed);
        sil::GameState lastEnd = sil::GameState::Running;
        using Clock = std::chrono::steady_clock;
        auto previousTime = Clock::now();
        double accumulator = 0.0;
        bool running = true;
        ShowWindow(window, showCommand);

        while (running) {
            MSG message{};
            while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
                if (message.message == WM_QUIT) {
                    running = false;
                    break;
                }
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            if (!running) {
                break;
            }

            // 5-Iに進んだら、この位置へ保留中のサイズ変更処理を追加する。
            const auto now = Clock::now();
            if (windowState.resetClock || windowState.minimized || !windowState.hasFocus) {
                previousTime = now;
                accumulator = 0.0;
                windowState.resetClock = false;
            }
            if (windowState.minimized || !windowState.hasFocus) {
                Sleep(10);
                continue;
            }
            const double elapsed = std::min(std::chrono::duration<double>(now - previousTime).count(), 0.25);
            previousTime = now;

            // GUIを追加するときは、この位置でNewFrameとUI作成を行う。
            accumulator += elapsed;
            while (accumulator >= sil::Environment::dt) {
                advanceGame(env, readAction(), lastEnd, initialSeed);
                accumulator -= sil::Environment::dt;
            }

            renderer.render(env.stage(), env.state());
            // GUIを追加するときは、この位置でGUIを描画する。
            renderer.present();
        }
    } catch (const std::exception& error) {
        MessageBoxA(nullptr, error.what(), "sphere is learning", MB_OK | MB_ICONERROR);
        return 1;
    }
    return 0;
}
```

</details>

**ここで一度だけ実行します。** 背景色が出る・色を変えられる・閉じられる、の三点を確認します。ゲームが動かないのは仮関数のためです。初期版ではサイズ変更の反映もまだ行いません。

入口の各型やCreateWindowExWの全引数は、覚えずに使って構いません。「OSの通知を処理するループがあり、その外で更新と描画を行う」「ウィンドウの実寸をRendererへ渡す」ことを押さえます。WM_QUIT後に描画へ進まない点も残してください。

#### 実習3：サイズ変更と一時停止（課題5-O：コピー可）

先に5-Iのresize()と深度処理を追加します。その後、上のコメント「保留中のサイズ変更処理」の位置へ、次を追加します。

```cpp
if (windowState.resizePending && !windowState.minimized) {
    renderer.resize(windowState.width, windowState.height);
    windowState.resizePending = false;
}
```

WM_SIZEやWM_ACTIVATEAPPの受け取り、最小化・非アクティブ中の休止は完成例に入っています。自力で書き直す必要はありません。止まっていた時間をゲームへ一度に加算しないよう、復帰時にpreviousTimeとaccumulatorの両方を戻していることだけ確認します。

**確認：** サイズ変更、最小化から復帰、別アプリから復帰を一度ずつ行います。背景色と、図形追加後の画面が復帰すれば次へ進みます。

#### 実習4：WASDから既存の行動へ変換する（課題5-P：自分で書く）

キーを調べるWin32の書き方は、そのままコピーします。WindowsMain.cppの無名名前空間へ追加します。

```cpp
bool isKeyDown(int key) {
    return (GetAsyncKeyState(key) & 0x8000) != 0;
}
```

これは「今押されているか」をboolで返します。押した瞬間の通知とは用途が違います。0x8000というビットの暗記は不要です。

**自分で書く範囲：** 既にあるreadAction()の本体だけを変更します。

- horizontalはDの押下値からAの押下値を引く。
- verticalはWの押下値からSの押下値を引く。
- sil::direcFromAxes(horizontal, vertical)を返す。

boolは整数の計算ではfalseが0、trueが1として使えます。分かりにくければ static_cast<int>(isKeyDown('D')) のように明示して構いません。反対キーの同時押しを相殺する処理は自分で組み立てます。斜め速度の補正はシミュレーション側に残します。

**確認は実行でまとめる：** 5-Qを終えてから、W、W+D、W+Sでそれぞれ前進、斜め移動、停止になることを確かめます。別の解答表の作成は不要です。

#### 実習5：ゲームの1ステップと再開（課題5-Q：理解して使う＋自分で書く）

時計の読み取り、accumulatorへの加算、dtごとのwhileループは完成例を使います。固定更新の仕組み自体はゲームにもAIにも重要なので、次の流れを読んで理解します。

```text
現実に経過した時間をaccumulatorへ加える
    → dt以上たまっている間、ゲームを1回更新してdtを引く
    → 余りの時間は次のフレームへ持ち越す
    → 更新後の状態を1回描画する
```

描画が速ければ更新0回、遅ければ複数回になる場合があります。whileをifへ変えると、遅い描画にゲーム速度が引きずられます。この意味は押さえますが、chronoの型や定型コードをゼロから再実装する必要はありません。elapsedの0.25秒上限は大量更新を避けるための方針で、その上限を超える停止では現実よりゲーム内時間が遅れます。

**自分で書く範囲：** 仮のadvanceGame()だけを、次の引数名付きの形へ置き換えます。本体は数行です。既存の更新ループは変更しません。

```cpp
void advanceGame(sil::Environment& env, sil::Direc action,
                 sil::GameState& lastEnd, std::uint32_t initialSeed)
{
    // TODO: env.step(action)の結果を受け取る。
    // TODO: result.endがsil::GameState::Running以外なら、
    //       lastEndへ終了理由を保存してからenv.reset(initialSeed)する。
}
```

終了理由の保存をreset()より先にする理由、描画からstep()を呼ばない理由は説明できるようにします。キーの代わりにAIの行動を渡せば、同じ更新関数が使える構成です。

後の第8.5節で保存済みAIをつなぐときは、この版では固定更新の中の`advanceGame(...)`呼び出しを、その節の更新・自動再開コードへ置き換えます。両方を呼ぶと二重更新になるので、どちらか一方にします。

**必須確認：** WASDで移動できること、障害物が動くこと、衝突やゴール・時間切れの後に即座に再開することを確認します。通常の描画負荷の変化で移動速度が大きく変わらず、別アプリから戻って即時間切れにならないことも確認します。既に試したサイズ変更の記録を重ねて書く必要はありません。



### 5.6 依存ライブラリとCMakeを接続する（課題5-R：コピー可）

**最初の実行より前に行います。** 設定を空欄から作る課題はありません。ターゲットに何を入れるかだけ確認し、完成例を使います。

| ターゲット | 所属するもの |
| --- | --- |
| sphere_core | シミュレーションだけ。Windowsや描画に依存させない |
| sphere_headless | ヘッドレス用の入口とcore |
| sphere_tests | テスト用の入口とcore |
| sphere_game | Windows用の入口、Renderer、描画ライブラリ |
| sphere_hello（残す場合） | Hello.cppだけ |

**現在のCMakeを整理するための完成例：CMakeLists.txt。** この教材範囲の最小構成です。既に独自ターゲットや設定を追加している場合は、それらを消さずに上の所属へ合わせます。各ターゲットへ全cppを追加しないでください。入口が重複し、ヘッドレスにも描画依存が混ざるためです。描画パスは現在のsrc/renderer/directX11/です。ファイル移動は不要です。

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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/app/Hello.cpp")
    add_executable(sphere_hello src/app/Hello.cpp)
    target_compile_features(sphere_hello PRIVATE cxx_std_20)
endif()

option(BUILD_TESTING "Build the tests" ON)
if(BUILD_TESTING)
    enable_testing()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/SimulationTests.cpp")
        add_executable(sphere_tests tests/SimulationTests.cpp)
        target_link_libraries(sphere_tests PRIVATE sphere_core)
        add_test(NAME simulation COMMAND sphere_tests)
    else()
        message(STATUS "Simulation tests are not implemented yet")
    endif()
endif()

if(WIN32)
    set(BUILD_TOOLS OFF CACHE BOOL "Build DirectXTK auxiliary tools" FORCE)
    set(BUILD_XAUDIO_WIN8 OFF CACHE BOOL "Build DirectXTK audio" FORCE)
    add_subdirectory(external/DirectXTK)

    add_executable(sphere_game WIN32
        src/app/WindowsMain.cpp
        src/renderer/directX11/Renderer.cpp
        src/renderer/directX11/Renderer.hpp)
    target_include_directories(sphere_game PRIVATE src)
    target_compile_definitions(sphere_game PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    target_link_libraries(sphere_game PRIVATE sphere_core DirectXTK user32 d3d11 dxgi)
    if(MSVC)
        target_compile_options(sphere_game PRIVATE /W4 /permissive- /utf-8)
    endif()
endif()
```

テストの条件分岐は、未実装でも構成できるためのものです。未実装のテストが成功した扱いにはしません。

登録済みのDirectXTK・ImGui submoduleを取得します。git submodule addを重ねる必要はありません。

```powershell
git submodule update --init --recursive
```

Visual StudioのDeveloper PowerShellで、プロジェクト根から以下を実行します。既存のbuild/と混ぜず、build-workbook/を使います。ローカルの.git/info/excludeに /build-workbook/ を一度追加し、生成物をコミット対象から外します。Visual Studioから開く場合はルートのCMakeLists.txtがあるフォルダーを開き、構成を更新してsphere_gameを起動対象にします。

```powershell
cmake -S . -B build-workbook -A x64
cmake --build build-workbook --config Debug --target sphere_game
.\build-workbook\Debug\sphere_game.exe
```

「ヘッダーが見つからない」はinclude先、「未解決シンボル」は定義やリンク先、「起動後のHRESULT失敗」は実行時の設定を調べます。全APIの調査ノートは不要です。Debugレイヤー不足ならWindowsのオプション機能Graphics Toolsを確認します。

**このPCでCMakeが終了コード`-1073740791`で止まる場合：** 日本語を含む作業ディレクトリとCMakeの組み合わせによる異常終了を確認しています。プロジェクト直下から`.\scripts\build.ps1`を実行すると、内部で作業ディレクトリを切り替えて、テストを有効にしたまま`build`へ生成・ビルドします。実行は`.\build\Debug\sphere_game.exe`です。テストも実行するときは`-RunTests`を付けます。[原因・検証結果と詳しい手順](build-troubleshooting.md)を参照してください。

ゲームが動いた区切りで、sphere_headlessと実装済みのsphere_testsもビルド・実行します。描画準備のたびに同じ確認を重ねる必要はありません。



### 5.7 Dear ImGuiで操作画面を作る

手動操作が動いてから追加します。バックエンドの接続と後始末はコピーし、開始・停止・表示内容だけ自分で書きます。順序は「UI作成 → ゲーム更新 → 3D描画 → GUI描画 → Present」です。

#### 実習1：ビルドと寿命（課題5-S：コピー可）

**差し込み例：** 5.6のif(WIN32)内、sphere_gameの定義後へ追加します。既存のImGui submoduleとWindowsのシステムライブラリを使います。

```cmake
add_library(sil_imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_win32.cpp
    external/imgui/backends/imgui_impl_dx11.cpp)
target_include_directories(sil_imgui PUBLIC external/imgui)
target_compile_definitions(sil_imgui PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
target_link_libraries(sil_imgui PRIVATE d3d11 dxgi d3dcompiler dwmapi user32 gdi32 imm32)
target_link_libraries(sphere_game PRIVATE sil_imgui)
target_compile_definitions(sphere_game PRIVATE SIL_WITH_IMGUI=1)
```

WindowsMain.cppのinclude群へ追加します。本体とバックエンドは同じsubmoduleの版を使います。

```cpp
#include <imgui.h>
#include <backends/imgui_impl_win32.h>
#include <backends/imgui_impl_dx11.h>
```

WindowStateへ bool imguiReady = false; を追加します。その定義より後、無名名前空間内へ次をコピーします。RAIIクラスの設計を追加課題にはしません。「初期化したものだけ逆順に終了し、Rendererより先に終了する」という役割を押さえます。

```cpp
class ImGuiSession {
public:
    ImGuiSession(HWND window, sil::renderer::Renderer& renderer, WindowState& state)
        : state_(state)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        try {
            ImGui::StyleColorsDark();
            if (!ImGui_ImplWin32_Init(window)) {
                throw std::runtime_error("ImGui Win32 initialization failed");
            }
            win32Ready_ = true;
            if (!ImGui_ImplDX11_Init(renderer.device(), renderer.context())) {
                throw std::runtime_error("ImGui DX11 initialization failed");
            }
            dx11Ready_ = true;
            state_.imguiReady = true;
        } catch (...) {
            shutdown();
            throw;
        }
    }
    ~ImGuiSession() { shutdown(); }
    ImGuiSession(const ImGuiSession&) = delete;
    ImGuiSession& operator=(const ImGuiSession&) = delete;

private:
    void shutdown() {
        state_.imguiReady = false;
        if (dx11Ready_) {
            ImGui_ImplDX11_Shutdown();
        }
        if (win32Ready_) {
            ImGui_ImplWin32_Shutdown();
        }
        ImGui::DestroyContext();
    }
    WindowState& state_;
    bool win32Ready_ = false;
    bool dx11Ready_ = false;
};
```

wWinMain内のRenderer構築の直後へ追加します。この順序ならスコープを抜ける時にImGuiが先に終了します。終了関数を別の場所から重ねて呼ぶ必要はありません。

```cpp
ImGuiSession gui(window, renderer, windowState);
```

#### 実習2：Windows通知とフレームの接続（課題5-T：コピー可）

WindowsMain.cppのグローバルスコープ（無名名前空間より前）へ宣言を追加します。現在のバックエンドヘッダーでは利用側でこの宣言を置く形になっています。

```cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND window, UINT message, WPARAM wParam, LPARAM lParam);
```

windowProcedureのstate取得直後、switchより前へ追加します。

```cpp
const LRESULT guiHandled = state && state->imguiReady
    ? ImGui_ImplWin32_WndProcHandler(window, message, wParam, lParam)
    : 0;
```

switchの後、最後のDefWindowProcWを呼ぶ直前へ追加します。既存switchでサイズ・フォーカス・終了の通知を先に処理する順序です。

```cpp
if (guiHandled) {
    return guiHandled;
}
```

メインループのコメント「この位置でNewFrameとUI作成」へ追加します。最小化・非アクティブ時のcontinueより後です。

```cpp
ImGui_ImplDX11_NewFrame();
ImGui_ImplWin32_NewFrame();
ImGui::NewFrame();
```

コメント「この位置でGUIを描画」へ追加します。renderer.render()の後、renderer.present()の前です。NewFrameしたフレームは必ずここまで進めます。

```cpp
ImGui::Render();
ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
```

#### 実習3：開始・停止・状態表示（課題5-U：自分で書く）

UIがゲームの動作を決める部分です。装飾は増やさず、次の仕様だけ実装します。

- メインループより前に int selectedStage = 0; と bool playing = false; を置く。
- Stage 1の選択肢と開始ボタンを出す。開始時はenv.reset(initialSeed)、lastEndをRunning、accumulatorを0、playingをtrueへ変更する。
- プレイ中は選択へ戻るボタンを出す。戻るとplayingをfalseにして更新を止める。
- 残り時間と直前の終了理由を表示する。自動再開後も終了理由は残す。
- playingのときだけ更新する。GUIがキーボード入力を使う間は停止行動を渡す。

APIの呼び方は次を参考にしてよく、名前や引数を調べて埋める必要はありません。

| 目的 | 使うコード |
| --- | --- |
| ステージの選択肢 | ImGui::RadioButton("Stage 1", &selectedStage, 0); |
| ボタンの判定 | if (ImGui::Button("Start")) { /* 開始処理 */ } |
| 残り時間の表示 | ImGui::Text("Remaining: %.1f s", remaining); |
| 終了理由の表示 | ImGui::Text("Last: %s", sil::StateToStr(lastEnd).c_str()); |
| GUIの入力要求 | ImGui::GetIO().WantCaptureKeyboard |

**自分で書く範囲：** ImGui::NewFrame()の直後に、次の枠とUIの本体を追加します。remainingは std::max(0, env.stage().maxSteps - env.state().currentStep) * sil::Environment::dt で計算できます。Beginの結果がfalseでもEndは呼びます。

```cpp
if (ImGui::Begin("Stage selection")) {
    // TODO: playingに応じて、選択・開始または選択へ戻る操作を出す。
    // TODO: 残り時間とlastEndを表示する。
}
ImGui::End();
```

更新を止めるための枠組みはコピーできます。既存の accumulator += elapsed; からwhileの終わりまでを、次へ置き換えます。advanceGame()の自動再開処理はそのまま使います。

```cpp
if (playing) {
    const auto action = ImGui::GetIO().WantCaptureKeyboard
        ? sil::Direc::Stop
        : readAction();
    accumulator += elapsed;
    while (accumulator >= sil::Environment::dt) {
        advanceGame(env, action, lastEnd, initialSeed);
        accumulator -= sil::Environment::dt;
    }
} else {
    accumulator = 0.0;
}
```

状態表示はそのフレームの更新前の値です。まずはそれで構いません。WantCaptureKeyboardはGetAsyncKeyStateを無効にしないため、アプリ側で停止行動へ差し替えます。

**必須確認：** 選択中は障害物が進まず、開始すると遊べること、選択へ戻ると止まること、GUIがキーボードを使う間は球が動かないこと、終了理由が自動再開後も読めることをまとめて確認します。



### 5.8 調べ方と参考資料

初期化の全APIを順番に調査する課題はありません。コピーした基盤は「何をする処理か」「いつ呼ぶか」「失敗したらどこを見るか」が分かれば進めます。必要になった設定と、実際に出たエラーを中心に調べます。

ゲーム側で変更するAPIは、入力の意味と結果を確認します。例えば座標変換なら「どこへ・どの大きさで置くか」、カメラなら「どこから何を見るか」を先に決め、型や呼び方は例を使います。

- [Direct3D 11の初期化](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-initialize)：部品の役割や初期化で困った時。
- [D3D11CreateDeviceAndSwapChain](https://learn.microsoft.com/en-us/windows/win32/api/d3d11/nf-d3d11-d3d11createdeviceandswapchain)：設定変更やHRESULTの確認時。
- [DirectXTK 3D shapes](https://github.com/microsoft/DirectXTK/wiki/3D-shapes)：図形を描く呼び方。
- [GeometricPrimitive](https://github.com/microsoft/DirectXTK/wiki/GeometricPrimitive)：形状生成とDrawの仕様。
- [Dear ImGuiのWin32・DX11例](https://github.com/ocornut/imgui/blob/master/examples/example_win32_directx11/main.cpp)：バックエンド接続で困った時。

COMの詳細、Win32の呼び出し規約、MRT、flip modelの比較、自作シェーダーは後の学習に回せます。本章の合格条件へ追加しません。ポインタ・配列・constも、今回のコードを読んで適切に渡せる範囲から繰り返し使って覚えます。

### 5.9 この章の完了条件と振り返り

以下はこれから実施する確認です。コピーした部分も含めて動作を確かめますが、同じ確認を各課題で繰り返す必要はありません。

- [ ] Windows x64でsphere_gameがビルド・起動・終了でき、背景色を変更できる。
- [ ] 床・壁・ゴール・動く障害物・プレイヤーが、意図した位置・大きさ・前後関係で表示される。
- [ ] カメラ・物体の座標変換・入力変換・更新と再開・UIの動作を自分で実装した。
- [ ] Rendererがゲーム状態を変更せず、WASDの9行動と固定更新・即時の自動再開が働く。
- [ ] サイズ変更、最小化、フォーカス切替から戻れ、GUIとゲームの入力が競合しない。
- [ ] 1ステージを選んで開始でき、選択へ戻れ、残り時間と直前の終了理由を確認できる。
- [ ] Debugで警告やHRESULT失敗を確認し、Debug・Releaseで手動プレイを確認した。
- [ ] ヘッドレスと実装済みテストが引き続き動く。未実装・未確認はそのまま記録した。

振り返りは「入力 → step → 更新後のState → 座標変換 → Draw → Present」を、自分のコードを見ながら追えれば十分です。全APIの引数の暗唱や、コピーした基盤の白紙からの再実装は求めません。

この最小構成ではデバイスロストからの自動復旧は扱いません。Presentの失敗コードを確認して例外として終了します。自作シェーダー、複雑なモデル、音声は次の学習範囲です。

### 5.10 背景色から手動プレイまで — 作業を減らす進行表

**5-Hまで実装した現在地からは、まず5.6と5.5の実習1〜2へ進みます。** 完成例を使って最初の表示へつなぎ、5-I以降は下の順に進めます。既習部分の課題や記録をやり直す必要はありません。

| 区切り | コピーして使う基盤 | 自分で考えて書く部分 | 一度確認して次へ進む条件 |
| --- | --- | --- | --- |
| 背景色 | 5.6、5-H直後のコンストラクター、5.5実習1〜2 | 実装済みの5-Hを使う | 背景色の変更とウィンドウの開閉 |
| 深度・サイズ変更 | 5-I、5-O | なし | サイズ変更・最小化から復帰できる |
| ゲームの見た目 | 形状の生成、既存の描画設定 | 5-J〜5-L：カメラ、world行列、Stage・Stateからの描画 | 各物体の位置・大きさ・前後関係が正しい |
| 手動プレイ | 時計・固定更新のループ、キー取得のAPI | 5-P・5-Q：入力変換、1ステップと終了・再開 | 移動・終了・即時再開が働く |
| 操作画面 | 5-S〜5-T：ビルド・初期化・通知・終了処理 | 5-U：開始・戻る・状態表示 | 選択中は止まり、開始すると遊べる |
| 最終確認 | 既存の設定 | 新機能を足さず不具合を直す | 5.9を一通り確認する |

自分で書く中心は5-J〜5-L、5-P、5-Qの小さな関数、5-Uです。直方体の練習は最初の一個だけにし、そのworld行列をdrawRect()へまとめてステージ表示に使います。確認用の別アプリや、同じ描画をする別Rendererは作りません。

図形を追加するまではcube_・sphere_を生成せず、render()でも使いません。追加する段階でコンストラクターへ形状生成を一度だけ足します。幅・高さのガード、背景・深度のクリア、Presentの結果確認は残します。

| 症状 | 最初に確認すること |
| --- | --- |
| ウィンドウが出ない | Win32の失敗コード、起動対象がsphere_gameか |
| 背景が出ない | コンストラクターの接続、HRESULT、幅・高さの保存、ClearとPresent |
| 背景だけで図形がない | Drawの呼び出し、world・view・projection、カメラの範囲 |
| 球が床に埋まる | 中心Yを半径にしているか |
| 奥の物体が手前を覆う | 深度ビューの作成・設定・毎フレームのクリア |
| サイズ変更で失敗する | 古い参照の解放、バインド解除、バックバッファをメンバーに残していないか |
| 障害物が動いて見えない | stageの初期設定ではなくstate.obstaclesを描いているか |
| 復帰直後に時間切れ | 前回時刻とaccumulatorの両方をリセットしているか |

見た目の自由課題や追加の調査ノートは任意です。まず手動プレイを通し、残り時間をAIの観測・学習・保存へ使います。



<a id="chapter-6"></a>

## 第6章 周辺を観測して行動を決める

第6章と第7章は、関数名・型・引数・手順だけを示し、コードは自分で書く章です。宣言だけのコードブロックは、名前と引数をそろえるための見本です。第7章7.1で、ファイルの置き場所と一部の名前（`Genome`→`ActionTable`、`Controller`→`TableAgent`など）を変更します。第8章のCLIは7.1の後の名前を呼びます。

### 6.1 この章で作るAIの形

人間は障害物が近いと待ったり避けたりします。AIにも「いま周りに何があるか」に応じて行動を変える仕組みが必要です。本作では、状況ごとに「この状況ならこの行動」と決めた**行動表**をAIの中身にし、第7章でその表を遺伝的アルゴリズムで育てます。

処理は次の4段階です。

1. プレイヤーを中心に周辺を3×3のマスに分け、自分のマスを除く8マスそれぞれに何があるかを1バイトの種類で表す（合計8バイト）。
2. 8マスの種類の組み合わせを、行動表の番号（0〜65535）へ変換する。
3. 行動表のその番号に書かれた行動（停止＋8方向の9行動のどれか）を選ぶ。
4. 行動表全体を1個体の遺伝子とし、交叉・突然変異で次の世代を作る（第7章）。

ニューラルネットワーク、重み、逆伝播は使いません。表を読むだけなので、ある状況でなぜその行動を選んだのかを、表の1要素を見れば説明できます。

**教材での選択（第6〜8章共通）**

| 対象 | 教材での選択 | 変えたときの影響 |
| --- | --- | --- |
| マス幅 | 1.5 | 観測の意味が変わるため、保存済み個体は使えなくなる |
| マスの種類 | 空き・壁・障害物・ゴールの4種類 | 種類数をKとすると、表の長さはKの8乗 |
| 重なったときの優先 | 障害物＞ゴール＞壁＞空き | 観測の意味が変わる |
| フィールド外 | 壁として扱う | 観測の意味が変わる |
| 行動決定 | 6更新ごと（0.1秒）。次の決定まで行動を維持 | 学習時と再生時で一致させる |
| 行動の番号 | `Direc`の宣言順（`Stop`＝0〜`DownLeft`＝8） | 宣言順を変えると保存済み個体の意味が変わる |

これらの値は動く例を作るための仮の選択です。学習性能を検証した設定ではありません。

### 6.2 マスの種類を1バイトで表す（課題6-A：自分で書く）

`src/ai/Policy.hpp`を作り、`core/Simulation.hpp`をincludeして、`namespace sil`の中に次を用意します。コアと同じく、Windows・DirectX・ImGuiのヘッダーは含めません。

| 名前 | 種類 | 内容 |
| --- | --- | --- |
| `Cell` | `enum class`。基底型`std::uint8_t` | `Empty`、`Wall`、`Obstacle`、`Goal`の順。値は0〜3 |
| `cellKinds` | `inline constexpr std::size_t` | 4。`Cell`の列挙子の数 |
| `neighborCount` | `inline constexpr std::size_t` | 8。観測するマスの数 |
| `cellWidth` | `inline constexpr float` | 1.5。1マスの幅 |
| `decisionSteps` | `inline constexpr int` | 6。行動を決め直す間隔（更新回数） |
| `Observation` | `std::array<Cell, neighborCount>`の別名（`using`） | 8マス分の観測 |

- `enum class Cell : std::uint8_t`のように基底型を書くと、1要素が1バイトになります。`static_assert(sizeof(Observation) == 8);`をヘッダーに置くと、8バイトであることをコンパイル時に確かめられます。
- `enum class`は整数へ暗黙に変換されません。番号として使う場所では`static_cast<std::size_t>(cell)`のように明示します。
- `inline constexpr`は、ヘッダーに置いた定数を複数の`.cpp`から使っても重複定義にならないための指定です。

**8マスの並び順。** 並び順は保存形式の一部になるので固定します。教材では、上の行の左→右、中央の行の左→右（自分のマスを飛ばす）、下の行の左→右とします。Zが奥（ゴール方向）なので、行の+1が画面の上です。

| 添字 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 列（X方向） | −1 | 0 | +1 | −1 | +1 | −1 | 0 | +1 |
| 行（Z方向） | +1 | +1 | +1 | 0 | 0 | −1 | −1 | −1 |

この表を`neighborOffsets`という`inline constexpr std::array`にしておくと、ループで使えます。要素の型は、列と行の2つの`int`を持つ小さな構造体か、`std::array<int, 2>`です。

### 6.3 周辺を観測する（課題6-B：自分で書く）

```cpp
// 宣言の見本。本体は自分で書く。
Cell classifyCell(const Stage& stage, const State& state, const Rect& cell);
Observation observe(const Environment& env);
```

**`classifyCell`** は、1マス分の長方形に何があるかを、1つの`Cell`で返します。

| 引数 | 使い方 |
| --- | --- |
| `stage` | 壁`stage.walls`、フィールド範囲`stage.bounds`、ゴール`stage.goal`を読む |
| `state` | 現在の障害物`state.obstacles`を読む。`stage.obstacles`は初期設定なので使わない |
| `cell` | 調べるマス。`center`がマスの中心、`half`が半分の大きさ（x、zとも`cellWidth / 2`） |

- 判定には既存の`overlaps(Rect, Rect)`を使います。
- 優先順に調べ、当てはまった時点で返します。障害物と重なる→`Obstacle`、ゴールと重なる→`Goal`、壁と重なるかフィールドから一部でもはみ出す→`Wall`、どれでもない→`Empty`。
- フィールドからのはみ出しは、`cell`の左右上下の端（`center ± half`）と`bounds`の端を比べて求めます。`overlaps`は「重なっているか」しか分からず、「中に収まっているか」は分からないので別に書きます。

**`observe`** は、`env.state().player.pos`を中心に`neighborOffsets`の8マスを作り、それぞれを`classifyCell`で分類して`Observation`へ詰めて返します。

- マスの中心は「プレイヤー位置＋オフセット×`cellWidth`」です。
- 自分のマスは調べません。プレイヤー自身は障害物として数えません。
- `observe`は環境を変更しないので、`const Environment&`で受け取ります。

**考えてみる。** 障害物が2マスにまたがると、両方のマスが`Obstacle`になります。「少しでも重なれば」その種類とした理由と、「マスの中心が入っていれば」に変えた場合に何が起きるかを説明できるようにしてください。

### 6.4 8マスの観測を表の番号へ変換する（課題6-C：自分で書く）

8バイトの観測をそのまま64ビットの整数とみなすと、番号は2の64乗通りになり、表を作れません。各マスが実際に取る値は4種類だけなので、観測を**4進数の8桁**とみなして番号へ詰めます。

- 番号の範囲は0〜4⁸−1＝65535です。
- 添字0のマスを一番下の桁（4⁰の位）、添字7を一番上の桁（4⁷の位）とします。番号＝Σ `obs[i]`×4ⁱ です。
- 定数`stateCount`（`inline constexpr std::size_t`）を4⁸として用意します。`cellKinds`を`neighborCount`回掛ける`constexpr`関数で求めると、種類数を変えても追従します。
- 4種類の場合は1マス2ビットなので、「2ビット左シフトして足す」でも同じ番号になります。ただしシフト版は、種類数を5に増やすと使えなくなります。

```cpp
std::size_t toIndex(const Observation& obs);
```

確認の目安：全マス`Empty`なら0、全マス`Goal`なら65535、添字0だけ`Wall`なら1、添字1だけ`Wall`なら4。

### 6.5 行動表から行動を選ぶ（課題6-D：自分で書く）

| 名前 | 内容 |
| --- | --- |
| `actionCount` | 9。`Direc`の列挙子の数 |
| `Genome` | `std::array<std::uint8_t, stateCount>`の別名。添字が観測の番号、値が行動の番号（0〜8） |

```cpp
Direc choose(const Genome& genes, const Observation& obs);
```

- `toIndex(obs)`で番号を求め、`genes`のその要素を`static_cast<Direc>(...)`で行動にします。
- 行動の番号は`Direc`の宣言順（`Stop, Up, Down, Left, Right, UpRight, UpLeft, DownRight, DownLeft`）です。第8章の保存形式にもこの順を書きます。
- 9以上の値は不正です。不正値を作らない責任は初期化・突然変異・読込にあり、`choose`は正しい値だけが来る前提にします。値の範囲はテストで確かめます。

`Genome`は65,536バイト（64KB）あります。関数には`const Genome&`で渡し、不要なコピーを避けます。

### 6.6 一定間隔で行動を決める（課題6-E：自分で書く）

```cpp
// 宣言の見本。メンバー関数の本体は自分で書く。
class Controller {
public:
    explicit Controller(const Genome& genes);
    void reset();
    Direc next(const Environment& env);
private:
    const Genome* genes_;
    int count_{};
    Direc held_{Direc::Stop};
};
```

| メンバー | 役割 |
| --- | --- |
| コンストラクター | 表の場所を`genes_`へ覚えるだけで、64KBの表はコピーしない |
| `reset()` | `count_`を0、`held_`を`Stop`へ戻す。前の試行の状態を持ち越さない |
| `next(env)` | `count_`が`decisionSteps`の倍数のときだけ`observe(env)`と`choose`で`held_`を更新する。その後`count_`を1増やし、`held_`を返す |

- **表の寿命はControllerより長くなければなりません。** 一時オブジェクトを渡すと、すぐに無効な場所を指します。参照メンバーではなくポインターにしたのは、Controllerを代入できるようにするためです。
- `next()`は、シミュレーションの更新（`env.step()`）の直前に1回だけ呼びます。描画のたびに呼ぶと、画面の速さで判断の頻度が変わります。
- 自動再開では、環境の`reset()`とControllerの`reset()`を両方呼びます。

**この方式の限界。** 観測は「いま」だけなので、障害物が近づいているのか遠ざかっているのかは区別できません。改善したくなったら、例えば`Obstacle`を「近づく障害物」と「遠ざかる障害物」に分けて種類を5つにします。表の長さは5⁸＝390,625（約381KB）に増えますが、まだ扱えます。前回の8マスを並べて16マスにする方法は4¹⁶（約43億）通りになり、表では扱えません。種類を増やすほど、1度も訪れない状況が増える点とのトレードオフです。

### 6.7 1個体を評価する（課題6-F：自分で書く）

```cpp
struct Metrics {
    float fitness{};   // 累積報酬
    float distance{};  // 開始位置から、試行中に到達した最大のZまでの差
    float survival{};  // 終了までの秒数
    float lateral{};   // 各更新のX方向の変位の絶対値の合計
    GameState end{GameState::Running};
};
Metrics evaluate(const Genome& genes, std::uint32_t environmentSeed);
```

`evaluate`の手順は次のとおりです。

1. `makeStage()`から`Environment`を作り、`reset(environmentSeed)`する。
2. `genes`から`Controller`を作り、`reset()`する。
3. 開始時のZ（`env.state().player.pos.z`）を、到達した最大Zの初期値として覚える。
4. `env.state().gameState`が`Running`の間、更新前のXを覚える→`env.step(controller.next(env))`→戻り値の`reward`を`fitness`へ足す→最大Zと横移動量を更新する、を繰り返す。
5. 終了後に`distance`、`survival`（`currentStep`×`Environment::dt`）、`end`を埋めて返す。

`step()`が返す`StepResult`のメンバーは`end`と`reward`、`State`の終了状態は`gameState`です。名前の違いに注意してください。

**任意課題。** 評価中に訪れた番号を`std::vector<bool>`などに記録して種類数を数えると、65,536通りのうち実際に使った状況の少なさが分かります。第7章で突然変異率を考える手がかりになります。

### 6.8 観測と行動をテストする（課題6-G：自分で書く）

`tests/PolicyTests.cpp`に、第4章と同じ`check`関数の形でテストを書きます。どのテストも「ステージを作る→必要な物だけ置く→`reset()`する→観測や行動を調べる」の順です。`makeStage()`の戻り値から`walls`と`obstacles`を空にし、`bounds`を十分広くすると、狙ったマスにだけ物を置けます。障害物は`speed`を0にすると、`reset()`後の位置を予想しやすくなります。

| テスト | 用意する状況 | 期待する結果 |
| --- | --- | --- |
| 空きマス | 周りに何も置かない | 8マスすべて`Empty` |
| 障害物の位置 | プレイヤーのZ+`cellWidth`の位置に小さな障害物 | 添字1だけ`Obstacle` |
| 優先順位 | 同じマスに壁と障害物を置く | そのマスが`Obstacle` |
| フィールド外 | プレイヤーを`bounds`の左端近くに置く | 添字0・3・5が`Wall` |
| 番号変換 | 6.4の確認の目安 | 0、65535、1、4 |
| 行動選択 | 全要素0の`Genome`で、ある番号の要素だけ1にする | その観測で`Up`、別の観測で`Stop` |
| 行動の維持 | `Controller`で`next()`を連続で呼ぶ | 1回目に決めた行動が6回目まで続く |
| リセット | 途中まで動かしたControllerを`reset()`する | 新しいControllerと同じ行動列になる |

**差し込み例：`CMakeLists.txt`の末尾に追加（コピー可）。**

```cmake
if(BUILD_TESTING)
    add_executable(policy_tests tests/PolicyTests.cpp)
    target_link_libraries(policy_tests PRIVATE sphere_core)
    add_test(NAME policy COMMAND policy_tests)
endif()
```

`policy_tests`が`src/`からのインクルードパスで`ai/Policy.hpp`を見つけられるか確認してください。見つからない場合は、`sphere_core`のインクルードディレクトリの設定を見直します。

<a id="chapter-7"></a>

## 第7章 遺伝的アルゴリズムで行動表を育てる

### 7.1 フォルダとファイルを再構成する（課題7-R：自分で書く）

第6章では、観測・行動表・評価を`src/ai/Policy`にまとめて書きました。この後、遺伝的アルゴリズム（第7章）と保存（第8章）を足し、期限後にはA*やモンテカルロ木探索（MCTS）など、Mario AI Competition 2009で使われた手法も試す予定です。全部を`src/ai`の1か所に置き、GA用の表を「Policy（方策一般）」と呼んだままだと、どれがどの手法の部品か分からなくなります。そこで、遺伝操作を書き始める前に、役割ごとにファイルを分け直します。

この節では処理の中身を変えません。名前と置き場所だけを変え、再構成の前後で同じ結果になることを確かめます。このように、動作を変えずに構造だけを整える作業を**リファクタリング**と呼びます。

**始める前に、第6章の状態をコミットしてください。** 再構成で壊れたときに、差分を見て戻せます。

#### 整理の考え方：行動を決めるものと、それを育てるもの

| 第6章で書いたもの | 性質 | ほかの手法でも使うか |
| --- | --- | --- |
| `Cell`、`observe`、`toIndex` | 周辺の見え方 | 使う |
| `Metrics`、1試行のループ | 1試行を実行して成績を測る | 使う。どのAIも同じ表で比べたい |
| `Genome`、`choose`、`Controller` | 行動表で行動を決める | 行動表方式だけ |
| 選択・交叉・突然変異（この章で書く） | 行動表を育てる | GAだけ |

A*やMCTSは学習せず、毎回その場で先読みして行動を決めます。一方GAには、「行動を決めるもの（行動表）」と「それを育てるもの（GA）」が別々にあります。この2つを別のフォルダに分けておくと、「行動表を別の方法で育てる」「別の表し方をGAで育てる」といった組み合わせを、片方だけ足して試せます。

#### 新しい構成

```text
src/ai/
  common/                    手法に依存しない部品
    Observation.hpp/.cpp     Cell, Observation, observe, toIndex など
    Episode.hpp              Metrics, runEpisode
  table/                     行動表という「方策の表し方」
    ActionTable.hpp/.cpp     ActionTable, choose, TableAgent, evaluate
    TableFile.hpp/.cpp       行動表の保存・読込（8.2で作る）
  ga/                        行動表を「育てる」手法
    GeneticTrainer.hpp/.cpp  Individual, initialPopulation など（7.3で作る）
tests/
  AiTests.cpp                第6章のテストを移したもの。以後のAIのテストもここへ足す
```

A*用の`search/`や、比較の基準にする単純なAI用の`rule/`などは、必要になったときに作ります。空のフォルダを先に作る必要はありません。

**名前の付け方**

| 接尾辞・名前 | 意味 | 例 |
| --- | --- | --- |
| `〜Agent` | 毎更新`Direc`を返し、プレイヤーを操作するもの | `TableAgent`。将来の`AStarAgent`、`MctsAgent` |
| `〜Trainer` | Agentの中身を学習・進化させるもの | `GeneticTrainer` |
| `〜File` | 保存形式 | `TableFile` |
| フォルダ名 | 手法の系統（共通部品・表し方・学習法・探索法） | `common/`、`table/`、`ga/`、`search/` |

**includeしてよい向き**

```text
core  ←  ai/common  ←  ai/table  ←  ai/ga
                            ↑           ↑
                 app（WindowsMain、ExperimentMain）、tests
```

矢印の元が、矢印の先をincludeできます。`common`は`table`や`ga`を、`table`は`ga`をincludeしません。逆向きのincludeが必要になったら、置き場所が間違っている合図です。`core`は`ai`を一切includeしません。

#### 名前の変更

| 第6章の名前 | 再構成後 | 置き場所 | 理由 |
| --- | --- | --- | --- |
| `Cell`、`Observation`、`cellKinds`、`neighborCount`、`cellWidth`、`neighborOffsets`、`stateCount`、`classifyCell`、`observe`、`toIndex` | 変更なし | `common/Observation` | — |
| `Metrics` | 変更なし | `common/Episode.hpp` | どの手法でも同じ成績表を使う |
| `evaluate`の中の1試行のループ | `runEpisode` | `common/Episode.hpp` | 行動表以外のAIでも使えるようにする |
| `Genome` | `ActionTable` | `table/ActionTable` | 「遺伝子」はGAから見た呼び方で、表そのものは育て方と関係ない |
| `actionCount`、`decisionSteps`、`choose` | 変更なし | `table/ActionTable` | — |
| `Controller`の`next` | `TableAgent`の`act` | `table/ActionTable` | 将来のほかのAgentと呼び方をそろえる |
| `evaluate(const Genome&, seed)` | `evaluate(const ActionTable&, seed)` | `table/ActionTable` | 中で`TableAgent`を作り、`runEpisode`を呼ぶだけにする |

自分で別の名前（例：`GenomeController`、`nextDirec`）を使っていた場合も、ここで表の名前にそろえてください。第8章のCLIはこの表の名前を呼びます。GAの中で行動表を指すときは、`Individual`のメンバー名を`genes`とし、GAらしい呼び方を残します。

#### ヘッダーと実装を分ける

`sphere_ai`というライブラリにまとめるため、各ファイルを`.hpp`（宣言）と`.cpp`（実装）に分けます。

| `.hpp`に置くもの | `.cpp`に置くもの |
| --- | --- |
| 型（`enum class`、`struct`、`class`の宣言）と`using`の別名 | 関数の本体 |
| `inline constexpr`の定数 | クラスのメンバー関数の本体（`TableAgent::act`など） |
| 関数の宣言（例：`Observation observe(const Environment& env);`） | そのファイルの中だけで使う補助関数 |
| テンプレート（`runEpisode`）の本体 | — |

- `.hpp`の先頭には`#pragma once`を書きます。同じヘッダーを2回includeしても1回分になります。
- `.cpp`では、対応する`.hpp`を最初にincludeします。そのヘッダーが単独でコンパイルできるかの確認になります。
- ヘッダーには`using namespace std;`を書かず、`std::array`、`std::uint8_t`、`std::size_t`と書きます。`.cpp`の中なら使っても構いません。
- `.cpp`に関数の本体を書くときも、`namespace sil { ... }`で囲みます。
- `.cpp`から`.cpp`をincludeしません。テストも`.hpp`をincludeし、実装はライブラリのリンクで使います。

#### `runEpisode`をテンプレートにする

```cpp
// ai/common/Episode.hpp に置く宣言の見本。本体もヘッダーに書く。
template <class AgentT>
Metrics runEpisode(AgentT& agent, std::uint32_t environmentSeed);
```

第6章の`evaluate`の手順のうち、`Controller`を作る部分以外をそのまま移します。`agent`には「`reset()`と`act(const Environment&)`を持つもの」なら何でも渡せます。

- **テンプレートとは**：型を引数にした関数の型紙です。`runEpisode(tableAgent, 42)`と呼ぶと、コンパイラーが`AgentT`を`TableAgent`として本体を生成します。生成するにはコンパイラーが本体を見る必要があるため、`.cpp`ではなくヘッダーに書きます。
- **仮想関数（`virtual`）にしない理由**：今のAgentは`TableAgent`の1種類だけです。共通の基底クラスを先に作るより、2つ目のAgent（A*など）を作って共通部分が見えてから決めるほうが、設計を外しません。テンプレートなら、基底クラスがなくても、後から来るAgentをそのまま受け取れます。
- `common/Episode.hpp`は`core/Simulation.hpp`だけをincludeし、`table/`のヘッダーはincludeしません。

`table/ActionTable`の`evaluate(const ActionTable& table, std::uint32_t environmentSeed)`は、`TableAgent`を作って`runEpisode`に渡し、結果を返すだけの関数になります。

#### CMakeを分ける（コピー可）

AIを`sphere_core`から分け、`sphere_ai`ライブラリにします。コアを純粋なシミュレーションのまま保ち、AIを使う実行ファイルだけが`sphere_ai`をリンクする形です。第6章で追加したテスト用のブロック（`policy_tests`など）を削除し、`sphere_core`の設定の後に次を追加します。

```cmake
add_library(sphere_ai
    src/ai/common/Observation.cpp
    src/ai/table/ActionTable.cpp)
target_link_libraries(sphere_ai PUBLIC sphere_core)
if(MSVC)
    target_compile_options(sphere_ai PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(sphere_ai PRIVATE -Wall -Wextra -Wpedantic)
endif()
```

テストは`if(BUILD_TESTING)`の中へ追加します。

```cmake
    add_executable(ai_tests tests/AiTests.cpp)
    target_link_libraries(ai_tests PRIVATE sphere_ai)
    add_test(NAME ai COMMAND ai_tests)
```

- `sphere_ai`は`sphere_core`を`PUBLIC`でリンクしています。そのため、`sphere_ai`をリンクした側も、`src/`のインクルードパスと`sphere_core`をそのまま使えます。
- `GeneticTrainer.cpp`は7.3で、`TableFile.cpp`は8.2で、ファイルを作った時点で`add_library(sphere_ai ...)`の一覧へ追加します。存在しないファイルを先に書くと、CMakeの構成が失敗します。
- 1つのソースファイルは、1つのターゲットにだけ入れます。Visual Studioの「CMakeターゲットに追加」は、選んだすべてのターゲットにファイルを足すことがあります。追加した後は`CMakeLists.txt`の差分を確認してください。
- `scripts/build.ps1`の既定のビルド対象に`ai_tests`は含まれません。`.\scripts\build.ps1 -Target sphere_tests,ai_tests -RunTests`のように指定します。

**文字コード**：新しく作るファイルはUTF-8で保存してください。`sphere_ai`は`/utf-8`でコンパイルするため、Shift-JISのコメントが残っていると警告C4828が出ます。Visual Studioでは「ファイル→名前を付けて保存」の保存ボタン横の▼から「エンコード付きで保存」を選び、「Unicode (UTF-8 シグネチャ付き)」にします。

#### 手順

1. 第6章の状態をコミットする。
2. 再構成の前に、全要素`Up`の行動表とシード42で`evaluate`を呼び、`fitness`、`distance`、`survival`、`end`を表示して記録しておく。
3. フォルダを作り、上の表に従ってファイルを分け、名前を変える。Visual Studioの「名前の変更」（Ctrl+R、Ctrl+R）を使うと、参照している箇所もまとめて変わる。
4. 第6章のテストを`tests/AiTests.cpp`へ移し、includeを新しいヘッダーに直す。
5. CMakeを直し、`sphere_ai`と`ai_tests`をビルドする。
6. 第6章のテストがすべて通ることと、手順2の値が変わっていないことを確かめる。
7. 古い`src/ai/Policy`と古いテストファイルを削除し、もう一度ビルドする。古いファイルを参照している箇所があれば、ここでエラーになる。
8. `src/core/`の中に`ai/`をincludeしている箇所がないことを検索で確かめて、コミットする。

**完了条件：** `sphere_ai`と`ai_tests`がビルドでき、第6章のテストが通る。再構成の前後で`evaluate`の結果が一致する。`src/ai/`の直下にファイルがなく、`common/`と`table/`に分かれている。

**考えてみる。** A*で「この行動を取ったら数手後にどうなるか」を試すには、`Environment`を複製して先へ進めます。今の`Environment`は`Stage`と`State`などの値だけを持つので、コピーで複製できます。シミュレーションと描画を分けた設計が、ここでも効いてきます（A*自体は期限後の課題です）。

### 7.2 個体と世代の考え方

| 遺伝的アルゴリズムの用語 | 本作での意味 |
| --- | --- |
| 遺伝子座 | 行動表の1要素（ある観測番号に対応する場所） |
| 対立遺伝子 | その要素の値（9行動のどれか） |
| 個体 | 行動表1つ（`ActionTable`、65,536要素） |
| 適応度 | 1試行の累積報酬（`Metrics::fitness`） |
| 世代 | 全個体を評価し、次の集団を作る1周 |

ランダムな表では、球はゴールへ進むとは限りません。多数の個体を同じ条件で試し、成績の良い個体の表を混ぜたり書き換えたりして次の世代を作ります。1世代で必ず改善するわけではないため、最良値だけでなく平均値と到達距離も記録します。

**教材の設定値（性能を検証した値ではありません）**

| 項目 | 設定 |
| --- | --- |
| 個体数 | 64 |
| エリート | 上位4個体をそのまま次世代へ残す |
| 親選択 | 無作為に選んだ3個体のうち、適応度が最大の個体（トーナメント選択） |
| 交叉 | 一様交叉。要素ごとに2親のどちらかからコピーする |
| 突然変異 | 要素ごとに確率0.01で、0〜8のランダムな行動へ置き換える |

突然変異を「値に小さな乱数を足す」形にしないのは、行動の番号に大小の意味がないからです。`Up`（1）に1を足した`Down`（2）は「少し違う行動」ではありません。そのため値を置き換えます。

確率0.01でも、65,536要素なら1個体あたり約655要素が変わります。ただし1回の試行で訪れる観測はごく一部なので、訪れない要素が変わっても成績には影響しません。6.7の任意課題で訪れた種類数を数え、変化が多すぎる・少なすぎると感じたら確率を調整します。

### 7.3 必要な関数（課題7-A〜7-C：自分で書く）

`src/ai/ga/GeneticTrainer.hpp`と`GeneticTrainer.cpp`を作ります。ヘッダーでは`ai/table/ActionTable.hpp`、`<random>`、`<vector>`を、`.cpp`では`<algorithm>`と`<stdexcept>`もincludeします。ヘッダーに宣言、`.cpp`に本体を書き、`GeneticTrainer.cpp`を`CMakeLists.txt`の`add_library(sphere_ai ...)`へ追加してください。

```cpp
// 宣言の見本。本体は自分で書く。
struct Individual { ActionTable genes{}; Metrics metrics{}; };

std::vector<Individual> initialPopulation(std::mt19937& random, int count = 64);
void evaluatePopulation(std::vector<Individual>& population, std::uint32_t seed);
std::vector<Individual> nextGeneration(const std::vector<Individual>& population,
                                       std::mt19937& random, double mutationRate = 0.01);
```

**課題7-A：`initialPopulation`**

| 引数 | 使い方 |
| --- | --- |
| `random` | 学習用の乱数生成器。呼び出し側が持ち、同じシードなら同じ集団になる |
| `count` | 個体数。エリート数の4より少なければ`std::invalid_argument`で拒否する |

- `random`を参照で受け取るのは、呼び出し後も乱数列の続きを使うためです。値渡しにすると内部状態がコピーされ、次の世代で同じ乱数を繰り返してしまいます。
- 全個体の全要素を0〜8の一様乱数で埋めます。分布は`std::uniform_int_distribution<int>(0, static_cast<int>(actionCount) - 1)`で作り、値を`static_cast<std::uint8_t>`して入れます。**`std::uniform_int_distribution<std::uint8_t>`は標準で認められていない型**なので、`int`で作ってから変換します。
- 64個体×64KBで約4MBです。`std::vector`の要素はヒープに置かれるので問題ありませんが、`Individual`のローカル変数を大量に作る書き方は避けます。

**課題7-B：`evaluatePopulation`**

- 全個体に`evaluate(genes, seed)`を行い、結果を`metrics`へ入れます。
- その後、`fitness`の大きい順に`std::stable_sort`で並べます。同点の順序を保つ`stable_sort`にすると、結果が決定的になります。
- 比較関数は「aをbより前に置くならtrue」を返すラムダです。

**課題7-C：`nextGeneration`**

前提は、`population`が評価・整列済みであることです。4個体未満なら例外にします。

1. 先頭4個体をそのまま新しい集団へコピーする（エリート）。
2. 新しい集団が元の個体数になるまで、次を繰り返す。
   1. トーナメントで親Aを選ぶ。集団から無作為に3個体を選び（重複可）、`fitness`が最大の個体にする。親Bも同じ方法で選ぶ。
   2. 子の各要素について、確率0.5で親Aの値、そうでなければ親Bの値をコピーする。
   3. 確率`mutationRate`で、その要素を0〜8の乱数に置き換える。
   4. 子の`metrics`は空のままにする。親の成績は引き継がず、次の世代で必ず評価し直す。

| 使う道具 | 用途 |
| --- | --- |
| `std::uniform_int_distribution<std::size_t>(0, 個体数 - 1)` | トーナメントで個体を選ぶ |
| `std::bernoulli_distribution(0.5)` | 各要素をどちらの親から取るか |
| `std::bernoulli_distribution(mutationRate)` | その要素を突然変異させるか |
| 7-Aと同じ行動の分布 | 置き換える行動 |

- トーナメントは小さなラムダにまとめ、選んだ個体の`const ActionTable&`を返すと64KBのコピーを避けられます。
- `mutationRate`を引数にしたのは、テストで0や1を指定するためです。CLIからは既定値の0.01で呼ばれます。

**乱数を使う順番も結果の一部です。** 分布を呼ぶ順番を変えると、同じシードでも別の集団になります。決定性のテストが通った後で順番を変えたら、以前の学習結果とは同じ条件で比べられません。

同じ環境シードを全個体に渡しますが、この教材のステージ自体には乱数がありません。決定的な方策を同じ条件で繰り返しても、多様な条件での強さを測ったことにはなりません。学習用シードを変えた独立実験と、同じ個体の評価回数を増やすことを区別してください。

### 7.4 遺伝操作の確認（課題7-D：自分で書く）

`AiTests.cpp`で`ai/ga/GeneticTrainer.hpp`をincludeし、次を確かめます。個体数は8で行います。4個体だと全個体がエリートになり、交叉と突然変異が働きません。

| 確認 | 方法 |
| --- | --- |
| シードの再現 | 同じシードの`std::mt19937`を2つ作り、`initialPopulation`の結果の`genes`が一致する |
| 値の範囲 | 初期集団と次世代の全要素が9未満 |
| 個体数 | `nextGeneration`の前後で個体数が同じ |
| エリート保存 | 次世代の先頭4個体の`genes`が、評価・整列後の先頭4個体と一致する |
| 突然変異なし | 全個体を同じ表にして`mutationRate`を0にすると、次世代も全員が同じ表 |
| 突然変異あり | 同じ条件で`mutationRate`を1にすると、エリート以外の表が元と異なる（元と一致する要素はおよそ9分の1） |

`std::array`同士は`==`で全要素を比較できます。

乱数生成器の状態が同じなら同じ列を生成できますが、`uniform_int_distribution`などの分布の変換方法は標準ライブラリの実装によって違います。異なる標準ライブラリ・OS間で同じ学習結果になるとは主張しません。

<a id="chapter-8"></a>

## 第8章 保存・読み込み・CLIを作る

### 8.1 行動表だけでは再実行条件が足りない

同じ行動表でも、マス幅、マスの種類と優先順位、8マスの並び、番号の桁の順、行動の番号が違えば、選ばれる行動は変わります。ステージの障害物速度や制限時間が違う場合も、学習時とは別条件です。行動表と一緒に、その意味を決める条件を保存します。

教材では読めるテキスト形式を使い、構成を固定したバージョン1とします。ステージ設定は文字列化して保存し、現在の`makeStage()`と一致する場合だけ受け入れます。対応する設定を現在のコードから復元する方式なので、異なるステージの保存ファイルを読む汎用形式ではありません。条件を変えたら読み込みで明示的に失敗し、黙って別条件で実行しないことを優先します。

### 8.2 保存形式（課題8-A：自分で書く）

`src/ai/table/TableFile.hpp`と`TableFile.cpp`を作ります。ヘッダーでは`ai/table/ActionTable.hpp`と`<string>`を、`.cpp`では`<cmath>`、`<fstream>`、`<iomanip>`、`<limits>`、`<sstream>`もincludeします。行動表の保存形式なので`table/`に置き、`ga/`には依存させません。`TableFile.cpp`を`add_library(sphere_ai ...)`へ追加してください。

```cpp
// 宣言の見本。本体は自分で書く。
std::string stageText();
std::string modelText();
struct SavedTable {
    ActionTable table{};
    int generation{};
    std::uint32_t environmentSeed{}, trainingSeed{};
    Metrics metrics{};
};
void saveTable(const std::string& path, const SavedTable& saved);
SavedTable loadTable(const std::string& path);
```

**ファイルの中身。** 1行ごとに次の内容を書きます。

```text
SIL_AGENT_V1
"stageText()の文字列"
"modelText()の文字列"
世代番号 環境シード 学習シード
fitness distance survival lateral 終了理由の整数
65536
031250004871...（65,536文字。1文字が1要素の行動番号）
```

**`stageText`**：`makeStage()`の設定を空白区切りの文字列にします。`bounds`、`player.pos`、`player.radius`、`player.speed`、`maxSteps`、`goal`、壁の個数と各壁、障害物の個数と各障害物（`box`、整数にした`axis`、`low`、`high`、`speed`、`phase`）の順です。`std::ostringstream`に`std::setprecision(std::numeric_limits<float>::max_digits10)`を設定してから書きます。`max_digits10`は、`float`を文字列にして読み戻したとき元の値へ戻るのに十分な桁数です。

**`modelText`**：観測と行動の決まりを1つの文字列で表した固定値です。例えば次のように、第6章で決めたことを全部入れます。

```text
neighbors8-cell1.5-rowsTopToBottom-skipCenter;kinds-Empty-Wall-Obstacle-Goal;priority-Obstacle-Goal-Wall;outside-Wall;index-base4-cell0Lowest;actions-Stop-Up-Down-Left-Right-UpRight-UpLeft-DownRight-DownLeft;decision6-dt1/60;reward-（自分のstep()の報酬計算）
```

第6章の選択や報酬を変えたら、この文字列も変えます。そうすれば古いファイルの読み込みが失敗し、別の意味の表で黙って動くことを防げます。

**`saveTable`**

- `std::ofstream`で開き、失敗したら`std::runtime_error`を投げます。
- 2つの設定文字列は`std::quoted`で書きます。空白を含む文字列を1項目として読み書きできます。
- 成績の浮動小数点は`max_digits10`の桁数で書きます。
- 行動表は要素数を書いた後、各要素を文字`'0'`〜`'8'`に変換し、1行に続けて書きます。数字の文字は`'0'`から連続しているので、`'0' + gene`を`char`へ変換すると、その数字の文字になります。
- **落とし穴**：`std::uint8_t`は多くの環境で`unsigned char`の別名です。`out << gene`と書くと数字ではなく文字コード3や5の制御文字が出力されます。数値として書くなら`static_cast<int>(gene)`、1文字として書くなら上の変換を使います。
- 最後に`close()`し、ストリームの状態を確認して、書き込み失敗も例外にします。

**`loadTable`**（不正なファイルは必ず例外にする）

- 先頭が`SIL_AGENT_V1`、ステージ文字列が`stageText()`、モデル文字列が`modelText()`と一致しなければ失敗。
- 世代番号が負、終了理由の整数が`Success`〜`Timeout`の範囲外、成績が有限でない（`std::isfinite`で調べる）なら失敗。
- 要素数が`stateCount`と違えば失敗。
- 行動表の行を`std::string`へ1語として読み、長さが`stateCount`か、全文字が`'0'`〜`'8'`かを確かめます。`c - '0'`で番号へ戻して`table`へ入れます。
- 最後に`in >> std::ws`で空白を読み飛ばし、`eof()`でなければ余計なデータがあるとして失敗。

この形式は方策の保存です。集団全体、途中の評価状態、乱数生成器の内部状態は保存していないため、学習途中から同じ乱数列で再開する機能にはなりません。また、保存は指定先を上書きします。実験ごとに別の出力先を使ってください。途中書き込みによる破損への対策は、一時ファイルへ書いて成功後に置換する方法を追加課題とします。

### 8.3 コマンドラインから実験する

以下は教材用CLIの完全例です（コピー可）。第6〜8章の関数を、7.1の後の名前と引数で用意していれば、そのまま呼び出せます。新しい依存関係を増やさず、`std::from_chars`で整数を読みます。文字列の一部だけが数値だった場合も、入力末尾まで消費したかを調べて拒否します。

**完全例：`src/app/ExperimentMain.cpp`**

```cpp
#include "ai/ga/GeneticTrainer.hpp"
#include "ai/table/TableFile.hpp"
#include <algorithm>
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
            sil::saveTable(prefix + "_fitness.sil", {best.genes, generation, o.seed, o.trainingSeed, best.metrics});
            sil::saveTable(prefix + "_distance.sil", {farthest->genes, generation, o.seed, o.trainingSeed, farthest->metrics});
        }
        if (generation != o.generations) population = sil::nextGeneration(population, random);
    }
}
void evaluate(const Options& o) {
    const auto saved = sil::loadTable(o.input);
    std::cout << "trial,fitness,distance,survival,lateral,end\n";
    for (int trial = 0; trial < o.evaluations; ++trial) {
        const auto m = sil::evaluate(saved.table, o.seed);
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
target_link_libraries(sphere_experiment PRIVATE sphere_ai)
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

**確認用の差し込み例：AiTests.cppで`ai/table/TableFile.hpp`をincludeし、`try`内へ追加。**

```cpp
sil::SavedTable original;
original.table[7] = 5;
original.table[sil::stateCount - 1] = 8;
original.metrics.end = sil::GameState::Timeout;
sil::saveTable("roundtrip.sil", original);
const auto restored = sil::loadTable("roundtrip.sil");
check(restored.table == original.table, "action table survives text roundtrip");
```

破損検査は、保存ファイルをコピーし、先頭のバージョン、要素数、行動表の文字（`9`や英字に変える、末尾を削って短くする）を変更して、読み込みが失敗するか確かめます。学習用の元ファイルは残してください。

### 8.5 保存済みAIを画面で動かす

第5章の人間用ループで行動を決めている場所を、次のように置き換えます。変数名は第5章の`env`へ合わせます。コードはコンストラクター直後・更新箇所・自動リセット箇所の3か所へ分けて差し込みます。

```cpp
// WindowsMain.cppのincludeへ追加。
#include "ai/table/TableFile.hpp"

// envを構築した後。パスは実行時の作業フォルダーからの相対位置。
const auto saved = sil::loadTable("run123_g20_distance.sil");
env.reset(saved.environmentSeed);
sil::TableAgent agent(saved.table);
agent.reset();
```

```cpp
// 固定時間刻みの更新内で、人間の入力から行動を作る部分を置換。
const auto result = env.step(agent.act(env));
if (result.end != sil::GameState::Running) {
    // ここで終了結果を画面表示用の変数へ記録する。
    env.reset(saved.environmentSeed);
    agent.reset();
}
```

`TableAgent`は`saved.table`を参照するだけなので、`saved`はAgentを使い終わるまで破棄されない場所に置きます。上の例のように、メッセージループより前の同じスコープで宣言すれば問題ありません。

第5章のもとの`env.step()`とリセット処理は残して二重に呼ばず、この処理へ置換してください。ImGui版では`if (playing)`の内側を置換し、開始ボタンで環境をリセットする際にも`saved.environmentSeed`を使って`agent.reset()`を呼びます。`TableAgent::act()`の呼び出しも1更新に1回です。`CMakeLists.txt`では、`sphere_game`のリンク先に`sphere_ai`を追加します。モデル選択GUIを追加する場合は、読込失敗を画面に表示し、成功したファイルだけを実行します。教材の最初の到達点はファイル名をコード中で指定する方法です。

### 8.6 最終世代のAIをウィンドウでデモ再生する

8.5は再構成前の名前（`TableAgent`、`loadTable`）で書いた一般的な説明です。この節では、今のリポジトリの実装に合わせます。使うのは`GAAgent`、`SavedGenome`、`loadGenome`と、`sphere_experiment`の保存名`<prefix>_g<世代>_<distance|fitness>.sil`です。**学習の最終世代をウィンドウで動かす専用の実行ファイル`sphere_aidemo`**を作ります。8.5と内容が重なる部分は、この節を優先してください。

**方針：手動プレイの`sphere_game`には手を入れない。** `sphere_game`は完成しているので、AIのデモは別の実行ファイルにして、新しい`wWinMain`を`src/app/AiDemoMain.cpp`に書きます。1つの実行ファイルに`wWinMain`は1つしか置けません。そのため、ソースもCMakeターゲットも別にします。

| | `sphere_game`（既存） | `sphere_aidemo`（この節） |
| --- | --- | --- |
| 入口 | `src/app/WindowsMain.cpp`の`wWinMain` | `src/app/AiDemoMain.cpp`の`wWinMain` |
| 行動を決めるもの | WASD（`readDirec`） | 保存済みの`GAAgent` |
| リンク | `sphere_core` | `sphere_ai`（`sphere_core`も含む） |
| 共通部分 | Renderer、ImGui、ウィンドウ処理、固定時間刻み | 同じものを使う |

**完成形。** `sphere_aidemo.exe`を起動すると、`results/`から最終世代の保存ファイルを自動で選んで読み込みます。学習時と同じシード・同じ固定時間刻みでAIが球を動かし、1回終わるたびに同じ条件で最初から繰り返します。ImGuiの窓には、読み込んだファイル名、世代番号、保存時の成績、現在の到達距離を表示します。コマンドライン引数でファイルを指定すれば、初期世代や途中世代も再生できます。

| 実習 | 内容 | 区分 |
| --- | --- | --- |
| 1 | `WindowsMain.cpp`を土台にして`AiDemoMain.cpp`を作る | コピー可 |
| 2 | CMakeに`sphere_aidemo`を追加する | コピー可 |
| 3 | 最終世代のファイルを探す`findLatestGenome` | 自分で書く |
| 4 | 起動引数と読み込み、`GAAgent`の寿命 | 理解して使う |
| 5 | 固定時間更新の中でAIに行動を決めさせる | 自分で書く |
| 6 | ImGuiで情報を表示する | 自分で書く |

#### 実習1：`WindowsMain.cpp`を土台にして`AiDemoMain.cpp`を作る（コピー可）

`src/app/WindowsMain.cpp`を`src/app/AiDemoMain.cpp`という名前でコピーします。Visual Studioで保存するときは、8章までと同じく「Unicode (UTF-8 シグネチャ付き)」を選びます。コピー後に**`AiDemoMain.cpp`の側だけ**、次のように整理します。

| 部品 | 扱い | 理由 |
| --- | --- | --- |
| `WindowState`、`WindowLifetime`、`windowProcedure`、`ImGuiSession` | そのまま残す | ウィンドウ・サイズ変更・フォーカス・ImGuiの処理は手動プレイと同じ |
| ウィンドウクラス名`L"SphereIsLearningWindow"` | `L"SphereIsLearningAiDemo"`へ変える | 2つを同時に起動して見比べるとき、同じ名前のクラスを登録しないため |
| ウィンドウタイトル`L"sphere is learning"` | `L"sphere is learning - AI demo"`などへ変える | どちらの窓か見分けるため |
| `readDirec` | 削除する | デモではキー入力で球を動かさない |
| `advanceGame` | 実習5で書き直す | Agentのリセットが必要になる |
| `lastEnd`、`lastReward`、Start/Stopボタン、結果の履歴 | 残すか消すかは自由 | 残すなら、Startボタンでも実習5のリセットを行う |
| エラー時の`MessageBoxA`のタイトル | `"sphere is learning AI demo"`などへ変える | どちらのexeのエラーか見分けるため |

ここまでで一度、AIを入れずにビルドして、`sphere_game`と同じステージが表示されるところまで確認します。この時点では球は動きません。

**重複について。** この方法では、`windowProcedure`や`ImGuiSession`などの約150行が2つのファイルにあります。期限を優先して`sphere_game`を壊さない形にしました。片方を直したらもう片方も直す必要がある点は、既知の制約としてREADMEに書いておきます。共通部分を`src/app/Win32Window.hpp`／`.cpp`へ切り出して両方から使う形は、追加課題とします（`sphere_game`にも手を入れるため、9.2の画面確認をやり直すこと）。

#### 実習2：CMakeに`sphere_aidemo`を追加する（コピー可）

`CMakeLists.txt`の`if(WIN32)`の中で、`sil_imgui`を定義した**後**（`add_executable(sphere_experiment ...)`の近く）に追加します。`sphere_game`の行は変更しません。

```cmake
    # 保存済みの最終世代AIをウィンドウで再生するデモ
    add_executable(sphere_aidemo WIN32
        src/app/AiDemoMain.cpp
        src/renderer/directX11/Renderer.cpp
        src/renderer/directX11/Renderer.hpp)
    target_include_directories(sphere_aidemo PRIVATE src)
    target_compile_definitions(sphere_aidemo PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
    target_link_libraries(sphere_aidemo PRIVATE sphere_ai sil_imgui DirectXTK user32 shell32 d3d11 dxgi)
    if(MSVC)
        target_compile_options(sphere_aidemo PRIVATE /W4 /permissive- /utf-8)
    endif()
```

- `WIN32`は、コンソールではなく`wWinMain`から始まるWindowsアプリにする指定です。`sphere_game`と同じです。
- `sphere_ai`は`sphere_core`を`PUBLIC`でリンクしているので、`sphere_core`は書かなくてもリンクされます。
- `shell32`は、実習4で起動引数を分解する`CommandLineToArgvW`のために必要です。
- `Renderer.cpp`は`sphere_game`と`sphere_aidemo`で2回コンパイルされます。2つなら問題ありません。3つ目のexeが増えたら、`sphere_renderer`という静的ライブラリにまとめることを検討します。
- 依存の向きは「`app`→`ai`→`core`」のままです。`core`や`renderer`から`ai`をincludeしません。

`.\scripts\build.ps1`でビルドし、`build\Debug\sphere_aidemo.exe`ができることを確認します。

#### 実習3：最終世代のファイルを探す（課題8-B：自分で書く）

`train`は、`--output seed42`なら`seed42_g0_*.sil`、`seed42_g25_*.sil`、`seed42_g50_*.sil`のように、世代番号を名前に入れて保存します。「最終世代」は、この番号が最大のファイルです。ファイル名を毎回書き換えなくて済むように、フォルダーから探す関数を`AiDemoMain.cpp`の無名名前空間に書きます。

```cpp
// 宣言の見本。本体は自分で書く。<filesystem>、<charconv>、<optional>、<string_view>をinclude。
// folder内の「prefix + "_g" + 数字 + "_" + kind + ".sil"」のうち、数字が最大のパスを返す。
// 1つもなければstd::nulloptを返す。
std::optional<std::filesystem::path> findLatestGenome(
    const std::filesystem::path& folder, std::string_view prefix, std::string_view kind);
```

**手順のヒント（上から順に試す）。**

1. `std::filesystem::exists(folder)`が偽なら、`std::nullopt`を返す。
2. `for (const auto& entry : std::filesystem::directory_iterator(folder))`で1つずつ見る。`entry.is_regular_file()`でないものは飛ばす。
3. `entry.path().filename().string()`で名前を`std::string`にする。先頭が`prefix + "_g"`、末尾が`"_" + kind + ".sil"`かを、`starts_with`と`ends_with`（C++20）で調べる。
4. 間に残った部分を`std::string_view`の`substr`で切り出し、8.3のCLIと同じ`std::from_chars`で整数にする。**末尾まで読めたか**も確かめる。`seed42_gX_distance.sil`のような名前を世代0と誤読しないためです。
5. これまでの最大値より大きければ、番号とパスを覚えておく。

**確認方法。** `results/`には現在`seed42_g0`、`g25`、`g50`があります。次の3点を、一時的に`OutputDebugStringA`や画面表示で確かめます。

- `findLatestGenome("results", "seed42", "distance")`が`results\seed42_g50_distance.sil`を返す。
- `kind`を`"fitness"`にすると、`g50_fitness`になる。
- 存在しない`prefix`では`std::nullopt`になる。

**考えてみる。** 名前を文字列のまま比べると、`g50`のほうが`g100`より大きいと判定されます。数値に直す理由を説明できるようにしてください。

**`distance`と`fitness`のどちらを再生するか。** 現在の`evaluate`は`fitness = bestZ`なので、2つは同じ値で選ばれます。ただし選び方が違います。`fitness`は`population.front()`（並べ替え後の先頭）、`distance`は`max_element`で選ぶため、同点の個体が複数あると別の個体になる場合があります。デモの既定は「どこまで進めたか」を見せる`distance`にします。報酬を変えたら、両方を見比べます。

#### 実習4：起動引数と読み込み、`GAAgent`の寿命（理解して使う）

**起動引数。** デモは2通りの起動に対応します。

```powershell
.\build\Debug\sphere_aidemo.exe                                   # results\から最終世代を自動で選ぶ
.\build\Debug\sphere_aidemo.exe results\seed42_g0_distance.sil    # 指定したファイルを再生する
```

`wWinMain`の3つ目の引数`PWSTR`には、コマンドライン全体が1つの文字列で入っています。これを`main`の`argv`のように分けるのが`CommandLineToArgvW`です（`<shellapi.h>`、`shell32`）。戻り値は`LocalFree`で解放する決まりなので、例外で抜けても解放されるように小さな型で包みます。

```cpp
// 差し込み例（理解して使う）：無名名前空間に追加。<shellapi.h>と<memory>をinclude。
struct LocalFreeDeleter {
    void operator()(wchar_t** p) const { LocalFree(p); }
};

// 起動引数の1つ目（なければ空）を返す。
std::filesystem::path firstArgument() {
    int count = 0;
    std::unique_ptr<wchar_t*, LocalFreeDeleter> args(CommandLineToArgvW(GetCommandLineW(), &count));
    if (!args || count < 2) return {};
    return args.get()[1];   // [0]はexe自身のパス
}
```

`std::filesystem::path`は`wchar_t`の文字列から直接作れるので、日本語を含むパスも変換せずに受け取れます。`std::unique_ptr`に解放方法（デリーター）を渡すと、`delete`の代わりに`LocalFree`を呼ばせられます。第5章の`ComPtr`と同じ「寿命が終わったら自動で解放する」考え方です。`wWinMain`の`PWSTR`引数は使わないので、引数名は書かないままにします。

**`GAAgent`の寿命。** `GAAgent`は、コンストラクターで受け取った`Genome`を**ポインターで指しているだけ**で、コピーを持ちません（`GAAgent.hpp`の`const Genome* genes_`）。そのため、遺伝子を持つ`SavedGenome`がAgentより先に破棄されると、Agentは無効なメモリを読みます。

```cpp
// 間違い：一時オブジェクトの遺伝子を指してしまう。
GAAgent agent{ loadGenome(path).genes };   // この行の終わりでSavedGenomeは破棄される
```

これを避けるため、メッセージループより前で、次の順に宣言します。C++のローカル変数は宣言の逆順に破棄されます。`SavedGenome`を先に、`GAAgent`を後に宣言すれば、Agentが先に消えます。

```cpp
// 差し込み例（理解して使う）：wWinMainで、ウィンドウとRendererを作る前。
// デモはAIを動かすことだけが目的なので、読み込めなければ起動時に止める。
std::filesystem::path genomePath = firstArgument();
if (genomePath.empty()) {
    const auto latest = findLatestGenome("results", "seed42", "distance");
    if (!latest) throw std::runtime_error("results/ has no seed42_g*_distance.sil");
    genomePath = *latest;
}
const SavedGenome saved = loadGenome(genomePath.string());   // modelText()の不一致などは例外になる
GAAgent agent{ saved.genes };                                // savedより後に宣言する
const std::string genomeName = genomePath.filename().string();
```

`sphere_game`と違い、デモでは読み込みの失敗は致命的です。ここで投げた例外は、`wWinMain`の既存の`catch`で`MessageBoxA`に表示されます。ウィンドウを作る前に読み込むので、ファイルがないときに空の窓が一瞬出ることもありません。

ImGuiの既定フォントは日本語の字形を持たないので、画面に出す文字列は英語にします。上の例外メッセージを英語にしたのも同じ理由です（`MessageBoxA`でも、`/utf-8`のバイト列をANSIとして表示するため日本語は化けます）。

**作業フォルダー。** `"results"`は、実行時の作業フォルダーからの相対パスです。リポジトリの直下で`.\build\Debug\sphere_aidemo.exe`と起動すれば`results\`が見つかります。Visual Studioのデバッガーから起動すると、作業フォルダーがビルドフォルダーになることがあります。見つからないときは、エラーメッセージに`std::filesystem::current_path().string()`を足して確かめてください。

#### 実習5：固定時間更新の中でAIに行動を決めさせる（課題8-C：自分で書く）

ここがこの節の中心です。`sphere_game`のループは、人間の入力を**1フレームに1回**`readDirec()`で読み、`while (accumulator >= dt)`の中で同じ行動を繰り返し使っています。人間ならそれで問題ありません。しかしAIは学習時、`evaluate`の中で**1ステップごとに**`agent.nextDirec(env)`を呼んでいました。そして`decisionSteps`（現在4）ステップに1回だけ表を引いていました。

```cpp
// Episode.cpp（学習時）の更新の形。デモでも同じ順番にする。
Direc d = agent.nextDirec(env);
lastResult = env.step(d);
```

もしデモで`nextDirec`をフレームに1回しか呼ばないと、どうなるでしょうか。画面の更新が遅れて1フレームに2ステップ進んだとき、Agentの内部カウンター`currentStep_`と環境のステップ数がずれます。すると学習時と違う観測で表を引き、保存時の成績を再現できません。**`nextDirec`は`env.step`の直前に、必ず1回ずつ呼ぶ**のが条件です。

**手順。**

1. 環境の初期化を`env.reset(saved.seed)`にする。`saved.seed`は学習時に保存した環境シードです。現在は`initialSeed`と同じ42ですが、`--seed`を変えて学習したファイルを読んだときに条件がずれないようにするためです。続けて`agent.reset()`も呼ぶ。
2. `while (accumulator >= sil::Environment::dt)`の内側で、`agent.nextDirec(env)`の結果を`env.step`に渡す。ループの外で行動を読んでおく処理は削除する。
3. 1回の試行が終わったら、`env.reset(saved.seed)`と`agent.reset()`を**両方**呼ぶ。コピー元の`advanceGame`は`env.reset`しか呼ばないので、`GAAgent&`も受け取るように書き直すか、終了したかどうかを`bool`で返して呼び出し側でリセットする。
4. Start/Stopボタンを残した場合、Startを押したときも`env`とAgentを両方リセットする。途中からAIに渡すと、学習時には存在しなかった状態から始まります。
5. ImGuiがキーボードを使っているかどうか（`WantCaptureKeyboard`）の判定は不要になる。デモではキー入力を読まないためです。

**考えてみる。** `GAAgent::reset()`は`currentStep_`だけを0に戻し、`current_`は前の試行の値のままです。それでも次の試行の最初の行動が正しく決まるのはなぜでしょうか。`nextDirec`の`if`の条件から説明してください。

#### 実習6：ImGuiで情報を表示する（課題8-D：自分で書く）

コピー元の`"Stage selection"`の窓を、`"AI demo"`などの名前の窓へ書き換えます。使う関数は第5章で使った`ImGui::Text`と`ImGui::Button`、それに`ImGui::Separator()`（区切り線）です。

- 読み込んだファイル：`genomeName`、世代番号`saved.generation`、環境シード`saved.seed`。
- 保存時の成績：`saved.metrics.bestZ`、`saved.metrics.surviveSeconds`、`StateToStr(saved.metrics.end)`。
- 現在の試行：到達距離`env.bestZ()`、経過秒数`env.surviveSec()`、残り時間`env.remainSec()`。
- 終わった試行の結果の履歴（コピー元の`lastResults`を使える）。毎回同じ結果が並べば、再現できている証拠になります。

**表示する数値の書式。** `ImGui::Text`は`printf`と同じ書式です。`float`は`%.2f`、`int`は`%d`、`std::uint32_t`は`%u`、文字列は`%s`と`.c_str()`を使います。`std::string`をそのまま渡すと未定義動作になります。

#### 確認方法と完了条件

| 確認すること | 方法 | 期待する結果 |
| --- | --- | --- |
| `sphere_game`が変わっていない | `sphere_game.exe`を起動してWASDで遊ぶ | 第5章と同じに動く |
| 最終世代が選ばれる | 引数なしで起動し、ファイル名の表示を見る | `seed42_g50_distance.sil`（現在の`results/`の場合） |
| 学習時の成績を再現する | 最初の1回が終わったときの到達距離、経過秒数、終了理由を、「保存時の成績」と比べる | 一致する |
| CLIの評価と一致する | `.\build\Debug\sphere_experiment.exe evaluate --input results\seed42_g50_distance.sil --seed 42` | 上と同じ`bestZ`・秒数・終了理由 |
| 繰り返しても同じ | 3回連続で試行を見る | 毎回同じ動き・同じ結果 |
| 画面の都合で結果が変わらない | 試行中にウィンドウサイズを変え、別アプリへ切り替えて戻る | 結果は変わらない（止まっている間は時間が進まないだけ） |
| 初期世代との違いが見える | `sphere_aidemo.exe results\seed42_g0_distance.sil` | 最終世代より手前で止まる、またはぶつかる |
| 読み込み失敗の扱い | 存在しないファイル名を引数に渡す | エラーのメッセージボックスが出て終了する |

**再現しないときに疑う順番。**

1. **`nextDirec`を呼ぶ位置。** 実習5のとおり、`env.step`の直前に1ステップ1回か。
2. **リセット漏れ。** 起動時・試行の終わり・Startで、`env`とAgentを両方リセットしているか。シードは`saved.seed`か。
3. **学習後にステージを変えていないか。** 現在の保存形式（`SIL_GENOME_V1`）は`modelText()`だけを照合し、ステージ（`makeStage()`）は保存していません。学習した後で障害物の配置・速度や`maxSteps`を変えると、読み込みには成功しても別の条件で動きます。ステージを変えたら`train`をやり直して、ファイルを作り直してください。8.2の`stageText()`を足せば、この食い違いも読み込み時に検出できます（追加課題）。
4. **ビルド構成の違い。** 学習をRelease、デモをDebugで行った場合、浮動小数点の計算順序が変わって結果がずれる可能性は否定できません。一致しないときは、同じ構成の`sphere_experiment evaluate`の結果と比べて原因を切り分けます。一致を確かめていない構成同士で「同じになる」とは書かないでください。

**完了条件：** `sphere_game`には変更がない。`sphere_aidemo`を引数なしで起動すると、最終世代のAIが学習時と同じ条件で動く。最初の試行の結果が、保存時の成績および`sphere_experiment evaluate`の結果と一致する。引数でファイルを指定すれば、ほかの世代も再生できる。9.2の「保存済みAI」の行と、9.3の比較用の録画（初期・途中・最終世代）にこのexeを使えます。

**追加課題（任意）。**

1. ImGuiのコンボボックス（`ImGui::BeginCombo`）で、`results/`内の全世代を切り替えられるようにする。読み込み直すときは、`GAAgent`も新しい遺伝子で作り直す（古い`SavedGenome`を指したままにしない）。
2. 再生速度（1倍・4倍）を選べるようにする。`accumulator`へ足す`elapsed`を倍にするだけで、`dt`は変えない。変えると学習時と別の物理になります。
3. 共通のウィンドウ処理を`src/app/Win32Window.hpp`／`.cpp`へ切り出し、`sphere_game`と`sphere_aidemo`の重複をなくす。

<a id="chapter-9"></a>

## 第9章 検証と作品への仕上げ

### 9.1 速さを測る

学習が遅いと感じても、何に時間が掛かるかは測るまで分かりません。まずReleaseビルドで同じ個体の評価時間を測ります。

**確認用の差し込み例：ExperimentMain.cppなどの一時的な計測箇所。`<chrono>`をincludeする。**

```cpp
const auto begin = std::chrono::steady_clock::now();
sil::ActionTable sample{};
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

個体数64、エリート4、トーナメント3、要素ごとの突然変異（行動の置き換え）確率0.01はコード内の教材設定です。CLIで変更するオプションは実装していません。保存は初期、中間、最終世代に行い、適応度最大と到達距離最大を別名で保存します。

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

第5章は2026年10月22日に、5-Hまでの既習本文を残し、定型部分をコピーし、ゲームによって変わる処理を自分で書く構成へ改稿した。章構成、差し込み先、現在のコアAPI・依存ヘッダーとの対応、コードフェンスを静的に確認した。コピー用の基盤と自力実装の骨組みを含む教材であり、改稿後のWindowsビルド・リンク・画面表示は未検証。上記の2026年10月20日の検証記録は、改稿後の第5章の動作保証を表すものではない。

第6〜8章は2026年10月25日に、ニューラルネットワークの重みを個体とする方式から、周辺8マスの観測番号ごとに行動を定めた行動表を個体とする方式へ改稿した。あわせて第6〜7章と第8章の保存処理は、関数名・型・引数・手順の説明だけを示す形式へ変更した。改稿後の内容は、どの環境でもビルド・実行していない。上記の2026年10月20日の検証記録は、改稿前のニューラルネットワーク版に対するものである。

2026年10月26日に、第7章の冒頭へ7.1「フォルダとファイルを再構成する」を追加し、第7〜9章のファイルパスと名前（`ActionTable`、`TableAgent`、`TableFile`、`GeneticTrainer`、`sphere_ai`、`ai_tests`）を再構成後のものへ変更した。章の構成と名前の対応を静的に確認しただけで、ビルド・実行はしていない。

2026年10月29日に、8.6「最終世代のAIをウィンドウでデモ再生する」を追加し、10月30日に、手動プレイの`sphere_game`とは別の実行ファイル`sphere_aidemo`（`src/app/AiDemoMain.cpp`に独自の`wWinMain`）を作る形へ書き直した。現在のリポジトリの`GAAgent`、`SavedGenome`、`loadGenome`、`WindowsMain.cpp`、`Episode.cpp`の呼び出し順と照らし合わせて静的に書いたもので、本文の手順による実装・ビルド・画面での再生は確認していない。
