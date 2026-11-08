# DLSS5 Geode

An experimental Geometry Dash / Geode project for Android 2.2081.

The goal is to explore lightweight real-time visual effects inspired by modern rendering:
- glow and bloom-style highlights
- color grading
- optional visual quality presets
- performance-conscious settings for Android

**Important:** this is not NVIDIA DLSS 5 and does not use NVIDIA hardware features. It is an independent visual-effects project.

## Build

GitHub Actions builds the Android 64-bit target when code is pushed or when the workflow is run manually. The resulting `.geode` package is uploaded as a workflow artifact.

## Current status

The repository currently contains the Geode project scaffold and Android build workflow. The visual effects are not implemented yet; they will be added and tested incrementally to avoid shipping an untested graphics hook.
