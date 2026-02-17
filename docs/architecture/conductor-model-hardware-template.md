# Conductor-Model-Hardware — Module Template

> Scope: implementation template to apply from now on in all coding phases.

---

## 1) Pattern intent

Split each module into three responsibilities:

- **Conductor**: orchestration, lifecycle, sequencing, retries, error mapping, concurrency boundaries.
- **Model**: deterministic business logic and state transitions, platform-agnostic.
- **Hardware**: platform adapters and peripheral/plugin calls.

Rule: external callers use conductor entry points only.

---

## 2) Firmware module template (ESP-IDF)

Recommended structure:

```
micro/components/<module>/
├── CMakeLists.txt
├── include/
│   └── <module>.h
└── src/
    ├── <module>_conductor.c
    ├── <module>_model.c
    ├── <module>_hardware.c
    ├── <module>_model.h
    └── <module>_hardware.h
```

If a module is pure algorithm/formatter, `_hardware` can be omitted.

### Responsibilities checklist

- **Conductor**
  - Owns public API from `include/<module>.h`.
  - Validates input pointers and preconditions.
  - Maps internal failures to `esp_err_t`.
  - Coordinates calls to model and hardware.
  - Contains task/mutex/queue-facing orchestration only.

- **Model**
  - No ESP-IDF headers.
  - No peripheral access.
  - Pure logic and state transitions.
  - Unit-testable with Ceedling host tests.

- **Hardware**
  - Encapsulates ESP-IDF drivers/APIs.
  - No business rules.
  - Minimal translation between platform types and model/conductor data.

### Dependency rules

- `conductor -> model`
- `conductor -> hardware`
- `model -X-> hardware`
- `hardware -X-> model` (except shared plain data structs)
- `main/tasks -> conductor` only

### Phase completion checklist (firmware)

- [ ] Module split respects conductor/model/hardware boundaries.
- [ ] Public API remains in conductor.
- [ ] Model has host-side tests for business logic.
- [ ] Hardware is thin and side-effect oriented.
- [ ] Refactorization and validation rerun after split.

---

## 3) App module template (Flutter)

Recommended structure per feature:

```
app/lib/features/<feature>/
├── conductor/
│   └── <feature>_conductor.dart
├── model/
│   ├── <feature>_state.dart
│   └── <feature>_logic.dart
├── hardware/
│   └── <feature>_adapter.dart
└── ui/
    ├── <feature>_screen.dart
    └── <feature>_widgets.dart
```

### Responsibilities checklist

- **Conductor**
  - Orchestrates use-cases, UI-triggered flows, and error mapping.
  - Coordinates model updates and hardware calls.

- **Model**
  - Domain state and pure rules (no plugin calls).
  - Unit-testable without BLE/storage plugins.

- **Hardware**
  - Plugin/platform adapters (`flutter_blue_plus`, storage, permissions).
  - No domain rules.

### Phase completion checklist (app)

- [ ] Feature has explicit conductor/model/hardware split.
- [ ] Model tests do not depend on plugins.
- [ ] Hardware adapters are isolated and replaceable.
- [ ] Validation rerun after refactorization to pattern.

---

## 4) Review checklist (for PR/task closure)

- [ ] Any new module/feature declares conductor/model/hardware responsibilities.
- [ ] Public API or UI entry points do not bypass conductor.
- [ ] Business logic is not mixed with hardware/plugin code.
- [ ] Refactorization completed without functional regression.
- [ ] Phase validation evidence updated in roadmap.

Quick 10-item PR version: `docs/architecture/conductor-model-hardware-pr-checklist.md`.
