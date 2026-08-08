# Disable Scenes Without Deleting Them — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let each row of the rotation table be switched off and back on, keeping its duration, jump target and transition intact while it is off.

**Architecture:** A single `bool enabled` field on `RotationStep`, defaulting to `true`. The rule is "a disabled step behaves as if it were not in the config at all", enforced at the four points where the engine can reach a step: ordered advance, shuffle advance, `onExpire` jump, and the manual-scene-change lookup. The UI adds a leading checkbox column.

**Tech Stack:** C++17, Qt 6 (`QTableWidget`, `QCheckBox`), libobs (`obs_data_*` for config JSON), hand-rolled `CHECK` macro tests run under CTest.

## Global Constraints

- **Language standard:** C++17. Tests must build with no Qt and no libobs — `tests/` compiles only `duration.cpp`, `rotation-step.cpp`, `rotation-plan.cpp`, `stream-presets.cpp` and `stream-health.cpp`.
- **No code comments** beyond the explanatory style already present in these files (block comments above functions describing behaviour are the existing convention and are expected; inline noise is not).
- **Formatting:** tabs for indentation, matching every file in `src/`.
- **Backward compatibility:** a config JSON written by v0.6.2 or earlier has no `enabled` key. It MUST load with every step enabled.
- **Locales:** every user-facing string exists in BOTH `data/locale/en-US.ini` and `data/locale/es-ES.ini`.
- **Commits:** descriptive title only — no body, no `feat:`/`fix:` prefix.
- **Build/test command** (`cmake` is not installed in this environment; compile directly):
  ```bash
  c++ -std=c++17 -Isrc tests/test_rotation.cpp src/duration.cpp src/rotation-step.cpp -o /tmp/ohmydj-rot && /tmp/ohmydj-rot
  ```
  If `cmake` IS available, prefer:
  ```bash
  cmake -S tests -B tests/build && cmake --build tests/build && ctest --test-dir tests/build --output-on-failure
  ```

---

## File Structure

| File | Responsibility | Change |
|---|---|---|
| `src/rotation-step.hpp` | `RotationStep` struct + pure resolver declarations | Add `enabled` field; change `ResolveShuffleIndex` signature |
| `src/rotation-step.cpp` | Pure advance logic | Filter disabled in both resolvers |
| `src/rotation-engine.cpp` | Engine, save/load | Filter in `IndexOfScene`; pass candidates to shuffle; persist `enabled` |
| `src/rotation-dock.cpp` | Qt table UI | New `ColEnabled` column with checkbox |
| `data/locale/en-US.ini`, `data/locale/es-ES.ini` | Strings | New column header |
| `tests/test_rotation.cpp` | Pure-logic tests | New cases; update shuffle call sites |

Tasks 1–3 are pure logic and fully covered by tests. Task 4 is Qt UI, verified by build + manual check.

---

### Task 1: Add the `enabled` field and skip disabled steps in ordered advance

**Files:**
- Modify: `src/rotation-step.hpp:12-21` (struct), `src/rotation-step.cpp:15-32` (`ResolveNextIndex`)
- Test: `tests/test_rotation.cpp`

**Interfaces:**
- Consumes: nothing (first task).
- Produces: `RotationStep::enabled` (`bool`, default `true`). `ResolveNextIndex(const std::vector<RotationStep> &steps, int fromIndex)` keeps its signature but now returns only enabled indices, or `-1` when none are enabled.

- [ ] **Step 1: Write the failing tests**

Add these three functions to `tests/test_rotation.cpp`, placed after `test_resolve_duplicate_target()` (around line 153). Note the helper `disabledStep`, which the later tasks reuse.

