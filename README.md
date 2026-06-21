# Oh My DJ

An OBS plugin that makes life simple for DJs: a single dock to rotate cameras /
scenes on a timer, and (planned) one-click multistreaming to several platforms.

It replaces the heavyweight, untranslated setup of *obs-multi-rtmp* +
*Advanced Scene Switcher* with one focused, friendly panel.

One dock, two tabs: **Cameras** and **Streaming**.

## Status

- [x] **Scene rotation** — define a flow of scenes, each shown for a chosen
  amount of *seconds / minutes / hours*, then loop. Per step you can override
  what happens when the time ends: advance to the next scene (loop) or jump to a
  specific scene.
- [x] **Multistream** — push the OBS output to Twitch, YouTube, etc. at once.
  One shared encoder feeds every destination (one encode, many outputs), so
  extra platforms cost bandwidth, not CPU. One button goes live everywhere.

### Scene rotation — how it works

The dock shows a table of steps:

| Scene | Duration | Unit | When it ends |
|-------|----------|------|--------------|
| CAM 1 | 2 | minutes | Next scene (loop) |
| CAM 2 | 30 | seconds | Next scene (loop) |
| CAM 3 | 1 | minutes | Next scene (loop) |

Press **Start rotation** and Oh My DJ switches the program scene on schedule,
looping back to the top. "When it ends" defaults to *next scene*; pick a scene
instead to jump straight to it (e.g. always return to your main camera).

The flow is saved per OBS profile in `oh-my-dj.json`.

### Multistream — how it works

The **Streaming** tab is a list of destinations:

| Platform | Server (RTMP URL) | Stream key | On | Status |
|----------|-------------------|------------|----|--------|
| Twitch | rtmp://live.twitch.tv/app | •••• | ☑ | ● LIVE |
| YouTube | rtmp://a.rtmp.youtube.com/live2 | •••• | ☑ | ● LIVE |

Set a video bitrate, press **Go live everywhere**, and Oh My DJ starts one RTMP
output per enabled destination, all sharing a single video/audio encode. Status
updates live per destination. Saved per OBS profile in `oh-my-dj-stream.json`.

## Testing

The pure rotation logic (durations, loop/jump resolution) has no OBS/Qt
dependency and is unit-tested on its own:

```bash
cmake -S tests -B tests/build
cmake --build tests/build
ctest --test-dir tests/build --output-on-failure
```

## Building

This project uses the standard [obs-plugintemplate](https://github.com/obsproject/obs-plugintemplate)
build system (CMake + obs-deps + Qt6). Dependencies are declared in
`buildspec.json` and fetched into `.deps` by the CMake preset.

```bash
# macOS
cmake --preset macos
cmake --build --preset macos

# Windows
cmake --preset windows-x64
cmake --build --preset windows-x64

# Linux
cmake --preset ubuntu-x86_64
cmake --build --preset ubuntu-x86_64
```

Requires Qt6 and the OBS frontend API (both enabled by default in
`CMakeLists.txt`). The built module goes into your OBS `obs-plugins` directory.
