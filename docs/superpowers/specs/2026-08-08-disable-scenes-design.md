# Disable scenes without deleting them

## Problem

A rotation config is built once and reused across sessions. Some nights call for
three cameras, some for two, some for one. Today the only way to drop a camera
from the flow is to remove its row, which throws away its duration, its
`When it ends` target and its transition — all of which have to be typed back in
the next time that camera is wanted. Users rebuild the config instead of
reusing it.

## Goal

Let each row in the rotation table be switched off and back on, keeping every
one of its settings intact while it is off.

## Core rule

**A disabled step behaves as if it were not in the config at all.**

This single rule decides every question below. There is no state in which a
disabled scene participates "a little".

## Model

`RotationStep` (`src/rotation-step.hpp`) gains one field:

```cpp
bool enabled = true;   // false => the engine ignores this step entirely
```

Defaulting to `true` means newly added steps and steps built by the wizard are
active without any of that code being touched.

## Persistence

`src/rotation-engine.cpp`, alongside the existing `amount_max` handling.

- **Save:** `obs_data_set_bool(o, "enabled", step.enabled)`.
- **Load:** call `obs_data_set_default_bool(o, "enabled", true)` before reading.

The default matters and is not optional. `obs_data_get_bool` returns `false` for
a missing key, so reading it directly would make every scene in a config written
by 0.6.2 or earlier appear disabled after upgrading. The explicit default makes
a missing key read as `true`.

## Engine

Four independent paths reach a step. All four must respect the flag; covering
only the first would leave shuffle and `When it ends` selecting disabled scenes.

1. **`ResolveNextIndex`** (`src/rotation-step.cpp`) — walking forward in order,
   skip over disabled steps rather than taking `fromIndex + 1` unconditionally.
   Returns `-1` when no enabled step exists, which makes the engine `park()`.

2. **`ResolveShuffleIndex`** (`src/rotation-step.cpp`) — currently takes a
   `count` and picks within `[0, count)`. It changes to take the list of
   eligible (enabled) indices, so a disabled step can never be drawn. The
   existing "don't repeat current, and don't repeat previous once there are
   at least 3 candidates" behaviour is preserved, now measured against the
   number of *enabled* steps.

3. **`onExpire`** ("When it ends") — when the named target scene belongs to a
   disabled row, do not jump to it; fall through to the next enabled step. This
   mirrors what already happens when the target scene is no longer in the flow.

4. **`evaluate()` / `IndexOfScene`** (`src/rotation-engine.cpp`) — switching to a
   disabled scene by hand in OBS must not start the rotation. `IndexOfScene`
   ignores disabled steps, so the lookup misses and the engine parks, exactly as
   it would for a scene that was never configured.

## UI

`src/rotation-dock.cpp`. A new leading column holds one checkbox per row,
following the pattern introduced in `515fcb3` for the wizard's secondary scene
picker.

- The `Column` enum gains `ColEnabled = 0`; the existing entries shift by one.
  `ColCount` continues to size the table.
- Both locale files (`data/locale/en-US.ini`, `data/locale/es-ES.ini`) get a
  header string for the new column.
- `addRow` creates the checkbox, sets it from `step.enabled`, and connects it to
  `onEdited` like every other editor in the row.
- `collectSteps` reads it back into `step.enabled`.
- A disabled row is greyed out, but its controls stay editable, so a camera that
  is currently off can still have its duration set up in advance.

`onMove` rebuilds rows from `collectSteps()`, so reordering carries the flag with
no extra work. Removing, adding and renaming need no changes.

## Testing

The engine logic is pure and already covered in `tests/test_rotation.cpp`, so it
is driven test-first there:

- forward order skips a disabled step
- shuffle never returns a disabled step
- `onExpire` pointing at a disabled scene falls through to the next enabled one
- every step disabled yields `-1`
- a config with no `enabled` key loads as enabled (regression guard for the
  `obs_data_set_default_bool` default)

`ResolveShuffleIndex` changes signature, so its existing tests are modified, not
just extended. This is unavoidable given that shuffle must respect the flag.

## Out of scope

- No warning or status text when every scene is disabled. That state parks the
  engine and shows the existing `Waiting` status, which is enough.
- No locking of controls on a disabled row.
- No changes to the wizard (`suggest-dialog`): it produces fresh plans whose
  steps are all enabled by default.

## Behaviour that falls out for free

- **All rows disabled** — equivalent to an empty config: the engine parks and
  nothing switches.
- **Exactly one row enabled** — the flow stays on that scene. `ResolveNextIndex`
  resolves back to the single enabled step, so the timer expiring is a no-op.
  This is the "tonight I only want one camera" case, and it needs no special
  handling.