```cpp
static RotationStep disabledStep(const char *scene, int amount, DurationUnit unit,
				 const char *onExpire = "")
{
	RotationStep s = step(scene, amount, unit, onExpire);
	s.enabled = false;
	return s;
}

static void test_resolve_skips_disabled()
{
	std::printf("a disabled step is passed over as if it were not there\n");
	std::vector<RotationStep> flow = {
		step("CAM1", 5, DurationUnit::Seconds),
		disabledStep("CAM2", 5, DurationUnit::Seconds),
		step("CAM3", 5, DurationUnit::Seconds),
	};
	CHECK(ResolveNextIndex(flow, 0) == 2); // CAM2 is off, so CAM1 leads to CAM3
	CHECK(ResolveNextIndex(flow, 2) == 0); // wrap still lands on an enabled step

	// Consecutive disabled steps are all skipped in one hop.
	std::vector<RotationStep> gap = {
		step("CAM1", 5, DurationUnit::Seconds),
		disabledStep("CAM2", 5, DurationUnit::Seconds),
		disabledStep("CAM3", 5, DurationUnit::Seconds),
		step("CAM4", 5, DurationUnit::Seconds),
	};
	CHECK(ResolveNextIndex(gap, 0) == 3);
}

static void test_resolve_all_disabled()
{
	std::printf("with every step disabled there is nothing to advance to\n");
	std::vector<RotationStep> flow = {
		disabledStep("CAM1", 5, DurationUnit::Seconds),
		disabledStep("CAM2", 5, DurationUnit::Seconds),
	};
	CHECK(ResolveNextIndex(flow, 0) == -1); // engine parks
	CHECK(ResolveNextIndex(flow, -1) == -1);
}

static void test_resolve_single_enabled_holds()
{
	std::printf("one enabled step holds the flow on that scene\n");
	// The "tonight I only want one camera" case: the timer expiring resolves
	// back to the same step, so the scene never changes.
	std::vector<RotationStep> flow = {
		disabledStep("CAM1", 5, DurationUnit::Seconds),
		step("CAM2", 5, DurationUnit::Seconds),
		disabledStep("CAM3", 5, DurationUnit::Seconds),
	};
	CHECK(ResolveNextIndex(flow, 1) == 1);
}
```

Register them in `main()` after `test_resolve_duplicate_target();`:

```cpp
	test_resolve_skips_disabled();
	test_resolve_all_disabled();
	test_resolve_single_enabled_holds();
```

- [ ] **Step 2: Run the tests and verify they fail**

Run:
```bash
c++ -std=c++17 -Isrc tests/test_rotation.cpp src/duration.cpp src/rotation-step.cpp -o /tmp/ohmydj-rot && /tmp/ohmydj-rot
```
Expected: compile error — `'struct ohmydj::RotationStep' has no member named 'enabled'`. That is the red phase; the field does not exist yet.

- [ ] **Step 3: Add the field**

In `src/rotation-step.hpp`, inside `struct RotationStep`, add after the `transition` member (line 18):

```cpp
	bool enabled = true;                     // false => the engine skips this step entirely
```

- [ ] **Step 4: Re-run and confirm the failure moved**

Run the same command.
Expected: it now compiles, and the run reports `FAIL` lines for the three new tests (e.g. `ResolveNextIndex(flow, 0) == 2`) plus `3 check(s) failed.`-style output. The struct exists; the logic does not.

- [ ] **Step 5: Implement the skip**

Replace the body of `ResolveNextIndex` in `src/rotation-step.cpp` (lines 15-32) with:

```cpp
int ResolveNextIndex(const std::vector<RotationStep> &steps, int fromIndex)
{
	const int count = static_cast<int>(steps.size());
	if (count == 0)
		return -1;

	const bool validFrom = fromIndex >= 0 && fromIndex < count;

	if (validFrom) {
		const std::string &target = steps[fromIndex].onExpire;
		if (!target.empty()) {
			for (int i = 0; i < count; ++i) {
				if (steps[i].enabled && steps[i].scene == target)
					return i;
			}
			// Target is disabled or no longer in the flow: fall through to next.
		}
	}

	// Walk forward from the current position, wrapping, until an enabled step
	// turns up. Starting at 1 keeps a lone enabled step resolving onto itself
	// only after every other candidate has been rejected.
	const int from = validFrom ? fromIndex : count - 1;
	for (int hop = 1; hop <= count; ++hop) {
		const int candidate = (from + hop) % count;
		if (steps[candidate].enabled)
			return candidate;
	}
	return -1; // nothing enabled
}
```

Note what changed beyond the skip: an out-of-range `fromIndex` used to return `0` immediately, which would hand back a disabled step. It now starts the walk at `count - 1` so the first candidate examined is index `0`, preserving the old "recover to the start" behaviour while still honouring the flag.

