---
description: Cut an Oh My DJ release (patch, minor or major), signed + notarized
argument-hint: patch | minor | major
---

Cut an Oh My DJ release. Bump level: `$ARGUMENTS` — it must be exactly `patch`, `minor` or `major`; if it's anything else (or empty), stop and ask.

Work directly on `main`: the version bump is committed and tagged on main.

## 1. Preflight

- The working tree must be clean and the current branch must be `main`. Run `git pull --ff-only origin main`. Abort on any failure.
- Run the logic tests and abort if anything is red — never release over failing tests:
  - `clang++ -std=c++17 -Isrc tests/test_rotation.cpp src/rotation-step.cpp src/duration.cpp -o /tmp/ohmydj-tests && /tmp/ohmydj-tests`
  - `clang++ -std=c++17 -Isrc tests/test_presets.cpp src/stream-presets.cpp -o /tmp/ohmydj-preset-tests && /tmp/ohmydj-preset-tests`

## 2. Compute the new version

- Current version: the top-level `"version"` in `buildspec.json` (the plugin's own, e.g. `0.1.0` — NOT the `version` fields inside `dependencies`).
- Derive the next `X.Y.Z` for the requested bump yourself (don't change anything yet).

## 3. Bump the version

- Set the top-level `"version"` in `buildspec.json` to the new `X.Y.Z`.
- Commit it on its own with a descriptive title only (no body, no prefix): `Release X.Y.Z`.

## 4. Tag and push

- Create an annotated tag: `git tag -a X.Y.Z -m "Oh My DJ X.Y.Z"`.
- Push both: `git push origin main && git push origin X.Y.Z`.
- The tag (matching `X.Y.Z`) triggers `.github/workflows/build-project.yaml` with `notarize:true`: it signs every artifact, notarizes + staples the macOS `.pkg`, and publishes a **draft** GitHub Release with the macOS `.pkg`, Windows `.zip` and Linux `.deb` (+ dbgsym).

## 5. Release notes and publish

- Confirm CI started: `gh run list --workflow=push.yaml --limit 1`. The macOS job is the one that signs + notarizes; it takes several minutes. Poll briefly if asked — don't block indefinitely on `gh run watch`.
- Curate notes from `git log <prev-tag>..HEAD --oneline --no-merges` (use `git describe --tags --abbrev=0 HEAD^` for the previous tag; if there is none, it's the first release). Keep ONLY user-facing items — new features and meaningful changes; drop refactors, tests, CI/release plumbing and dependency bumps. Collapse related commits. Write for a DJ, not for a developer.
- Set them on the draft: `gh release edit X.Y.Z --notes "..."`.
- Leave it as a draft for review, unless asked to publish — then `gh release edit X.Y.Z --draft=false`.

## 6. Report

- Report: the new version, the curated changelog items, the Actions URL, and the release URL. A complete release attaches the signed + notarized `oh-my-dj-X.Y.Z-macos-universal.pkg` plus the Windows and Linux assets.
