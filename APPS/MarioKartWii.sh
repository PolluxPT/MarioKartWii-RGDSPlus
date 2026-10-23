#!/bin/sh
# RGDS+ Stock Linux experimental launcher. Uses only files next to this script.
APP_PARENT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd) || exit 1
APP_DIR="$APP_PARENT/MarioKartWii"
cd "$APP_DIR" || exit 1
exec >>"$APP_DIR/Arranque.txt" 2>&1
# Truncate at each launch; keep all diagnostics inside the application folder.
: >"$APP_DIR/Arranque.txt"
echo 'Mario Kart Wii for RGDS+ - Final 1.0'
date
uname -a
echo "XDG_RUNTIME_DIR=${XDG_RUNTIME_DIR:-} WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-}"
umask 077
if [ "$(uname -m)" != aarch64 ]; then echo 'ERROR: requires aarch64 Linux'; exit 1; fi

MKW_OUTPUT=0
MKW_FB_WIDTH=1024
MKW_FB_HEIGHT=768
MKW_SCALE=1
MKW_LAYOUT=fit
MKW_AUDIO_GAIN=0.35
MKW_BOTTOM=0
MKW_MINIMAP=1
MKW_TEST_SECONDS=0
MKW_LOWER_OFF=1
MKW_REAL_EGL=
SDL_GAMECONTROLLERCONFIG=
if [ -f "$APP_DIR/config.txt" ]; then . "$APP_DIR/config.txt"; fi
export MKW_OUTPUT MKW_FB_WIDTH MKW_FB_HEIGHT MKW_SCALE MKW_LAYOUT MKW_BOTTOM MKW_MINIMAP MKW_REAL_EGL SDL_GAMECONTROLLERCONFIG
export MKW_AUDIO_GAIN
export MKW_VBO_CACHE=1 MKW_MENU_REPEAT=1 MKW_SELECT_ALL=1
export MKW_DATA_DIR="$APP_DIR" MKW_LOG_FILE="$APP_DIR/Jogo.txt"
export MKW_EGL_LIBRARY="$APP_DIR/libmkw_wayland.so"
unset MKW_HEADLESS

if [ -z "${XDG_RUNTIME_DIR:-}" ] && [ -S /var/run/wayland-0 ]; then export XDG_RUNTIME_DIR=/var/run; fi
if [ -z "${WAYLAND_DISPLAY:-}" ] && [ -n "${XDG_RUNTIME_DIR:-}" ] && [ -S "$XDG_RUNTIME_DIR/wayland-0" ]; then export WAYLAND_DISPLAY=wayland-0; fi
if [ -z "${WAYLAND_DISPLAY:-}" ]; then echo 'ERROR: Wayland session not found. Send Arranque.txt.'; exit 1; fi

# Launch bundled glibc explicitly. No system files are replaced, no fixed /tmp loader.
RUNNER="$APP_DIR/libs/ld-linux-aarch64.so.1"
LIBS="$APP_DIR/libs:/usr/lib/aarch64-linux-gnu:/lib/aarch64-linux-gnu:/usr/lib:/lib"
chmod +x "$RUNNER" "$APP_DIR/mkw3ds" "$APP_DIR/graphics_probe" "$APP_DIR/input_info"
run_binary() { "$RUNNER" --library-path "$LIBS" "$@"; }
run_binary "$APP_DIR/input_info"

game_pid=
timer_pid=
guardian_pid=
lower_saved=
VOLUME_NODE=/sys/class/anbernic_misc/openbor_volume
volume_saved=
volume_active=0

# RGDS+ Stock hardware volume is a level from 0 through 10.
# Capture it once; do not replace the snapshot while the game is running.
save_volume() {
    if [ ! -r "$VOLUME_NODE" ] || [ ! -w "$VOLUME_NODE" ]; then
        echo 'RGDS volume: hardware node unavailable; cannot preserve volume.'
        return 1
    fi
    volume_saved=$(cat "$VOLUME_NODE") || return 1
    case "$volume_saved" in
        [0-9]|10) ;;
        *) echo "RGDS volume: invalid initial level: $volume_saved"; return 1 ;;
    esac
    volume_active=1
    echo "RGDS volume: saved initial level=$volume_saved/10"
}

