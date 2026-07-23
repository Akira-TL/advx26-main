# Issue tracker: Local Markdown

Issues, specifications, and Wayfinder maps for this repository live as Markdown files under `.scratch/`.

## Conventions

- One effort per directory: `.scratch/<effort-slug>/`
- Wayfinder map: `.scratch/<effort-slug>/map.md`
- Specification: `.scratch/<feature-slug>/spec.md`
- Tickets: `.scratch/<effort-slug>/issues/<NN>-<slug>.md`, numbered from `01`
- Ticket type: `Type: research|prototype|grilling|task`
- Ticket state: `Status: open|claimed|resolved`
- Blocking dependencies: `Blocked by: NN, NN`
- Comments append under `## Comments`

## Wayfinding operations

- **Map:** `.scratch/<effort>/map.md`
- **Child ticket:** `.scratch/<effort>/issues/NN-<slug>.md`
- **Frontier:** open, unblocked, and unclaimed child tickets, ordered by number
- **Claim:** change `Status: open` to `Status: claimed` before work
- **Resolve:** append an `## Answer`, set `Status: resolved`, then add a one-line context pointer to the map's `## Decisions so far`

Independent human decisions may be asked in one batch. Dependent decisions remain in later rounds or use `Blocked by` edges.
