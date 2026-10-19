# Publish the complete package on GitHub

Suggested repository: **MarioKartWii-RGDSPlus**. Use a clean, extracted copy of the project.

1. Optionally place your icon at `APPS/Imgs/MarioKartWii.png`.
2. Run `Criar_ZIP.bat` or `python tools/package_release.py`.
3. Create or update the GitHub repository by uploading the contents of this project folder, excluding `dist` and all local working data.
4. Create a release tagged **v1.1.0** and titled **Mario Kart Wii for RGDS+ — Complete package v1.1.0**.
5. Copy `RELEASE_NOTES.md` into its description.
6. Attach **`dist/MarioKartWii_RGDSPlus_v1.1.0.zip`** and publish.

That ZIP is the user-facing complete release: it includes the app, `ROM` instructions, the PT/EN preparation tool, the original `PC/builder.zip`, and both getting-started guides.

The checked-in builder archive contains the original source scripts, not game data. Never upload your ROM, `PARA_CONSOLA`, `PC/work`, the extracted `PC/builder`, or converted game assets. The `.gitignore` helps with Git clients; browser uploads still require selecting only the intended clean project files.

If you already published v1.0.0, this is a new release. Its game binaries remain the same; v1.1.0 changes packaging, conversion workflow and documentation. Users who already have working converted data do not need to convert again.