restore_volume() {
    [ "$volume_active" = 1 ] || return 0
    if ! printf '%s\n' "$volume_saved" > "$VOLUME_NODE"; then
        echo 'RGDS volume: restoration write failed.'
        return 1
    fi
    volume_current=$(cat "$VOLUME_NODE") || return 1
    if [ "$volume_current" != "$volume_saved" ]; then
        echo "RGDS volume: verification failed (expected=$volume_saved, read=$volume_current)."
        return 1
    fi
    volume_active=0
    echo "RGDS volume: restored initial level=$volume_saved/10"
}

# Keep the original game usable if this firmware does not expose the node.
save_volume || echo 'RGDS volume: continuing without hardware-volume snapshot.'

# Independent recovery process: restore if the launcher disappears without cleanup.
# The child gets a copy of the original snapshot, not subsequent game volume changes.
if [ "$volume_active" = 1 ]; then
    launcher_pid=$$
    (
        trap '' HUP
        while kill -0 "$launcher_pid" 2>/dev/null; do sleep 1; done
        restore_volume
    ) &
    guardian_pid=$!
fi
LOWER_LIGHT=/sys/class/backlight/backlight/brightness
restore() {
    [ -z "$timer_pid" ] || kill "$timer_pid" 2>/dev/null
    if [ -n "$game_pid" ]; then
        kill -TERM "$game_pid" 2>/dev/null
        sleep 1
        kill -KILL "$game_pid" 2>/dev/null
    fi
    if [ -n "$lower_saved" ]; then
        printf '%s\n' "$lower_saved" > "$LOWER_LIGHT"
        echo "Lower screen restored: $lower_saved"
    fi
    restore_volume
    if [ -n "$guardian_pid" ]; then
        kill -TERM "$guardian_pid" 2>/dev/null || :
        wait "$guardian_pid" 2>/dev/null || :
    fi
}
trap restore EXIT
trap 'exit 130' INT
trap 'exit 143' TERM HUP

if [ -f "$APP_DIR/3ds/mkw-3ds/content.txt" ] && [ -f "$APP_DIR/3ds/mkw-3ds/Race/Common.szs" ]; then
    echo 'Converted content detected. Starting game (experimental).'
    PROGRAM="$APP_DIR/mkw3ds"
    case "$MKW_TEST_SECONDS" in ''|*[!0-9]*) MKW_TEST_SECONDS=0;; esac
    LIMIT="$MKW_TEST_SECONDS"
    # Verified Stock profile from the working ScummVM package:
    # upper=backlight1, lower=backlight. Never use aggregate lcdoff.
    if [ "$MKW_LOWER_OFF" = 1 ] && [ "$MKW_OUTPUT" = 0 ] && [ -r "$LOWER_LIGHT" ] && [ -w "$LOWER_LIGHT" ]; then
        lower_saved=$(cat "$LOWER_LIGHT")
        case "$lower_saved" in ''|*[!0-9]*) lower_saved=;; esac
        if [ -n "$lower_saved" ]; then
            printf '0\n' > "$LOWER_LIGHT"
            echo "Lower backlight off; previous=$lower_saved (upper untouched)."
        fi
    fi
else
    echo 'Converted content not present. Running graphics test only (red / green / blue).'
    echo 'Raw ISO, WBFS and RVZ files are not read by this executable.'
    echo 'Use PC/Preparar_Jogo.bat and copy its converted 3ds folder here.'
    PROGRAM="$APP_DIR/graphics_probe"
    LIMIT=20
fi

(exec "$RUNNER" --library-path "$LIBS" "$PROGRAM") &
game_pid=$!
# exec makes the tracked pid the real game process for cleanup.
wait_pid="$game_pid"
if [ "$LIMIT" -gt 0 ]; then
    echo "Automatic return after $LIMIT seconds (test build)."
    (
        sleeper=
        trap '[ -z "$sleeper" ] || kill "$sleeper" 2>/dev/null; exit 0' TERM INT HUP
        sleep "$LIMIT" &
        sleeper=$!
        wait "$sleeper"
        echo '[watchdog] Test time limit reached; terminating application.'
        kill -TERM "$wait_pid" 2>/dev/null
        sleep 3 &
        sleeper=$!
        wait "$sleeper"
        kill -KILL "$wait_pid" 2>/dev/null
    ) &
    timer_pid=$!
fi
wait "$game_pid"
result=$?
game_pid=
echo "Exit code: $result"
[ -z "$timer_pid" ] || kill "$timer_pid" 2>/dev/null
timer_pid=
exit "$result"
