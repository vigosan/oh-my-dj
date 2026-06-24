# Oh My DJ

An OBS plugin that makes life simple for DJs: a single dock to rotate cameras /
scenes on a timer, and (planned) one-click multistreaming to several platforms.

It replaces the heavyweight, untranslated setup of *obs-multi-rtmp* +
*Advanced Scene Switcher* with one focused, friendly panel.

One dock, two tabs: **Cameras** and **Streaming**.

## Status

Both features are **automatic**: the DJ only enables them. There are no manual
Start/Stop buttons — the plugin reacts to OBS.

- [x] **Scene rotation** — define a flow of scenes, each shown for a chosen
  amount of *seconds / minutes / hours*, then loop. Per step you can override
  what happens when the time ends: advance to the next scene (loop) or jump to a
  specific scene. Tick *Automatic rotation* and the flow engages on its own
  whenever the program scene matches a step.
- [x] **Multistream** — push the OBS output to Twitch, YouTube, etc. at once.
  It reuses OBS's own encoder (so it inherits OBS's resolution/bitrate, zero
  extra CPU) and follows OBS: enabled platforms go live when you press *Start
  Streaming* and stop when you stop it.

### Scene rotation — how it works

The dock shows a table of steps:

| Scene | Duration | Unit | When it ends |
|-------|----------|------|--------------|
| CAM 1 | 2 | minutes | Next scene (loop) |
| CAM 2 | 30 | seconds | Next scene (loop) |
| CAM 3 | 1 | minutes | Next scene (loop) |

Tick **Automatic rotation**. When you switch the program scene to one of the
scenes in the flow, its timer starts and Oh My DJ advances on schedule, looping
back to the top. Switch to a scene that isn't in the flow and it simply waits.
"When it ends" defaults to *next scene*; pick a scene instead to jump straight
to it (e.g. always return to your main camera).

The flow is saved per OBS profile in `oh-my-dj.json`.

### Multistream — how it works

The **Streaming** tab is a list of destinations:

| Platform | Server (RTMP URL) | Stream key | On | Status |
|----------|-------------------|------------|----|--------|
| Twitch | rtmp://live.twitch.tv/app | •••• | ☑ | ● LIVE |
| YouTube | rtmp://a.rtmp.youtube.com/live2 | •••• | ☑ | ● LIVE |

Pick a platform (Twitch, YouTube, Facebook Live, …) and its RTMP server URL is
filled in for you — just paste the stream key and tick it **On**. Enable **Sync
with OBS streaming**: now when you press *Start Streaming* in OBS, every enabled
platform goes live too, reusing OBS's encoder (same resolution/bitrate, no extra
encode), and stops when you stop OBS. Status updates live per destination.
Saved per OBS profile in `oh-my-dj-stream.json`.

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

## License

Oh My DJ is licensed under the GNU General Public License v2.0 or later
(GPL-2.0-or-later); see [`LICENSE`](LICENSE). As a plugin that links against
libobs, it is a derivative work of OBS Studio and inherits its license. The full
source is available in this repository.

## AI assistance disclaimer

Oh My DJ is written and maintained by a human developer. AI coding tools were
used to assist during development. All code is reviewed, tested, and validated
by the author.

Oh My DJ is a third-party plugin. It is not part of OBS Studio and is not
developed by, affiliated with, or endorsed by the OBS Project.
