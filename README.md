# jack_fofs

## Build

### Release (recommended)

Using presets (keeps build artifacts separate):

```sh
cd .. #(repository root)
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

If `jfofs --version` shows an older version after you build, you are likely
still running an older binary already installed in `/usr/local/bin`.

- System-wide install (updates `/usr/local/bin/jfofs`): `sudo cmake --install
  build-release`
- User-local install (no sudo): reconfigure with
  `-DCMAKE_INSTALL_PREFIX=$HOME/.local`, then `cmake --install build-release`
  and ensure `$HOME/.local/bin` is before `/usr/local/bin` in `PATH`.

## Clock Sources

The online engine supports two clock sources:

- `1`: JACK frame time. This is the default and matches the previous behavior.
- `2`: JACK transport frame time. This follows the JACK transport position
  instead of the client frame clock.

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

The transport-backed clock flushes queued online events when the transport
position jumps or when the clock source changes at runtime. This keeps the queue
aligned with the new transport timeline after locate, loop, or similar
discontinuities.

`JFOFS_CLOCK_OFFLINE_FREE_RUNNING` is reserved for the offline engine and is not
accepted by the online `jfofs` API.

## jfofs_add Timing

`jfofs_add` takes an absolute timestamp in microseconds on the currently
selected online clock. It does not mean "delay this many microseconds from now".
In normal use, compute it as the current jfofs time plus some lead time:

```c
uint64_t t = jfofs_get_time(jfofs) + lead_us;
int status = jfofs_add(jfofs, t,
                       ampl, freq, gliss, phi, beta, alpha, amin,
                       cutoff, pan1, pan2, pan3);
```

Internally, `time_us` is converted to an absolute frame number:

```text
start_frame = floor(time_us * sample_rate / 1000000)
```

The control client advances `next_frame` once per audio cycle. The queue then
places the event into one buffer-sized slot of a ring buffer.

An event is accepted into the normal queue when its frame lies in this range:

```text
next_frame <= start_frame < next_frame + n_slots * buffer_size
```

This gives three outcomes:

- `start_frame < next_frame`: the event is late and `jfofs_add` returns
  `JFOFS_FOF_LATE_WARNING`.
- `next_frame <= start_frame < next_frame + n_slots * buffer_size`: the event is
  queued normally and `jfofs_add` returns `JFOFS_SUCCESS`.
- `start_frame >= next_frame + n_slots * buffer_size`: the event is reported as
  `JFOFS_FOF_EXCESS_INFO`.

At the moment, `excess` should be treated as outside the usable scheduling
window. The queue stores those events in the `excess` list, but the code still
marks excess handling as TODO, so they are not part of the normal playable slot
ring.

Expressed directly in microseconds, the accepted non-late, non-excess window is:

```text
ceil(next_frame * 1000000 / sample_rate)
  <= time_us <
ceil((next_frame + n_slots * buffer_size) * 1000000 / sample_rate)
```

There is one practical caveat near the lower bound: if you target the next
imminent buffer, the queue can still race with `next_frame` advancing and return
`JFOFS_FOF_LATE_WARNING`. In practice, schedule at least one buffer ahead, and
preferably a few buffers ahead. The tests use a lead time of four buffers.

Queued online events are also flushed if the clock source changes or the JACK
transport timeline jumps. That keeps scheduling aligned with the new transport
position, but it also means previously accepted future events may be discarded
on transport discontinuities.
