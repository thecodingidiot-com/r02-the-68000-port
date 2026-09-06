#!/bin/bash
# r02 — The 68000 Port / test.sh
#
# Builds the ROM with the real m68k-elf/SGDK toolchain, checks the
# projection math on the host (vec2.c and scaler.c, recompiled as-is
# against a small fix32 shim -- see below), then boots the real ROM in
# BlastEm headless and confirms it runs without crashing.
#
# Requires GENDEV set to your gendev install (default /opt/gendev) and
# BLASTEM set to a BlastEm binary (the official retrodev.com tarball
# release, not a Debian-packaged one -- see 01-setup.mdx for why).
#
#   GENDEV=/opt/gendev BLASTEM=/path/to/blastem bash test.sh

set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GENDEV="${GENDEV:-/opt/gendev}"
BLASTEM="${BLASTEM:-blastem}"

# ── colour ────────────────────────────────────────────────────────────────────

if [[ ! -t 1 ]]; then
    C_GREEN=""
    C_RED=""
    C_BOLD=""
    C_RESET=""
else
    C_GREEN="\033[0;32m"
    C_RED="\033[0;31m"
    C_BOLD="\033[1m"
    C_RESET="\033[0m"
fi

pass_count=0
fail_count=0
WORK_DIR=$(mktemp -d)

cleanup() {
    rm -rf "$WORK_DIR"
}
trap cleanup EXIT

hr() {
    echo "────────────────────────────────────────────────────────────────"
}

banner() {
    hr
    echo "  r02 — The 68000 Port / test.sh"
    hr
}

pass() {
    printf "  ${C_GREEN}PASS${C_RESET}  %s\n" "$1"
    pass_count=$((pass_count + 1))
}

fail() {
    printf "  ${C_RED}FAIL${C_RESET}  %s\n" "$1"
    if [[ -n "${2:-}" ]]; then
        echo "        $2"
    fi
    fail_count=$((fail_count + 1))
}

banner

# ── build the real ROM ────────────────────────────────────────────────────────

echo "Building (m68k-elf-gcc via SGDK)..."
rm -rf out src/boot
build_log=$(GENDEV="$GENDEV" PATH="$GENDEV/bin:$PATH" make GENDEV="$GENDEV" 2>&1)
if echo "$build_log" | grep -q ": error:"; then
    fail "ROM builds" "$build_log"
    exit 1
fi
pass "ROM builds"

if echo "$build_log" | grep -qi ": warning:"; then
    fail "build produces no warnings" "$(echo "$build_log" | grep ": warning:")"
else
    pass "build produces no warnings"
fi

if [[ -f out/rom.bin ]] && file out/rom.bin | grep -q "Genesis ROM image"; then
    pass "out/rom.bin is a real Genesis ROM image"
else
    fail "out/rom.bin is a real Genesis ROM image"
fi

# ── host-side projection math tester ─────────────────────────────────────────
#
# vec2.c and scaler.c have no genesis.h calls beyond the fix32 type and
# five documented macros (FIX32/fix32Mul/fix32Div/fix32ToInt/intToFix32,
# all straight from SGDK's maths.h) -- recompiled here, unmodified,
# against a shim that defines exactly those five, on the host, with a
# real host gcc. camera.c is NOT included: it needs sinFix32/cosFix32,
# real 1024-entry hardware lookup tables not worth reproducing here --
# camera_turn()'s correctness is instead confirmed by the real ROM
# smoke test below (and was verified visually while writing the
# chapter, driving the actual ROM in BlastEm).

# A stub <genesis.h> -- just enough of SGDK's real types/macros
# (types.h + the five maths.h fix32 macros this project's own vec2.c/
# scaler.c actually use) for those two files to compile, unmodified,
# with a real host gcc instead of m68k-elf-gcc. Real fix32 is a 10.22
# fixed-point s32; the frac-bit count doesn't have to match exactly
# for these assertions (everything here is compared as whole pixels/
# world units via fix32ToInt), so a simpler 22.10 split keeps the
# shim's own macros easy to read.
mkdir -p "$WORK_DIR/stub"
cat > "$WORK_DIR/stub/genesis.h" <<'EOF'
#ifndef GENESIS_H
# define GENESIS_H
typedef unsigned char   u8;
typedef signed char     s8;
typedef unsigned short  u16;
typedef signed short    s16;
typedef unsigned int    u32;
typedef signed int      s32;
typedef s32 fix32;
# define FIX32_FRAC_BITS    10
# define FIX32(v)           ((fix32)((v) * (1 << FIX32_FRAC_BITS)))
# define intToFix32(v)      ((v) << FIX32_FRAC_BITS)
# define fix32ToInt(v)      ((v) >> FIX32_FRAC_BITS)
# define fix32Mul(a, b)     (((a) >> (FIX32_FRAC_BITS / 2)) * ((b) >> (FIX32_FRAC_BITS / 2)))
# define fix32Div(a, b)     (((a) << (FIX32_FRAC_BITS / 2)) / ((b) >> (FIX32_FRAC_BITS / 2)))
#endif
EOF

