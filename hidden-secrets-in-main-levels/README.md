# Hidden Secrets in Main Levels (`janis25.secrets_in_main_levels`)

Built for **Geode SDK v6.0.0** (Geometry Dash `2.2091`, C++23).

## Implemented Mechanics
- **Stereo Madness (`ID: 1`) & Secret Madness**:
  - Automatic injection of secret Toggle Orb (`Group ID 1`) in official *Stereo Madness*.
  - `GJBaseGameLayer::toggleGroup` hook to arm the hidden sequence.
  - `1.0s` post-completion freeze followed by custom `FLAlertLayer` confirmation popup (`Reverse Secret`).
  - Full frame-by-frame timeline rewind (`X`, `Y`, rotation, vehicle state, gravity, camera, progress bar) synchronized with pre-processed reverse audio streams (`FFmpeg areverse`).