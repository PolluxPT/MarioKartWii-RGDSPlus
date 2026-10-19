# Configuration

Edit `APPS/MarioKartWii/config.txt` before preparation, or the same file in the assembled application after preparation.

| Setting | Default | Meaning |
|---|---|---|
| `MKW_OUTPUT` | `0` | Upper screen on the tested Stock profile |
| `MKW_FB_WIDTH` / `MKW_FB_HEIGHT` | `1024` / `768` | Output window size |
| `MKW_LAYOUT` | `fit` | Preserve aspect ratio; `full` stretches |
| `MKW_SCALE` | `1` | Original internal resolution |
| `MKW_AUDIO_GAIN` | `0.35` | Audio gain ceiling; use `0.20` for lower volume |
| `MKW_LOWER_OFF` | `1` | Disable lower backlight; `0` keeps it on |
| `MKW_TEST_SECONDS` | `0` | No automatic test time limit |

Volume follows the Stock 0–10 level. Gain is `MKW_AUDIO_GAIN × (level/10)²`; zero is mute.

The tested hardware profile uses `backlight1` for the upper panel and `backlight` for the lower panel. The launcher restores the previous lower brightness when it exits normally, after a game failure, or after a signal handled by its cleanup trap.

Edit `controls.txt` for actions; `controls.default.txt` contains the final reference layout. L3/R3 remain alternative camera controls only if the device exposes those clicks. Primary camera/trick actions use L2 and R2.