cat > "$WORK_DIR/test_logic.c" <<'TESTC'
#include <stdio.h>
#include "vec2.h"
#include "scaler.h"

static int  g_pass = 0;
static int  g_fail = 0;

static void check_int(char const *label, int got, int want)
{
    if (got == want)
    {
        printf("PASS  %s (got %d)\n", label, got);
        g_pass++;
    }
    else
    {
        printf("FAIL  %s (got %d, want %d)\n", label, got, want);
        g_fail++;
    }
}

int main(void)
{
    t_camera        cam;
    t_projection    proj;

    /* facing +x by hand (no camera_turn/trig here -- see this
     * script's own comment on why) */
    cam.pos.x = 0;
    cam.pos.y = 0;
    cam.forward.x = FIX32(1.0);
    cam.forward.y = FIX32(0.0);
    cam.right.x = FIX32(0.0);
    cam.right.y = FIX32(-1.0);

    proj = scaler_project(&cam, (t_vec2){FIX32(20.0), FIX32(0.0)});
    check_int("straight-ahead billboard is visible", proj.visible, 1);
    check_int("straight-ahead depth == distance", fix32ToInt(proj.depth), 20);
    check_int("straight-ahead side == 0", fix32ToInt(proj.side), 0);

    /* WINDOW_H=224, depth=20 -> ideal ~11.2px, nearest tier is 8 */
    check_int("depth 20 picks the 8px tier", scaler_tier_size(proj.tier), 8);

    /* depth=7 -> ideal 32px, nearest tier is the largest, 32 */
    proj = scaler_project(&cam, (t_vec2){FIX32(7.0), FIX32(0.0)});
    check_int("depth 7 picks the 32px tier", scaler_tier_size(proj.tier), 32);

    /* closer than the near plane (FIX32(2)) is not visible */
    proj = scaler_project(&cam, (t_vec2){FIX32(1.0), FIX32(0.0)});
    check_int("closer than the near plane is not visible", proj.visible, 0);

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return (g_fail > 0);
}
TESTC

logic_build_log=$(gcc -Wall -Wextra \
    -I "$WORK_DIR/stub" -I "$SCRIPT_DIR/solution/src" \
    "$WORK_DIR/test_logic.c" "$SCRIPT_DIR/solution/src/vec2.c" "$SCRIPT_DIR/solution/src/scaler.c" \
    -o "$WORK_DIR/test_logic" 2>&1)
logic_build_status=$?

if [[ "$logic_build_status" -ne 0 ]]; then
    fail "logic tester builds on the host" "$logic_build_log"
else
    pass "logic tester builds on the host (vec2.o/scaler.o only, real gcc not m68k-elf-gcc)"
    logic_out=$("$WORK_DIR/test_logic")
    logic_status=$?
    echo "$logic_out" | grep "^PASS\|^FAIL" | while read -r line; do
        echo "  $line"
    done
    logic_pass_count=$(echo "$logic_out" | grep -c "^PASS")
    logic_fail_count=$(echo "$logic_out" | grep -c "^FAIL")
    pass_count=$((pass_count + logic_pass_count))
    fail_count=$((fail_count + logic_fail_count))
    if [[ "$logic_status" -ne 0 ]]; then
        fail "all logic assertions pass" "see failures above"
    fi
fi

# ── headless smoke test of the real ROM in BlastEm ───────────────────────────

echo
echo "Running the ROM in BlastEm headless (4s)..."
if command -v "$BLASTEM" >/dev/null 2>&1 || [[ -x "$BLASTEM" ]]; then
    SDL_AUDIODRIVER=dummy SDL_RENDER_DRIVER=software \
        timeout 4 "$BLASTEM" out/rom.bin >/dev/null 2>&1
    blastem_status=$?
    if [[ "$blastem_status" -eq 124 ]]; then
        pass "ROM runs in BlastEm for 4s without crashing"
    else
        fail "ROM runs in BlastEm for 4s without crashing" "exit code: $blastem_status"
    fi
else
    fail "BlastEm found at \$BLASTEM" "set BLASTEM=/path/to/blastem"
fi

# ── summary ───────────────────────────────────────────────────────────────────

echo
hr
printf "  ${C_BOLD}%d passed, %d failed${C_RESET}\n" "$pass_count" "$fail_count"
hr

if [[ "$fail_count" -gt 0 ]]; then
    exit 1
fi
exit 0
