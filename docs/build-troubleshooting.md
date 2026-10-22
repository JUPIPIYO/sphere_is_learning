# WindowsのCMake異常終了とビルド手順

## 通常のビルド

プロジェクト直下のPowerShellで実行します。

```powershell
.\scripts\build.ps1
.\build\Debug\sphere_game.exe
```

`build.ps1`はゲーム・ヘッドレス・テストをビルドします。`BUILD_TESTING=ON`を設定し、`-RunTests`を付けるとCTestでテストも実行します。

```powershell
.\scripts\build.ps1 -RunTests
.\scripts\build.ps1 -Configuration Release
.\scripts\build.ps1 -Target sphere_tests -RunTests
```

CMakeはPATHから探し、見つからなければVisual Studio Installerの`vswhere`で検出します。必要なら`-CMakePath 'C:\...\cmake.exe'`を指定できます。`build`は既存のジェネレーターとCPU設定を使います。新規フォルダーでは選択されたCMakeの既定値を使うため、特定の構成が必要なら先にその構成で生成してください。

## headlessのときに動き、描画を接続してから止まった理由

2026年10月23日のローカル調査結果です。

- 以前のCMakeは`enable_testing()`と`add_test()`で自分のテストを登録し、DirectXTKを`add_subdirectory()`していませんでした。
- 描画用のCMakeを接続すると、DirectXTKの`CMakeLists.txt`から`include(CTest)`が実行されます。
- `BUILD_TESTING=ON`ではCTestのダッシュボード用ターゲット設定も読み込まれ、`CTestTargets.cmake`の`find_program(CMAKE_CTEST_COMMAND ...)`に到達します。
- このPCでは、値が設定済みの変数への`find_program()`が、日本語を含む現在の作業ディレクトリからの実行時に`0xC0000409`で異常終了しました。ゲームやDirectXTKのない最小のCMakeスクリプトでも再現しました。
- 同じスクリプトを`C:\Windows`から実行すると成功しました。ソース・ビルド先に日本語が含まれていても、絶対パスで指定した元のプロジェクトの設定生成は成功し、再実行も成功しました。
- Visual Studio同梱の`3.31.6-msvc6`と、公式配布の`3.31.12`の両方で最小再現を確認しました。CMake内部の故障箇所・修正版は未特定です。

`BUILD_TESTING=OFF`は問題の設定経路を飛ばすため成功しますが、自分のテストも無効になるので恒久的な対策にはしません。ビルド用スクリプトは実行中の作業ディレクトリだけを変え、終了時・失敗時とも元に戻します。ファイルの移動やOS設定の変更は行いません。

これはCMake本体を修正するものではなく、確認済みの条件を避ける回避策です。Visual Studioのフォルダー表示から直接行うCMake生成には適用されません。当面はPowerShellからスクリプトでビルドしてください。

## Git履歴との関係

`e9a2de4`ではビルド生成物のGit追跡をやめています。生成物は各環境で再生成するもので、この整理自体が今回の異常終了の原因だという証拠はありません。同コミットには各ターゲットへ入口・描画ソースを混在させる変更もありますが、それは通常のコンパイル・リンクに関わる別問題で、現行CMakeでは所属が整理されています。

直接の発生契機は、現行の未コミット変更にある`add_subdirectory(external/DirectXTK)`の追加です。Git pushやCloud側の外部設定がこのPCのCMakeを変更したことを示す証拠は、リポジトリ履歴からは確認できません。

## テストの失敗との区別

設定生成・ビルド・テスト実行は別の段階です。調査時の`simulation`テストは実行できましたが、`reflection keeps overshoot`で失敗しました。これはCMakeの異常終了ではなく、障害物の更新後時刻に関するシミュレーションの検証結果です。ビルド修繕でテストの期待値やゲームの挙動は変更しません。

参考：[CTest](https://cmake.org/cmake/help/v3.31/module/CTest.html)、[find_program](https://cmake.org/cmake/help/v3.31/command/find_program.html)。