- [ ] **Step 6: Run the tests and verify they pass**

Run the same command.
Expected: `All rotation tests passed.` — the new cases AND every pre-existing case (`test_resolve_loop`, `test_resolve_jump`, `test_resolve_edge_cases`, `test_resolve_repeated_scene_cycle`) must be green, since all their steps default to `enabled = true`.

- [ ] **Step 7: Commit**

```bash
git add src/rotation-step.hpp src/rotation-step.cpp tests/test_rotation.cpp
git commit -m "Skip disabled steps when advancing the rotation in order"
```

---

### Task 2: Keep shuffle away from disabled steps

**Files:**
- Modify: `src/rotation-step.hpp:37-44` (declaration), `src/rotation-step.cpp:34-61` (`ResolveShuffleIndex`), `src/rotation-engine.cpp:163-172` (`advance`)
- Test: `tests/test_rotation.cpp:155-191` (existing shuffle tests are rewritten)

**Interfaces:**
- Consumes: `RotationStep::enabled` from Task 1.
- Produces: `int ResolveShuffleIndex(const std::vector<RotationStep> &steps, int current, int previous, int roll)` — replaces the old `(int count, int current, int previous, int roll)`. It derives its own candidate list from the enabled steps.

**Why the signature changes:** the function only ever received a `count`, so it had no way to tell an enabled step from a disabled one. Passing the steps themselves is the smallest change that lets it filter, and it matches how `ResolveNextIndex` already takes the vector.

- [ ] **Step 1: Rewrite the existing shuffle tests for the new signature**

In `tests/test_rotation.cpp`, replace the four functions `test_shuffle_skips_current`, `test_shuffle_skips_previous_when_three_or_more`, `test_shuffle_roll_walks_candidates` and `test_shuffle_edge_cases` (lines 155-191) with the following. The helper `flowOf` builds a plain enabled flow of N steps so the old expectations stay readable.

```cpp
static std::vector<RotationStep> flowOf(int count)
{
	std::vector<RotationStep> flow;
	for (int i = 0; i < count; ++i)
		flow.push_back(step("CAM", 5, DurationUnit::Seconds));
	return flow;
}

static void test_shuffle_skips_current()
{
	std::printf("shuffle never picks the current step\n");
	// 2 steps: from either one, the only valid pick is the other.
	CHECK(ResolveShuffleIndex(flowOf(2), 0, -1, 0) == 1);
	CHECK(ResolveShuffleIndex(flowOf(2), 1, -1, 0) == 0);
}

static void test_shuffle_skips_previous_when_three_or_more()
{
	std::printf("shuffle avoids the immediately previous step once there are >=3\n");
	// 3 steps, current=1, previous=0 => only candidate left is 2, for any roll.
	CHECK(ResolveShuffleIndex(flowOf(3), 1, 0, 0) == 2);
	CHECK(ResolveShuffleIndex(flowOf(3), 1, 0, 5) == 2); // roll is taken modulo the candidates
}

static void test_shuffle_roll_walks_candidates()
{
	std::printf("shuffle maps roll onto the surviving candidates in order\n");
	// 4 steps, current=0, previous=-1 => candidates are {1,2,3}.
	CHECK(ResolveShuffleIndex(flowOf(4), 0, -1, 0) == 1);
	CHECK(ResolveShuffleIndex(flowOf(4), 0, -1, 1) == 2);
	CHECK(ResolveShuffleIndex(flowOf(4), 0, -1, 2) == 3);
	CHECK(ResolveShuffleIndex(flowOf(4), 0, -1, 3) == 1); // wraps around the candidate list
}

static void test_shuffle_edge_cases()
{
	std::printf("shuffle edge cases\n");
	CHECK(ResolveShuffleIndex(flowOf(0), -1, -1, 0) == -1); // nothing to play
	CHECK(ResolveShuffleIndex(flowOf(1), 0, -1, 0) == 0);   // single scene loops onto itself
	// 2 steps: previous must NOT be excluded (it would leave zero candidates).
	CHECK(ResolveShuffleIndex(flowOf(2), 0, 1, 0) == 1);
	// First pick of a session (current=-1): any step is fair game; roll selects.
	CHECK(ResolveShuffleIndex(flowOf(3), -1, -1, 0) == 0);
	CHECK(ResolveShuffleIndex(flowOf(3), -1, -1, 2) == 2);
}

static void test_shuffle_skips_disabled()
{
	std::printf("shuffle never lands on a disabled step\n");
	// CAM2 is off: from CAM1 the only candidates are CAM3 and CAM4.
	std::vector<RotationStep> flow = {
		step("CAM1", 5, DurationUnit::Seconds),
		disabledStep("CAM2", 5, DurationUnit::Seconds),
		step("CAM3", 5, DurationUnit::Seconds),
		step("CAM4", 5, DurationUnit::Seconds),
	};
	CHECK(ResolveShuffleIndex(flow, 0, -1, 0) == 2);
	CHECK(ResolveShuffleIndex(flow, 0, -1, 1) == 3);
	CHECK(ResolveShuffleIndex(flow, 0, -1, 2) == 2); // wraps over 2 candidates, never 1

	// "Avoid previous" is measured against enabled steps only: with 3 enabled
	// the previous one is excluded, leaving exactly one candidate.
	CHECK(ResolveShuffleIndex(flow, 2, 0, 0) == 3);
}

static void test_shuffle_disabled_edge_cases()
{
	std::printf("shuffle with too few enabled steps\n");
	std::vector<RotationStep> allOff = {
		disabledStep("CAM1", 5, DurationUnit::Seconds),
		disabledStep("CAM2", 5, DurationUnit::Seconds),
	};
	CHECK(ResolveShuffleIndex(allOff, 0, -1, 0) == -1); // nothing enabled => park

	std::vector<RotationStep> oneOn = {
		disabledStep("CAM1", 5, DurationUnit::Seconds),
		step("CAM2", 5, DurationUnit::Seconds),
		disabledStep("CAM3", 5, DurationUnit::Seconds),
	};
	CHECK(ResolveShuffleIndex(oneOn, 1, -1, 0) == 1); // lone enabled step holds
	CHECK(ResolveShuffleIndex(oneOn, -1, -1, 0) == 1); // first pick finds it
}
```

