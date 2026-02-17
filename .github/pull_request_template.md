## Summary

- What changed?
- Why was it needed?
- Scope boundaries (what is intentionally not included)?

## Phase & Roadmap

- Roadmap affected: `docs/roadmap.micro.md` / `docs/roadmap.app.md`
- Phase/Task(s):
- Roadmap status updated: [ ] Yes [ ] No [ ] N/A

## Validation

- Build/Run checks executed:
- Tests executed:
- Manual validation executed:
- Result: [ ] Pass [ ] Fail (explain)

## Conductor-Model-Hardware (Required)

Reference:
- `docs/architecture/conductor-model-hardware-template.md`
- `docs/architecture/conductor-model-hardware-pr-checklist.md`

Quick checklist:
- [ ] 1. Public entry points call conductor only (no direct hardware access from tasks/UI/controllers).
- [ ] 2. Conductor handles input validation and maps failures to public error contract.
- [ ] 3. Model contains business rules/state transitions only (no ESP-IDF/plugin imports).
- [ ] 4. Hardware layer is thin and I/O focused (drivers/adapters), without domain logic.
- [ ] 5. Dependency direction is clean: `conductor -> model`, `conductor -> hardware`, never inverse.
- [ ] 6. Duplicate orchestration or logic was removed/refactored (Boy Scout Rule applied).
- [ ] 7. Names and boundaries are explicit (`*_conductor`, `*_model`, `*_hardware` or equivalent folders).
- [ ] 8. Model behavior is covered by focused tests (Ceedling/Flutter unit tests as applicable).
- [ ] 9. Integration path was validated after refactorization (build/run + phase validation checks).
- [ ] 10. Roadmap evidence/status was updated for the phase/task impacted by this PR.

## Risks & Rollback

- Potential risks:
- Rollback plan:

## Notes for Reviewer

- Priority review areas:
- Open questions:
