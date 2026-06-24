# Forum submission — Oh My DJ

Paste the text below into the OBS Resources submission. It is written to comply
with the OBS Forum Resource and IP Policy (license, branding, AI disclaimure).

Resource name to use: **Oh My DJ** (do NOT prefix or include "OBS" in the name).

---

## Oh My DJ — camera/scene rotation and multistream for DJs

> **Third-party plugin.** Oh My DJ is not part of OBS Studio and is not
> developed by, affiliated with, or endorsed by the OBS Project.
>
> **AI assistance:** AI coding tools were used to assist during development.
> All code is written, reviewed, tested, and validated by a human author.

Oh My DJ adds a single dock with two tabs — **Cameras** and **Streaming** — that
covers the two things a streaming DJ does most: rotate cameras/scenes on a timer
and push the stream to several platforms at once. Both features are
**automatic**: you only enable them, there are no manual Start/Stop buttons. The
plugin reacts to OBS events.

### Scene rotation
Define a flow of scenes, each shown for a chosen amount of seconds / minutes /
hours, then loop. Per step you can override what happens when the time ends:
advance to the next scene, or jump to a specific scene. Enable *Automatic
rotation* and the flow engages on its own whenever the program scene matches a
step. The flow is saved per OBS profile.

### Multistream
Push the output to Twitch, YouTube, Facebook Live, etc. at the same time. It
reuses OBS's own encoder, so it inherits OBS's resolution and bitrate with no
extra CPU cost. Enable *Sync with OBS streaming* and enabled destinations go
live when you press **Start Streaming** in OBS and stop when you stop it. Status
updates live per destination.

### Install
Download the build for your platform from GitHub Releases and place the module
in your OBS `obs-plugins` directory (the release notes include per-platform
instructions). macOS builds are signed and notarized.

- Releases: https://github.com/vigosan/oh-my-dj/releases
- Source code: https://github.com/vigosan/oh-my-dj

### License
GPL-2.0-or-later. As a plugin linking against libobs it is a derivative work of
OBS Studio and inherits its license. Full source is in the repository above.

### Reporting bugs
Please report issues on the GitHub issue tracker:
https://github.com/vigosan/oh-my-dj/issues
You may also use this resource thread for questions.

---

## Checklist before re-submitting

- [ ] Repository is public and up to date (it is).
- [ ] `LICENSE` (GPL-2.0) present in the repo root — DONE.
- [ ] License stated in README — DONE.
- [ ] AI-assistance disclaimer present in the description AND README — DONE.
- [ ] Name is "Oh My DJ" — no "OBS" in the resource name.
- [ ] Do not use the OBS logo in your icon/marketing assets.
- [ ] Link the download to GitHub Releases (a reputable host), not temporary
      file hosts.
- [ ] Submission is a stable release (you are on 0.2.1).
- [ ] Description is in English (this one is).