Register the two new tests in `main()`, right after `test_shuffle_edge_cases();`:

```cpp
	test_shuffle_skips_disabled();
	test_shuffle_disabled_edge_cases();
```

- [ ] **Step 2: Run the tests and verify they fail**

Run:
```bash
c++ -std=c++17 -Isrc tests/test_rotation.cpp src/duration.cpp src/rotation-step.cpp -o /tmp/ohmydj-rot && /tmp/ohmydj-rot
```
Expected: compile error — no matching function for the `ResolveShuffleIndex(std::vector<RotationStep>, int, int, int)` calls, since the declaration still takes an `int count`.

- [ ] **Step 3: Update the declaration**

In `src/rotation-step.hpp`, replace the `ResolveShuffleIndex` declaration and its comment block (lines 37-44) with:

```cpp
// Index of the next step in shuffle mode: a random pick among the ENABLED steps
// that avoids `current` and — once there are >= 3 enabled — also avoids
// `previous`, so the flow never stutters back and forth between the same two
// scenes. `roll` is a non-negative integer supplied by the caller (the RNG in
// production); it is mapped onto the surviving candidates, so any value is valid
// and the result is deterministic.
//   - one enabled step  => that step's index (it loops onto itself)
//   - none enabled      => -1
// Pure logic, no OBS/Qt/RNG dependencies, so it is unit-testable on its own.
int ResolveShuffleIndex(const std::vector<RotationStep> &steps, int current, int previous, int roll);
```

- [ ] **Step 4: Implement the filtered pick**

In `src/rotation-step.cpp`, replace the whole `ResolveShuffleIndex` body (lines 34-61) with:

