# Agent instructions

## Project

This repository contains the Tuya T5AI firmware and orchestration decisions for the MOB competition prototype.

- Project root: `/home/akira/Projects/advx26`
- Canonical firmware application: `firmware/`
- Backend service: `backend/` Git submodule
- Mobile client: `clients/soundpola-app/` Git submodule
- TuyaOpen SDK: external checkout at `/home/akira/SDKs/TuyaOpen-v1.9.0`
- Code intelligence: CodeGraph in `.codegraph/` (database remains local and ignored)

## Agent skills

### Issue tracker

Issues and Wayfinder decision maps may use the local Markdown tracker under `.scratch/`, but `.scratch/` is private working state: never add or commit it. See `docs/agents/issue-tracker.md`.

### Triage labels

Use the default local triage vocabulary. See `docs/agents/triage-labels.md`.

### Domain docs

This is a single-context repository using root `CONTEXT.md` and `docs/adr/`. See `docs/agents/domain.md`.

## Wayfinder interaction

When interviewing the user, ask multiple questions in one round when they are genuinely independent and can be answered in parallel. Split questions into later rounds only when one answer changes the available options or meaning of another question.

Wayfinder produces local decisions and planning artifacts by default. Keep those interaction artifacts under ignored `.scratch/`; durable product decisions belong in `CONTEXT.md`, ADRs, specifications, source code, or tests.

## Development workflow

1. Decide whether the previous development slice should be committed.
2. State the next development plan.
3. Implement code.
4. Run syntax and relevant build checks.
5. Show the complete branch structure, not only the current branch.
6. Provide optional build, flash, or validation commands when useful.

Only operate on hardware identities explicitly assigned to this workstream. Treat additional connected boards as unmanaged: do not flash them, open their serial ports, or infer roles without direct user approval.

Commit messages use lowercase `fix` or `feat`, followed by a module and a Chinese description, for example:

```text
feat(media): 实现云端媒体清单解析
```
