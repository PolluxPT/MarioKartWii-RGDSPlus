# Getting started — prepare Mario Kart Wii on Windows

## What you get

Download and fully extract the complete release, place your disc image in **`ROM`**, and open **`PREPARAR_ROM.bat`**. The preparation tool converts the assets and assembles **`PARA_CONSOLA/APPS`**, ready to copy to the TF2 card. No programming is required.

First-time setup requires the free tools below. Their executables are not bundled; the links point to official distributions. The original asset builder and the PT/EN preparation front-end **are bundled**.

## 1. Set up your PC once

Use 64-bit Windows 10/11, an Internet connection and at least **20 GiB of free disk space**. Processing may take an hour or longer. Extract the entire ZIP to a simple location such as `C:\MKW_RGDS`; do not run from inside the ZIP or on the console's card.

Install:

| Tool | Installation |
|---|---|
| **64-bit Python 3.13.x** | [Python for Windows](https://www.python.org/downloads/windows/). Choose the 3.13 series and enable **Add python.exe to PATH**. The original builder supports 3.11–3.14; do not use 3.15. |
| **Git for Windows** | [Git](https://git-scm.com/downloads/win), using the standard installer options. |
| **Node.js LTS** | [Node.js](https://nodejs.org/), including npm. |
| **devkitPro** | [Official installer](https://github.com/devkitPro/installer/releases/latest). Run `devkitProUpdater-…exe`, select **3DS Development** and keep `C:\devkitPro`. The builder needs its `tex3ds` texture tool; you do not need to write or compile a 3DS application. |

Close any old preparation windows after installing. Open **`VERIFICAR_FERRAMENTAS.bat`** and choose **2 — English**. Resolve entries marked `MISSING` before continuing.

### If your disc image is RVZ

Download [Dolphin for Windows](https://dolphin-emu.org/download/) and extract the **complete distribution** into `PC/Dolphin`. A subfolder is fine. `DolphinTool.exe` must remain with its DLLs; do not copy just that executable.

The preparation tool calls DolphinTool to convert RVZ to ISO, then runs the asset builder. You do not need to launch the emulator or type commands. DolphinTool is unnecessary for ISO and WBFS inputs.

## 2. Add your disc image

Put your own **Mario Kart Wii USA, disc ID RMCE01**, in `ROM`. Supported containers: **ISO, WBFS and RVZ**. Spaces in the filename are fine and no specific filename is required; the contents are checked.

Do not use the PAL/European version (`RMCP01`), modified images, NKit, or split WBFS files. Region alone does not guarantee compatibility: the original builder also audits internal files against its expected disc revision and rejects mismatches.

## 3. Convert and prepare

1. Open **`PREPARAR_ROM.bat`**.
2. Choose **2 — English**. If there are several images, select the one you want by number.
3. Wait through the four stages: disc format, dependencies, asset conversion, and application assembly.
4. Preparation is only complete when you see **DONE** and the `PARA_CONSOLA/APPS` path.

Python packages and the builder's pinned public source dependencies are downloaded automatically. Your game data is processed locally; this preparation tool does not upload it. Technical messages from the original builder remain in English.

Your RVZ/ISO/WBFS is not modified. The ISO generated from an RVZ is cached in `PC/work`. Other generated files and caches live in `PC/builder`; keep them if you need to resume. Running again may reuse completed builder stages.

## 4. Copy to RGDS+

1. Close the game and insert the TF2 card into your PC.
2. Copy the **contents** of `PARA_CONSOLA/APPS` into the card's `APPS` directory, merging folders and replacing application files as needed.
3. Verify these paths on the card:
   - `APPS/MarioKartWii.sh`
   - `APPS/MarioKartWii/3ds/mkw-3ds/content.txt`
   - `APPS/MarioKartWii/3ds/mkw-3ds/Race/Common.szs`
4. Reinsert the card and launch **APPS → MarioKartWii** on RGDS+.

Do not copy the original disc image to the console. Avoid doubled paths such as `APPS/APPS`, `3ds/3ds`, or `mkw-3ds/mkw-3ds`.

## Icon

If you have an icon, place it at `APPS/Imgs/MarioKartWii.png` **before preparing**. It will be copied to the ready-to-install folder. You can also add it afterwards at `PARA_CONSOLA/APPS/Imgs/MarioKartWii.png`. The application works without it.

## Troubleshooting

- Missing tool: run `VERIFICAR_FERRAMENTAS.bat` and follow the setup table above.
- Preparation error or interruption: keep **`PC/Preparacao.txt`** and run again. Do not delete all caches as the first troubleshooting step.
- Console-only failure: keep `APPS/MarioKartWii/Arranque.txt` and `Jogo.txt`.
- Only colour screens appear: the converted data was not found at the expected paths.
- No sound: use the physical volume buttons; level zero is mute.

Do not publish your disc image, `PC/work`, `PC/builder`, or `PARA_CONSOLA` on GitHub. Distributable releases are built from the clean project files and exclude user data.

The application and gameplay were tested on RGDS+ Stock Linux. The new PT/EN front-end is checked with automated tests and simulations; a new complete conversion could not be repeated without the disc image. The included engine and builder come from the original project base used here.
