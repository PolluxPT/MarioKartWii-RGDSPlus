# Mario Kart Wii for RGDS+

Adaptação para RGDS+ Stock Linux / RGDS+ Stock Linux adaptation.

**Português:** [Guia completo](README_PT.md) · [Começar e converter a ROM](INICIO_PT.md)

**English:** [Full README](README_EN.md) · [Prepare your disc image](START_HERE_EN.md)

## PT — utilização da release

1. Descarrega e extrai a **release completa v1.1.0**.
2. Instala as ferramentas indicadas no guia, uma única vez.
3. Coloca a tua ROM USA RMCE01 em **ROM**.
4. Abre **PREPARAR_ROM.bat** e escolhe Português.
5. Copia **PARA_CONSOLA/APPS** para o cartão TF2 e abre o jogo.

## EN — release workflow

1. Download and extract the **complete v1.1.0 release**.
2. Install the tools listed in the guide once.
3. Put your USA RMCE01 disc image in **ROM**.
4. Open **PREPARAR_ROM.bat** and select English.
5. Copy **PARA_CONSOLA/APPS** to the TF2 card and launch the game.

The original H700 asset builder and the PT/EN front-end are included. System tools require the documented first-time setup. ISO/WBFS/RVZ supported; RVZ uses DolphinTool. No ROM or converted game assets are distributed.

O preparador original H700 e a interface PT/EN estão incluídos. As ferramentas de sistema exigem a preparação inicial documentada. Suporta ISO/WBFS/RVZ; RVZ usa DolphinTool. Não distribui ROM nem dados convertidos do jogo.

## Ícone / Icon

`APPS/Imgs/MarioKartWii.png` — optional / opcional. The preparation tool copies it into the assembled APPS. O preparador copia-o para APPS pronta para a consola.

## Publicação / Publishing

[Português](docs/PUBLICAR_NO_GITHUB.md) · [English](docs/PUBLISHING_EN.md)

Use **Criar_ZIP.bat** to build `dist/MarioKartWii_RGDSPlus_v1.1.0.zip`, the complete release to attach on GitHub.

## Créditos / Credits

Based on [MiniBandit/MKW-3DS](https://github.com/MiniBandit/MKW-3DS) and Matrioshka Brain's H700 port, retaining credits to dudewhat and third-party authors. RGDS+ adaptation and hardware testing with Filipe Baeta. See [THIRD_PARTY.md](THIRD_PARTY.md) and the original notices bundled with the application. The root MIT license only covers the RGDS+ additions.

This is a game-specific remake adaptation, not a Wii emulator. Not affiliated with Nintendo or ANBERNIC. Tested on RGDS+ Stock Linux; bike wheelies await a test with an unlocked bike. The new front-end is locally tested with simulated inputs; a new end-to-end disc conversion has not been repeated without the disc image.
