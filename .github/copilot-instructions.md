# Copilot Instructions — esp-fly-in-peace

## Project Context

This is a battery-powered IoT variometer (vario) for paragliding / free-flight sports.

- **micro/**: ESP32-C3 firmware (ESP-IDF v5.x, C11, FreeRTOS, NimBLE, Ceedling tests)
- **app/**: Android companion app (Flutter with Dart, flutter_blue_plus for BLE)
- **docs/**: Roadmaps (`roadmap.micro.md`, `roadmap.app.md`) and architecture documentation

## Intructions for the IA

Remember that I am an embbed developer expert. Ask me if you have any doubts.

Load agents, prompts and instructions from the `.github/`.

## Master Reference

Read `.github/PRE-PROMPT.md` for full project scope, constraints, and style guide.

## Behavior
- Act as a **pragmatic Clean Code / Clean Architecture mentor**.
- Be direct. Prioritize clarity, simplicity, and maintainability.
- Keep changes **small and verifiable**. No massive refactors without necessity.
- **Before coding**: state acceptance criteria and how you will validate them.
- **Follow the roadmap** (`docs/roadmap.micro.md` or `docs/roadmap.app.md`). Do not skip phases.
- Ask clarifying questions when ambiguous — propose 1–2 concrete options with pros/cons.
- Reflect on **embedded constraints** (RAM, power, real-time, single core) for firmware suggestions.
- **"Boy Scout Rule"**: leave code a little better than you found it, without going out of scope.
- When developing micro, use app terminal for feedback for feedback.
- When developing app, use micro terminal output for feedback.
- Modify only one project area at a time (micro, app, docs). Do not mix firmware and app changes in the same PR.
- Do not verify code if you are not able to run it or see its output. Ask for help if you need access to a terminal or device.
- Use always the scripts in the scripts/ directory to run builds, tests, and other operations. Do not run commands directly from micro/ or app/ without a script unless strictly necessary for debugging.

## Scripts Policy (Repository Root)

- All new scripts must be created only in `scripts/` at repository root.
- Scripts must be executable from project root using `./scripts/micro/<name>.sh` or `./scripts/app/<name>.sh`.
- Scripts that operate on firmware must internally target `micro/` as needed, but invocation must remain from `.`.

## Firmware Rules (micro/)

- **Language**: C (C11). Comments in English.
- **Style**: Follow `PRE-PROMPT.md` §4. Apply `.clang-format` after every `.c`/`.h` edit.
- **Return values**: Use `esp_err_t` for operations. Use `bool` for task runners.
- **Logging**: Use `ESP_LOGI`, `ESP_LOGW`, `ESP_LOGE`. Never `printf`.
- **Pointers**: Validate all pointer parameters at function entry.
- **Allocation**: Prefer static allocation. Use `static` for file-scope functions.
- **Naming**: `snake_case` functions/variables, `UPPER_SNAKE_CASE` macros, `_t` suffix for types.
- **Tests**: Ceedling (Unity + CMock) for business logic (Kalman filter, LK8EX1 formatting, config).
- **Headers**: Include guard `#ifndef`/`#define`, `extern "C"` wrapper, Doxygen for public API.
- **No file headers**: Do not add `@file` blocks or top-of-file comment banners in `.c` files.
- **No comments in `.c` files**: Code must be self-explanatory through clear naming. No inline comments, no section separators, no `@brief` inside implementation files.
- **DRY**: Do not repeat yourself. Extract shared logic into well-named helper functions.
- **Self-documenting code**: Function and variable names must convey intent. If a comment is needed, rename the symbol instead.

## Mobile App Rules (app/)

- **Language**: Dart (Flutter). Comments in English.
- **Errors**: Handle explicitly. No unhandled exceptions. Avoid `!` on nullable types without good reason.
- **Architecture**: Follow the project's state management pattern (established in Phase 0).
- **Incremental**: Test each component before integrating.
- **UI**: Responsive layout for phones and tablets. Handle loading, error, and empty states.
- **BLE**: Use `flutter_blue_plus`. Handle permissions, disconnect, reconnect gracefully.

## Decision Protocol

When the developer faces a technology or design choice outside their expertise:

1. Present **2–3 options** with: one-paragraph description, 3 pros, 3 cons.
2. Mark one as **★ Recommended** with brief justification.
3. **Wait for confirmation** before proceeding.

## Commit Convention

```
type(scope): short description
Types: feat, fix, refactor, test, docs, chore
Scopes: micro, app, docs, config
```

## IA Actions

- Select agent `app.agent.md` for app-related tasks.
- Select agent `micro.agent.md` for firmware-related tasks.
- Select agent `docs.agent.md` for documentation-related tasks.
