# Contributing to Janis25-mods

Thank you for wanting to collaborate on **Hidden Secrets in Main Levels** and **New Gamemodes Animations**!

## How to Contribute (Pull Requests)
1. **Fork** this repository (`https://github.com`).
2. Clone your fork locally and build with **Geode SDK v6.0.0** (C++23, Geometry Dash `2.2091`):
   ```bash
   cmake -B build
   cmake --build build --config RelWithDebInfo
   ```
3. Create a feature branch (`git checkout -b feat/new-level-secret`).
4. Commit your changes and open a **Pull Request** describing the level ID, trigger group, and tested behavior.

## Open Ideas
- Adding new reverse courses & custom secrets for official main levels (`Dry Out`, `Polargeist`, `Dash`, etc.).
- Adding new procedural animations for Vehicle modes (`Wave`, `Swing`, `Spider`).