```cpp
int ResolveShuffleIndex(const std::vector<RotationStep> &steps, int current, int previous, int roll)
{
	const int count = static_cast<int>(steps.size());

	std::vector<int> enabled;
	enabled.reserve(count);
	for (int i = 0; i < count; ++i) {
		if (steps[i].enabled)
			enabled.push_back(i);
	}

	if (enabled.empty())
		return -1;
	if (enabled.size() == 1)
		return enabled.front();

	// Exclude the current step; exclude the previous one too, but only while
	// that still leaves something to pick (i.e. once there are >= 3 enabled).
	const bool excludePrevious = enabled.size() >= 3;

	std::vector<int> candidates;
	candidates.reserve(enabled.size());
	for (int i : enabled) {
		if (i == current)
			continue;
		if (excludePrevious && i == previous)
			continue;
		candidates.push_back(i);
	}

	if (candidates.empty())
		return current; // degenerate guard; should not happen given the rules above

	const int size = static_cast<int>(candidates.size());
	return candidates[((roll % size) + size) % size];
}
```

- [ ] **Step 5: Update the engine's call site**

In `src/rotation-engine.cpp`, `advance()` currently computes a `roll` from `count` and passes `count`. Replace lines 165-172:

```cpp
	const int count = static_cast<int>(steps_.size());
	int next;
	if (shuffle_) {
		const int roll = count > 0 ? static_cast<int>(rng_() % static_cast<unsigned>(count)) : 0;
		next = ResolveShuffleIndex(count, current_, previous_, roll);
	} else {
		next = ResolveNextIndex(steps_, current_);
	}
```

with:

```cpp
	const int count = static_cast<int>(steps_.size());
	int next;
	if (shuffle_) {
		const int roll = count > 0 ? static_cast<int>(rng_() % static_cast<unsigned>(count)) : 0;
		next = ResolveShuffleIndex(steps_, current_, previous_, roll);
	} else {
		next = ResolveNextIndex(steps_, current_);
	}
```

The `roll` is still derived from the total step count. That is fine: `ResolveShuffleIndex` takes it modulo its own candidate list, so any non-negative value is valid.

- [ ] **Step 6: Run the tests and verify they pass**

Run the same command.
Expected: `All rotation tests passed.`

- [ ] **Step 7: Commit**

```bash
git add src/rotation-step.hpp src/rotation-step.cpp src/rotation-engine.cpp tests/test_rotation.cpp
git commit -m "Keep shuffle from picking disabled steps"
```

---

### Task 3: Ignore disabled scenes on manual switch, and persist the flag

**Files:**
- Modify: `src/rotation-engine.cpp:63-70` (`IndexOfScene`), `src/rotation-engine.cpp:246-262` (load), `src/rotation-engine.cpp:278-288` (save)

**Interfaces:**
- Consumes: `RotationStep::enabled` from Task 1.
- Produces: config JSON key `"enabled"` (bool) per entry of the `steps` array. Absent key reads as `true`.

There is no pure-logic test for this task: `IndexOfScene` is in an anonymous namespace and the save/load path needs libobs, neither of which the standalone test binary links. It is verified by build plus the manual check in Step 6.

- [ ] **Step 1: Filter the manual-switch lookup**

In `src/rotation-engine.cpp`, replace `IndexOfScene` (lines 63-70) with:

```cpp
// Index of the enabled step showing `scene`. Disabled steps are invisible here,
// so switching to one by hand parks the rotation exactly as an unconfigured
// scene would.
int IndexOfScene(const std::vector<RotationStep> &steps, const std::string &scene)
{
	for (int i = 0; i < static_cast<int>(steps.size()); ++i) {
		if (steps[i].enabled && steps[i].scene == scene)
			return i;
	}
	return -1;
}
```

- [ ] **Step 2: Load the flag with a `true` default**

In `LoadRotationConfig`, inside the `for` loop over the steps array, add this line immediately after `obs_data_t *o = obs_data_array_item(arr, i);` (line 247):

```cpp
			obs_data_set_default_bool(o, "enabled", true);
```

and add this line after the `step.transition = ...` assignment (line 258):

```cpp
			step.enabled = obs_data_get_bool(o, "enabled");
```

**This default is the whole point of the step.** `obs_data_get_bool` returns `false` for a missing key, so without `obs_data_set_default_bool` every scene in a config written by v0.6.2 or earlier would come back disabled after upgrading — the user's flow would silently stop working.

- [ ] **Step 3: Save the flag**

In `SaveRotationConfig`, add after the `obs_data_set_string(o, "transition", ...)` line (line 285):

```cpp
		obs_data_set_bool(o, "enabled", step.enabled);
```

