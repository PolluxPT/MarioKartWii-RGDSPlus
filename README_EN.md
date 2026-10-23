[![Download](https://img.shields.io/badge/Download-v1.1.1-blue?style=for-the-badge)](https://github.com/PolluxPT/MarioKartWii-RGDSPlus/releases/latest)

# Mario Kart Wii for RGDS+

A Stock Linux adaptation for ANBERNIC RGDS+, based on MiniBandit's **MKW-3DS** remake and Matrioshka Brain's H700 port, with RGDS+ hardware testing by Filipe Baeta. This is a game-specific native remake adaptation, not a general Wii emulator.

## Download and prepare

Use the **complete v1.1.1 release** under Releases. Follow [START_HERE_EN.md](START_HERE_EN.md) for the full first-time setup and conversion instructions.

1. Install Python 3.13, Git, Node.js LTS and devkitPro/tex3ds as documented. RVZ inputs also need DolphinTool from the official Dolphin distribution.
2. Place your own USA **RMCE01** ISO, WBFS or RVZ in `ROM`.
3. Open `PREPARAR_ROM.bat` and choose **English**.
4. When preparation reports **DONE**, copy the contents of `PARA_CONSOLA/APPS` into `APPS` on the TF2 card.
5. Launch **APPS → MarioKartWii** on RGDS+.

The original H700 builder and the PT/EN front-end are included; you do not need the original download shared during development. Initial system tools require separate installation using the official links in the guide. Dependencies are then prepared by the script. No programming is required.

No ROM or converted Nintendo game data is included. PAL/European images, NKit and split WBFS are not supported by this workflow. The original builder audits the expected USA revision. Preparation may take an hour or more and requires at least 20 GiB free. Game data stays on your PC.

If you already have working converted data, update the application files while keeping `APPS/MarioKartWii/3ds/mkw-3ds`; no new conversion is required.

## Features

- Upper-screen output at 1024×768 with the original aspect ratio preserved.
- RGDS+ controller mapping, rear view and directional tricks.
- Physical volume controls with a smooth low-volume curve.
- Lower backlight disabled during the game and restored on exit.
- No automatic three-minute play limit.
- Reused startup and game logs inside the application directory.

The final game application was tested on RGDS+ Stock Linux. Other models are unverified. Wheelies await a test with an unlocked bike. The newly packaged preparation front-end is checked with automated tests and simulated inputs; a fresh end-to-end conversion could not be repeated without a disc image.

## Controls

| Control | Action |
|---|---|
| A / B | Accelerate and confirm / brake and back |
| Left stick or D-pad | Steering |
| L1 / R1 | Item / hop and drift |
| Hold L2 | Rear view |
| R2 or X + stick direction | Trick on a suitable ramp |
| Y, neutral or up | Bike wheelie |
| Y + stick down | Request end of wheelie |
| Start / Special | Pause / HUD indicators |
| Start + Select | Exit |
| Volume + / − | Adjust sound |

Actual trick eligibility and animation are implemented by the remake. See [configuration options](docs/CONFIGURATION_EN.md).

## Icon

Add your PNG at `APPS/Imgs/MarioKartWii.png` before preparation. It is copied into the assembled application folder. The game works without the icon. To include it in your public installer, add it before running `Criar_ZIP.bat`.

## Source, packaging and publication

RGDS+ adaptation sources are in `Sources`. The original builder is shipped in `PC/builder.zip` with a checksum and extracts automatically on first preparation. It is kept separate from local working data to make the public package easy to upload.

Run `python tools/package_release.py`, or open `Criar_ZIP.bat`, to produce the complete `dist/MarioKartWii_RGDSPlus_v1.1.0.zip`. The installer includes the application, original builder archive, front-end, documentation and optional icon. It excludes your ROM, local dependencies, conversion cache and converted game files. See [publishing instructions](docs/PUBLISHING_EN.md).

The bundled H700 source snapshot is labelled **v1.21**, while the unchanged executable is labelled **v1.22**. That snapshot is not claimed to reproduce the executable exactly. Build instructions for the RGDS+ adapter are in `Sources/BUILD.txt`.

## Credits and licensing

- MiniBandit: [MKW-3DS](https://github.com/MiniBandit/MKW-3DS), retaining credits to the initial work by dudewhat.
- Matrioshka Brain: H700 port and compatibility layer, as identified in the original package.
- Filipe Baeta: RGDS+ adaptation project and hardware testing.
- Third-party notices are preserved in `APPS/MarioKartWii/licenses` and described in [THIRD_PARTY.md](THIRD_PARTY.md).

The root MIT license covers the RGDS+ additions, not third-party components or a separately supplied icon. This project is not affiliated with Nintendo or ANBERNIC. For issues, provide the firmware version and the appropriate logs: `PC/Preparacao.txt` for conversion, or `Arranque.txt` and `Jogo.txt` for console problems. Do not attach disc images or converted game data.
