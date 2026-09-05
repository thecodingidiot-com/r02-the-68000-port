# r02-the-scaler

Companion repository for **r02 — The Scaler on the Mega Drive** at
[thecodingidiot.com](https://thecodingidiot.com) — r01's scaler,
cross-compiled for a real Sega Mega Drive.

---

## Follow my journey

Working through r02 alongside the implementation pages? Build the ROM
step by step, then run the tester.

Clone this repository:

```bash
git clone https://github.com/thecodingidiot-com/r02-the-scaler.git r02-practice
cd r02-practice/solution
bash gen_assets.sh
GENDEV=/opt/gendev make
GENDEV=/opt/gendev BLASTEM=/path/to/blastem bash ../test.sh
```

All tests must pass before the chapter is complete. See the chapter's
own setup page for how to install `gendev` (bundles a real m68k-elf
GCC + SGDK, no root/Docker needed) and BlastEm.

---

## Follow your journey

Building this independently? Here is the full project brief.

r01's exact scaler — a camera projecting billboards by depth — cross-
compiled for the Sega Mega Drive's 68000, with everything the x86
version could take for granted rebuilt from scratch:

- **No FPU.** `float`/`cosf`/`sinf` genuinely fail to link on this
  toolchain (verified, not assumed). `vec2.c`/`camera.c`/`scaler.c`
  are ported to SGDK's `fix32` fixed-point type and its real
  `sinFix32`/`cosFix32` hardware lookup tables — a 1024-step angle,
  not radians.
- **No hardware sprite scaling.** The Mega Drive VDP genuinely cannot
  do it (a Master System VDP feature, dropped for Mega Drive). The
  real, documented technique: draw each billboard at all four VDP
  hardware sizes (8×8 up to 32×32) ahead of time, and pick whichever
  one the depth math lands closest to at runtime.
- **No filesystem.** The road's billboards are a `const` C array
  compiled straight into the ROM, not a text file read at runtime.
- **Steering shifts world position directly** (`cam->right`, never
  rotated), the same model
  [r01](https://github.com/thecodingidiot-com/r01-the-scaler)'s own
  correction settled on — carried straight over rather than
  re-derived, since none of Hang-On, Out Run, or Space Harrier ever
  rotate the camera to steer. `camera_turn()` is untouched and still
  real; `main.c` just doesn't call it any more. `MIN_SIDE`/`MAX_SIDE`
  fence the play area, same range as r01.

Source is split by concern, one file per module:

| File | Contents |
| --- | --- |
| `main.c` | SGDK init, the game loop (joypad → update → render), the one runtime `PAL_setPalette` call |
| `vec2.c` / `vec2.h` | fixed-point 2D vectors — add, subtract, scale, dot |
| `camera.c` / `camera.h` | position, a 1024-step facing angle, and the derived `forward`/`right` axes via `sinFix32`/`cosFix32` |
| `scaler.c` / `scaler.h` | the projection: world position → depth, side, nearest hardware tier, screen position |
| `scene.c` / `scene.h` | the road, as a compiled-in `const` array — no file I/O anywhere |
| `render.c` / `render.h` | the only file that calls SGDK's sprite engine (`SPR_*`) |
| `res/sprites.res` | the eight `SPRITE` resource declarations rescomp compiles into the ROM |
| `gen_assets.sh` | generates the eight source PNGs (two subjects × four hardware sizes) |

Build and test your own version first. Use `solution/` to compare
once you are done, not before.

---

## Building the solution

```bash
cd solution
bash gen_assets.sh
GENDEV=/opt/gendev make
```

Run it in BlastEm:

```bash
blastem out/rom.bin
```

Controls: D-pad Up/Down to drive forward/backward, Left/Right to
steer (fenced, same `MIN_SIDE`/`MAX_SIDE` range
[r01](https://github.com/thecodingidiot-com/r01-the-scaler) uses).

`gen_assets.sh` needs Python3 + Pillow:

```bash
sudo apt install python3-pil
```

---

## What the tester checks

**Build** — the ROM compiles and links with zero warnings from the
real m68k-elf-gcc, and `out/rom.bin` is a genuine, correctly-headered
Genesis ROM image.

**A standalone projection tester** — `vec2.c` and `scaler.c`
recompiled *unmodified* against a small host-side `fix32` shim (real
`gcc`, not `m68k-elf-gcc` — no emulator needed for this part), and
run as an ordinary host program, asserting real numbers: a billboard
straight ahead has `side = 0`; specific depths pick the exact
expected hardware tier (20 world units out picks the 8px tier, 7
picks the 32px tier); a billboard closer than the near plane is not
visible. `camera.c` is not part of this tester — it needs the real
`sinFix32`/`cosFix32` hardware tables, which is exactly why the next
check exists.

**`out/rom.bin`** — boots in BlastEm and runs for four seconds
without crashing. A smoke test, not a visual one — actually driving
down the road and watching billboards jump between hardware sizes as
you approach is done by running it yourself.

---

## License

MIT License. See [LICENSE](LICENSE).