- [ ] **Step 4: Verify the pure tests still pass**

Run:
```bash
c++ -std=c++17 -Isrc tests/test_rotation.cpp src/duration.cpp src/rotation-step.cpp -o /tmp/ohmydj-rot && /tmp/ohmydj-rot
```
Expected: `All rotation tests passed.` (This task does not change pure logic; this confirms nothing regressed.)

- [ ] **Step 5: Verify the plugin still compiles**

Build the plugin the way the project normally does (see `README.md` / `build_macos`). The engine changes must compile cleanly with no new warnings.

- [ ] **Step 6: Manual backward-compatibility check**

Before building, back up the existing config so the old-format file can be restored:
```bash
cp "$HOME/Library/Application Support/obs-studio/basic/profiles/"*/oh-my-dj.json /tmp/oh-my-dj-pre-enabled.json
```
Load the plugin with that pre-existing config in place (it has no `enabled` key) and confirm all four scenes appear checked and the rotation still runs. This is the regression the `obs_data_set_default_bool` default exists to prevent.

- [ ] **Step 7: Commit**

```bash
git add src/rotation-engine.cpp
git commit -m "Persist the per-scene enabled flag and hide disabled scenes from manual switching"
```

---

### Task 4: Add the enable checkbox column to the table

**Files:**
- Modify: `src/rotation-dock.cpp:26` (enum), `:53-55` (header labels), `:208-260` (`addRow`), `:262-281` (`collectSteps`)
- Modify: `data/locale/en-US.ini`, `data/locale/es-ES.ini`

**Interfaces:**
- Consumes: `RotationStep::enabled` from Task 1; the persistence from Task 3.
- Produces: no new API. `collectSteps()` keeps its signature and now fills `enabled`.

- [ ] **Step 1: Add the column to the enum**

In `src/rotation-dock.cpp`, replace line 26:

```cpp
enum Column { ColScene = 0, ColAmount, ColUnit, ColOnExpire, ColTransition, ColCount };
```

with:

```cpp
enum Column { ColEnabled = 0, ColScene, ColAmount, ColUnit, ColOnExpire, ColTransition, ColCount };
```

Every other column shifts by one automatically, and `ColCount` still sizes the table. No other line referencing `ColScene`/`ColAmount`/etc. needs editing — they are all symbolic.

- [ ] **Step 2: Add the header label**

Replace the `setHorizontalHeaderLabels` call (lines 54-56) with the new string as the FIRST entry, keeping the existing ones in order:

```cpp
	table_->setHorizontalHeaderLabels(
		{T("OhMyDj.Col.Enabled"), T("OhMyDj.Col.Scene"), T("OhMyDj.Col.Duration"),
		 T("OhMyDj.Col.Unit"), T("OhMyDj.Col.OnExpire"), T("OhMyDj.Col.Transition")});
```

The key prefix is `OhMyDj.Col.` (not `Column`) — that is the existing convention in both locale files. Only `OhMyDj.Col.Enabled` is new.

- [ ] **Step 3: Add the locale strings**

In `data/locale/en-US.ini`, immediately before `OhMyDj.Col.Scene` on line 17:
```ini
OhMyDj.Col.Enabled="On"
```

In `data/locale/es-ES.ini`, in the same position before `OhMyDj.Col.Scene` on line 17:
```ini
OhMyDj.Col.Enabled="Activa"
```

es-ES translates every column header (`"Escena"`, `"Duración"`, `"Al vencer"`…), so the Spanish label is translated too rather than left as "On".

- [ ] **Step 4: Create the checkbox in `addRow`**

In `addRow`, immediately after `table_->insertRow(row);` (line 211), add:

```cpp
	auto *enabledCell = new QWidget();
	auto *enabled = new QCheckBox(enabledCell);
	enabled->setObjectName("enabled");
	enabled->setChecked(step.enabled);
	enabled->setToolTip(T("OhMyDj.Enabled.Hint"));
	auto *enabledLayout = new QHBoxLayout(enabledCell);
	enabledLayout->setContentsMargins(0, 0, 0, 0);
	enabledLayout->setAlignment(Qt::AlignCenter);
	enabledLayout->addWidget(enabled);
	table_->setCellWidget(row, ColEnabled, enabledCell);
```

