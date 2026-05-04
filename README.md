# jack_fofs

## Clock Sources

The online engine supports two clock sources:

- `1`: JACK frame time. This is the default and matches the previous behavior.
- `2`: JACK transport frame time. This follows the JACK transport position instead of the client frame clock.

Server selection:

```sh
jfofs --clock_source 1
jfofs --clock_source 2
```

Runtime API selection:

```c
jfofs_set_clock_source(jfofs, JFOFS_CLOCK_JACK_FRAME_TIME);
jfofs_set_clock_source(jfofs, JFOFS_CLOCK_JACK_TRANSPORT);
```

The transport-backed clock flushes queued online events when the transport position jumps or when the clock source changes at runtime. This keeps the queue aligned with the new transport timeline after locate, loop, or similar discontinuities.

`JFOFS_CLOCK_OFFLINE_FREE_RUNNING` is reserved for the offline engine and is not accepted by the online `jfofs` API.

## Build

### Release (recommended)

Using presets (keeps build artifacts separate):

```sh
cd ..
cmake --preset release
cmake --build --preset release
sudo cmake --install build-release
```

Alternative (presets from within `src/`):

```sh
cd src
cmake --preset release
cmake --build --preset release
sudo cmake --install ../build-release
```

Without presets:

```sh
cmake -S src -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release -j
sudo cmake --install build-release
```

### Debug

Using presets:

```sh
cmake --preset debug
cmake --build --preset debug
```

Without presets:

```sh
cmake -S src -B build-debug -DCMAKE_BUILD_TYPE=Debug -DDEBUG_ENABLE=ON
cmake --build build-debug -j
```

### RelWithDebInfo

Using presets:

```sh
cmake --preset relwithdebinfo
cmake --build --preset relwithdebinfo
```

Without presets:

```sh
cmake -S src -B build-relwithdebinfo -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-relwithdebinfo -j
```

## Install

```sh
 cmake --build build-release --target install
 ```

If `jfofs --version` shows an older version after you build, you are likely still running an older binary already installed in `/usr/local/bin`.

- System-wide install (updates `/usr/local/bin/jfofs`): `sudo cmake --install build-release`
- User-local install (no sudo): reconfigure with `-DCMAKE_INSTALL_PREFIX=$HOME/.local`, then `cmake --install build-release` and ensure `$HOME/.local/bin` is before `/usr/local/bin` in `PATH`.
