# CLAUDE.md — sphere is learning

Claude Code向けの概要。正式な規約は`AGENTS.md`にあり、内容が食い違う場合は`AGENTS.md`を優先する。大きな設計変更や実装支援の前に`docs/handoff.md`を読むこと。

## プロジェクトの目的

- C++20とDirect3D 11で、障害物を避けて上部のゴールを目指す見下ろし型ゲームを作り、そのゲーム内で遺伝的アルゴリズムによるAI学習を行う。
- ユーザーの動機は2つ。**Direct3Dを学ぶこと**と、**ゲーム開発エンジニア職の就活用ポートフォリオを作ること**（Unity/C#経験はあるが、C++の開発経験を補うのが狙い）。
- 期限は2026年11月16日（ES提出）。週15時間程度。期限時点の必須成果は「1ステージでGAの学習によりプレイが改善する様子を示す」「学習結果を保存する」「保存したAIをグラフィックス付きで動かす」。ステージクリアは必須ではない。

## 役割分担（最重要）

- **コードはユーザー本人が書く。** Claudeの通常の役割は、要件の精査、設計方針の説明、チャット内でのサンプルコード提示、レビュー、デバッグの手がかりの提示。
- ユーザーが明示的に依頼しない限り、`src/`、`tests/`、`CMakeLists.txt`、`scripts/`などのソース・テスト・ビルド設定を作成・変更しない。サブエージェントにも同じ制約を適用する。
- 文書は依頼された範囲でのみ作成・更新する。
- 自動でpush、履歴の書き換え、重要なファイルの削除をしない。
- 学習目的のプロジェクトなので、答えをそのまま渡すより、ユーザーが自分で書いて説明できるよう、理由・手順・確認方法を添えて示す。

## 教科書

`docs/cpp_tutorial.md`が、ユーザーが進めている教科書。章とTODOの対応は次のとおり。

| 章 | 内容 | `docs/todo.md` |
| --- | --- | --- |
| 1〜4 | CMake、C++の読み方、ヘッドレスなシミュレーション、テスト | 手順2〜5 |
| 5 | Win32、Direct3D 11、DirectXTK、固定時間更新、Dear ImGui | 手順6〜8 |
| 6〜8 | 周辺マス観測、NN＋遺伝的アルゴリズム、保存・CLI | 手順9〜10 |
| 9 | 計測、最終確認、上達の比較 | 手順11 |

教科書は「コピー可」「理解して使う」「自分で書く」で課題を分けている。支援時はこの区分を尊重し、「自分で書く」課題の完成コードを先回りして提示しない（求められた場合はヒントから段階的に）。

## 技術スタックと構成

- C++20 / CMake（3.24以上）/ Direct3D 11 / DirectXTK / Dear ImGui / 自作の遺伝的アルゴリズム。
- 依存ライブラリは`external/`のgitサブモジュール（DirectXTK、imgui）。新しい依存の追加は理由を説明してから。
- 主要ディレクトリ：
  - `src/core/` — シミュレーション（`sil::Environment`、`Stage`、`State`、`Direc`、`GameState`）。**Windows・DirectX・ImGuiのヘッダーを含めない。**
  - `src/renderer/directX11/` — `Renderer`（教科書の`d3d11/`とはパスが異なる）。
  - `src/app/` — `WindowsMain.cpp`（ゲーム）、`HeadlessMain.cpp`（描画なし実行）。
  - `tests/` — CTestで実行する自作の軽量テスト。
- CMakeターゲット：`sphere_core`、`sphere_headless`、`sphere_tests`（全OS）、`sphere_game`（Windowsのみ）。

## 設計原則（AGENTS.mdの要約）

1. シミュレーションと描画を分離する。Rendererは`const Stage&`と`const State&`を読むだけで状態を変更しない。
2. ヘッドレス実行に対応し、学習は実時間に縛られずに動く。
3. 固定タイムステップ（1/60秒）と明示的なシードによる決定的な実行。OS間の浮動小数点の一致は検証せずに主張しない。
4. AIは観測と行動（停止＋8方向の9行動）だけで環境とやり取りする。
5. 汎用ゲームエンジンを作らない。必要な機能だけを、単純に作る。
6. 移動・衝突・リセット・終了条件・決定性・観測・行動適用をテストする。

## ビルドとテスト

macOS / Linux（シミュレーションとテストのみ）：

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Windows（PowerShell、ゲームを含む）：

```powershell
.\scripts\build.ps1 -RunTests
.\build\Debug\sphere_game.exe
```

日本語を含むパスでのCMake異常終了の回避策が`scripts/build.ps1`に入っている。経緯は`docs/build-troubleshooting.md`を参照。

## 注意事項

- このクラウド環境（Linux）ではWindowsビルドとDirect3Dの表示を検証できない。実際にWindowsで試していない限り「動作確認済み」と言わない。提案した確認と、実際に行った確認を区別して報告する。
- ソースの文字コードが混在している（`src/core`はBOM付きUTF-8、`src/app`と`src/renderer`はShift-JIS系）。MSVCは`/utf-8`でコンパイルするため、Shift-JISのファイルに日本語の文字列リテラルを入れると文字化けする。
- 知的財産：オリジナルの素材・レベル配置・名称を使う。The World's Hardest Gameや任天堂の素材・特徴的なデザインを再現しない。
- 文書：`README.md`（概要・手順）、`docs/handoff.md`（背景・進捗・計画）、`docs/specification.md`（ルール・AI環境）、`docs/architecture.md`（設計）、`docs/todo.md`（作業チェックリスト）。要件や設計が変わったら、依頼の範囲で関連文書を更新する。
