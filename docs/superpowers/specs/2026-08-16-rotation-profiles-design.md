# Rotation profiles and drag-and-drop reordering

## Problem

The Cameras tab holds a single rotation table. A user who streams different
kinds of shows — a music set one night, an interview the next — has to rebuild
the table by hand each time.

Reordering also costs one click per position: the Up and Down buttons move the
selected row a single step at a time.

## Scope

Two changes to the Cameras tab:

1. Named profiles. Each profile stores the rows of the rotation table. The user
   picks the active profile from a combo box and manages profiles with New,
   Rename and Delete.
2. Drag-and-drop row reordering, alongside the existing Up and Down buttons.

Out of scope: the Streaming tab. Multistream targets stay shared across
profiles. A profile switch never changes where the user broadcasts.

`Automatic rotation` and `Order` (In order / Shuffle) stay global. They are
properties of the session, not of a camera layout, and switching profiles
leaves them untouched.

## Data model

`RotationConfig` gains profiles:

```cpp
struct RotationProfile {
    std::string name;
    std::vector<RotationStep> steps;
};

struct RotationConfig {
    std::vector<RotationProfile> profiles;
    std::string activeProfile;   // name; empty or unknown => first profile
    bool enabled = false;
    bool shuffle = false;
};
```

`RotationStep` does not change.

### On-disk format

```json
{
  "enabled": true,
  "shuffle": false,
  "active_profile": "Music",
  "profiles": [
    { "name": "Music",     "steps": [ ... ] },
    { "name": "Interview", "steps": [ ... ] }
  ]
}
```

`steps` keeps its current per-step keys (`scene`, `amount`, `amount_max`,
`unit`, `on_expire`, `transition`, `enabled`).

### Migration

A config written by an earlier version has `steps` at the root and no
`profiles`. `LoadRotationConfig` reads those steps into a single profile named
`Default` and sets `active_profile` to it.

The new format is written from then on. The root-level `steps` key is not
written. An older build reading the new file finds no `steps` and starts with an
empty table; the data is still in the file and reappears once the newer build
runs again. This is accepted — the format moves forward only.

### Pure helpers

New file `src/rotation-profiles.{hpp,cpp}`, free of OBS and Qt so the test
harness can build it:

```cpp
// Guarantees at least one profile, non-empty unique names, and an
// activeProfile that names an existing profile.
RotationConfig NormalizeProfiles(RotationConfig config);

// A free name derived from base: "Interview", then "Interview 2", ...
std::string UniqueProfileName(const std::vector<RotationProfile> &profiles,
                              const std::string &base);

// Moves the step at `from` to index `to`, preserving every element.
void MoveStep(std::vector<RotationStep> &steps, int from, int to);
```

`LoadRotationConfig` passes its result through `NormalizeProfiles` on both the
new and the migrated path, so the rest of the code never handles a config with
zero profiles or a dangling `activeProfile`.

## UI

### Profile bar

A row above the table in `RotationDock`:

```
Profile: [ Music ▾ ]  [New]  [Rename]  [Delete]
```

- **Selector.** Changing it saves the current table into the profile being left,
  loads the new profile's steps into the table, and pushes them to the engine
  immediately. A profile switch while streaming behaves like an edit to the
  table: the rotation moves to the new profile's first step at once.
- **New.** A `QInputDialog` asks for a name; the profile starts empty. The user
  fills it with `Add scene` or `Suggest...`.
- **Rename.** A `QInputDialog` prefilled with the current name.
- **Delete.** Confirmed with a `QMessageBox`. Disabled while one profile
  remains, so the count never reaches zero.

Names entered blank or already taken are resolved through `UniqueProfileName`
rather than rejected.

### Drag and drop

The table enables internal moves:

```cpp
table_->setDragDropMode(QAbstractItemView::InternalMove);
table_->setDragDropOverwriteMode(false);
table_->setDefaultDropAction(Qt::MoveAction);
```

Qt's own row move cannot be used: every cell is a `cellWidget` (combo boxes and
spin boxes), and those do not travel with the row. A `QTableWidget` subclass
overrides `dropEvent`, computes the source and target rows, emits
`rowMoved(from, to)` and does not call the base implementation.

`RotationDock` handles that signal the way `onMove` already works: collect the
steps, move one, rebuild every row, select the target, persist. `onMove` and the
drop handler are refactored onto a shared `reorderSteps(from, to)`. Up and Down
keep their current behaviour.

### Locale

New keys land in both `data/locale/en-US.ini` and `data/locale/es-ES.ini`:
`OhMyDj.Profile.Label`, `OhMyDj.Profile.New`, `OhMyDj.Profile.Rename`,
`OhMyDj.Profile.Delete`, `OhMyDj.Profile.NewTitle`, `OhMyDj.Profile.NamePrompt`,
`OhMyDj.Profile.DeleteConfirm`, `OhMyDj.Profile.DefaultName`.

## Testing

`tests/test_profiles.cpp`, registered in `tests/CMakeLists.txt` next to the
existing `rotation`, `presets` and `health` targets. Written before the
implementation.

- A legacy config (root `steps`, no `profiles`) migrates to one profile named
  `Default` holding those steps.
- An empty config yields exactly one profile, never zero.
- An `activeProfile` that is empty or names no profile falls back to the first.
- Duplicate and blank profile names resolve to unique non-empty names.
- Deleting the active profile leaves another profile active.
- `UniqueProfileName` returns "Interview 2" when "Interview" is taken.
- `MoveStep` preserves every element and lands the moved step at the target
  index, both moving up and moving down.

The Qt widgets and the OBS `obs_data_*` read/write path cannot run in the
standalone harness. They are covered by manual verification in OBS only: switch
profiles while streaming, rename and delete, drag a row, restart OBS and confirm
the active profile and its rows come back.

## Work order

Three commits, each compiling with tests passing:

1. Profile model, migration and pure helpers, with tests. The dock reads and
   writes the active profile; no profile UI yet.
2. The profile bar in the dock, plus locale keys in both files.
3. Drag and drop, with `onMove` refactored onto `reorderSteps`.
