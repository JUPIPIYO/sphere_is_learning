# AGENTS.md — Game AI Simulator

## Project Overview

This project develops a game simulation and AI training framework in modern C++.

The application is a simple obstacle-avoidance game inspired by the general gameplay concept of *The World's Hardest Game*.

The player starts at the bottom of an arena and must reach a goal at the top while avoiding moving obstacles.

The game is simulated in a two-dimensional plane but rendered using Direct3D 11 in a three-dimensional scene.

The project's primary purpose is to demonstrate C++ software engineering, game simulation architecture, graphics programming, and AI algorithm implementation.

Read `docs/handoff.md` before proposing major architectural changes or beginning substantial implementation work.

## Technology Stack

- Language: C++20
- Build system: CMake
- Graphics: Direct3D 11
- Graphics helpers: DirectXTK
- Development GUI: Dear ImGui
- AI: Initial implementation using genetic algorithms
- Testing: Automated C++ tests
- Version control: Git / GitHub

Additional dependencies must be justified before introduction.

## Development Environments

Two environments are planned:

1. Physical Windows 11 PC with an Intel x64 processor — primary development, simulation and AI execution, Direct3D 11 validation, and performance evaluation.
2. MacBook Pro with Apple M4 — supplementary source editing, portable C++ development, and testing of simulation and AI components.

VMware Fusion and Windows ARM are outside the current scope.

The project must support different CPU architectures without hard-coded architecture-specific library paths.

Windows-specific rendering code must not prevent the simulation and AI components from building on macOS.

## Architectural Principles

### 1. Separate simulation and rendering

The game simulation must not depend on DirectX, window handles, GUI libraries, or operating-system-specific graphics APIs.

Rendering must consume simulation state without modifying game behavior.

### 2. Support headless execution

The simulator must support execution without a window or graphical output.

AI training should run without real-time frame rate restrictions.

### 3. Use deterministic simulation

Prefer a fixed simulation timestep.

All randomness must be controlled through explicit seeds.

Given the same initial state, seed, and action sequence, the simulation should produce reproducible results within the same supported execution environment.

Cross-platform floating-point reproducibility is not assumed automatically and must be tested before being claimed.

### 4. Keep the AI interface modular

AI agents must interact with the environment through clearly defined observations and actions.

The framework should support replacing an AI algorithm without modifying the simulator's internal mechanics.

### 5. Prefer simple implementation

Avoid unnecessary abstractions, dependencies, and premature optimization.

Do not build a general-purpose game engine. Implement only the features required for the simulator and its intended extensions.

### 6. Testing

Add tests for critical simulation behavior, including:

- Movement and collision detection
- Reset and terminal conditions
- Deterministic execution
- Observation generation
- Agent action application

Use benchmarks when evaluating optimization claims.

## Coding Guidelines

- Use modern C++20 features where appropriate.
- Prefer RAII and standard-library containers.
- Use smart pointers only when ownership requires them.
- Avoid unnecessary dynamic allocation in high-frequency simulation loops.
- Use descriptive identifiers and consistent naming.
- Keep function calls on one line where reasonably readable.
- Keep platform-specific code isolated.
- Treat compiler warnings seriously.
- Avoid large unrelated refactors.

## Agent Workflow

### 役割分担

- コーディングは原則としてユーザー本人が担当する。
- エージェントの通常の担当範囲は、要件の精査、設計方針の考案・説明、およびサンプルコードの提示とする。
- サンプルコードは原則としてチャット内に提示し、目的、設計上の理由、組み込み方、検証方法を説明する。
- ユーザーから対象作業の実装・修正を明示的に依頼されない限り、ソースコード、テスト、ビルド設定を作成・変更しない。サブエージェントに実装を委譲する場合も同じ制約を適用する。
- 文書の作成・更新は、ユーザーが依頼した範囲で行う。
- 開発手順や他の文書に「実装する」とある場合も、この役割分担に従い、通常はユーザーが実装するための設計・サンプルコードを提示する。

Before proposing substantial features, or implementing them when explicitly requested:

1. Read the relevant specifications.
2. Inspect the existing implementation.
3. Explain important design choices and trade-offs.
4. Propose a small, verifiable implementation step.
5. Present sample code and implementation guidance for the user. Modify implementation files only when explicitly requested.
6. Explain relevant validation steps; when implementation is explicitly requested, build and run relevant tests where the environment permits.
7. Summarize the proposal or changes, actual validation results, and remaining issues. Distinguish suggested checks from checks actually performed.

Do not claim that a Windows build or Direct3D rendering was verified unless it was actually tested in a suitable Windows environment.

Do not silently modify the project's architecture or add dependencies without explaining why.

Do not automatically push commits, rewrite Git history, or delete significant project files without explicit authorization.

## Documentation

Maintain:

- `README.md` — public project overview and setup instructions
- `docs/handoff.md` — background, progress, and development roadmap
- `docs/specification.md` — game rules and AI environment requirements
- `docs/architecture.md` — technical design and major interfaces

Update relevant documents when requirements or architecture change.

## Intellectual Property

The project must use original game assets, level layouts, naming, and code.

Do not include Nintendo or World's Hardest Game copyrighted assets or reproduce their distinctive visual designs.

Only the general concept of navigating an obstacle-avoidance environment should be used as inspiration.

## Current Priority

Prioritize the following:

1. Establish a portable C++20/CMake project.
2. Define the headless simulation architecture.
3. Implement a minimal playable environment.
4. Integrate Direct3D 11 rendering.
5. Implement agent interaction and a genetic algorithm.
6. Add training evaluation and visualization.

The job application deadline is November 16, 2026, with approximately 15 development hours per week planned. The deadline deliverable must demonstrate gameplay improvement through genetic-algorithm training on one stage, save learning results, and run saved agents with graphics. Stage completion is not required. Keep the initial obstacles to horizontal or vertical reciprocating motion within the playing plane; additional obstacle types and generalization evaluation are later extensions.