The wrapper widget + centered layout is what centres a checkbox in a `QTableWidget` cell; a bare `QCheckBox` would sit flush left against the cell edge.

Add the tooltip string to BOTH locale files:
```ini
OhMyDj.Enabled.Hint="Uncheck to skip this scene without deleting it"
```
es-ES:
```ini
OhMyDj.Enabled.Hint="Desmárcala para saltarte esta escena sin borrarla"
```

`<QCheckBox>` is already included at the top of the file (line 7) — no include change is needed for this step.

- [ ] **Step 5: Grey out a disabled row and wire the signal**

Still in `addRow`, at the end of the function (after the existing `connect` calls, line 259), add:

```cpp
	auto applyEnabledLook = [enabledCell, scene, amountCell, unit, onExpire, transition](bool on) {
		const double opacity = on ? 1.0 : 0.45;
		for (QWidget *w : {static_cast<QWidget *>(scene), amountCell,
				   static_cast<QWidget *>(unit), static_cast<QWidget *>(onExpire),
				   static_cast<QWidget *>(transition)}) {
			auto *effect = qobject_cast<QGraphicsOpacityEffect *>(w->graphicsEffect());
			if (!effect) {
				effect = new QGraphicsOpacityEffect(w);
				w->setGraphicsEffect(effect);
			}
			effect->setOpacity(opacity);
		}
		Q_UNUSED(enabledCell);
	};
	applyEnabledLook(enabled->isChecked());
	connect(enabled, &QCheckBox::toggled, this, [applyEnabledLook](bool on) { applyEnabledLook(on); });
	connect(enabled, &QCheckBox::toggled, this, &RotationDock::onEdited);
```

The controls stay editable by design — only their opacity changes — so a camera that is currently off can still have its duration set up in advance.

Add `#include <QGraphicsOpacityEffect>` to the Qt include block at the top of the file (after `<QComboBox>`, keeping the list alphabetical). It is the one include this task adds.

- [ ] **Step 6: Read the flag back in `collectSteps`**

In `collectSteps`, add inside the row loop, before `steps.push_back(std::move(step));` (line 278):

```cpp
		auto *enabledBox = table_->cellWidget(row, ColEnabled)->findChild<QCheckBox *>(
			QLatin1String("enabled"));
		step.enabled = enabledBox && enabledBox->isChecked();
```

This mirrors the existing `AmountSpin` helper's `findChild` lookup for the spin boxes. Because `onMove` rebuilds every row from `collectSteps()`, reordering carries the flag with no further work.

- [ ] **Step 7: Build and verify manually**

Build the plugin. Then in OBS:
1. Confirm a new leading column of checkboxes appears, all checked, with existing rows unchanged.
2. Uncheck one row — it dims, its controls stay editable.
3. Restart OBS and confirm the row is still unchecked (persistence round-trip from Task 3).
4. Start the rotation and confirm the unchecked scene never comes up, in both `In order` and shuffle.
5. Switch to the unchecked scene by hand and confirm the status returns to `Waiting`.
6. Uncheck all but one row and confirm the flow stays on that scene.

- [ ] **Step 8: Commit**

```bash
git add src/rotation-dock.cpp data/locale/en-US.ini data/locale/es-ES.ini
git commit -m "Add a per-row enable checkbox to the rotation table"
```

---

## Verification

After all four tasks:

```bash
c++ -std=c++17 -Isrc tests/test_rotation.cpp src/duration.cpp src/rotation-step.cpp -o /tmp/ohmydj-rot && /tmp/ohmydj-rot
c++ -std=c++17 -Isrc tests/test_plan.cpp src/rotation-plan.cpp src/duration.cpp src/rotation-step.cpp -o /tmp/ohmydj-plan && /tmp/ohmydj-plan
```

Both must print `All ... tests passed.` `test_plan.cpp` matters here because `BuildRotationPlan` constructs `RotationStep`s: the new field defaults to `true`, so wizard-generated plans stay fully enabled.

If `cmake` is available, `ctest --test-dir tests/build --output-on-failure` should show all four suites passing.

## Out of scope

Per the spec: no warning when every scene is disabled, no locking of controls on a disabled row, no changes to `suggest-dialog`.
