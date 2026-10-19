#!/usr/bin/env python3
"""Build a ROM-free installer ZIP; optionally include the user's launcher icon."""
from pathlib import Path
import os
import zipfile
import hashlib
import re

ROOT = Path(__file__).resolve().parents[1]
APP = ROOT / 'APPS/MarioKartWii'
PLACEHOLDERS = {
    'LEIA.txt',
    'mkw-3ds/COLOCAR_DADOS_CONVERTIDOS_AQUI.txt',
}
FORBIDDEN = {'.iso', '.rvz', '.wbfs', '.wbf1', '.wad', '.szs', '.brres', '.brstm', '.gmdl', '.t3x'}
PC_FILES = ['PC/preparar.py', 'PC/Preparar_Jogo.bat', 'PC/builder.zip',
            'PC/builder.sha256', 'PC/Dolphin/LEIA_README.txt']
ROOT_FILES = ['VERSION', 'README.md', 'README_PT.md', 'README_EN.md',
              'INICIO_PT.md', 'START_HERE_EN.md', 'LEIA_PRIMEIRO.txt', 'LICENSE',
              'THIRD_PARTY.md', 'CHANGELOG.md', 'RELEASE_NOTES.md',
              'PREPARAR_ROM.bat', 'VERIFICAR_FERRAMENTAS.bat', 'Criar_ZIP.bat',
              '.gitignore', 'ROM/LEIA_README.txt']


def validate():
    for p in (ROOT / 'ROM').rglob('*'):
        if p.is_file() and p.relative_to(ROOT).as_posix() != 'ROM/LEIA_README.txt':
            raise ValueError('A pasta ROM contem ficheiros pessoais. Cria a release a partir de uma copia limpa do projeto.')
    for p in (ROOT / 'APPS').rglob('*'):
        if p.is_symlink():
            raise ValueError(f'Ligacao simbolica inesperada: {p.relative_to(ROOT)}')
        if p.is_file() and p.suffix.lower() in FORBIDDEN:
            raise ValueError(f'Dados de jogo/imagem de disco encontrados: {p.relative_to(ROOT)}. Usa uma copia limpa do projeto.')
    for p in (APP / '3ds').rglob('*'):
        if p.is_file() and p.relative_to(APP / '3ds').as_posix() not in PLACEHOLDERS:
            raise ValueError(f'Conteudo do jogo encontrado: {p.relative_to(ROOT)}. Nao sera incluido.')
    for name in ['mkw3ds', 'libmkw_wayland.so', 'config.txt', 'controls.txt', 'libs/ld-linux-aarch64.so.1']:
        if not (APP / name).is_file():
            raise ValueError(f'Falta um ficheiro da instalacao: {name}')
    icon = ROOT / 'APPS/Imgs/MarioKartWii.png'
    if icon.exists() and icon.read_bytes()[:8] != b'\x89PNG\r\n\x1a\n':
        raise ValueError('MarioKartWii.png nao e um PNG valido.')
    for name in PC_FILES + ROOT_FILES:
        if not (ROOT / name).is_file():
            raise ValueError(f'Falta um componente da release completa: {name}')
    expected = (ROOT / 'PC/builder.sha256').read_text().split()[0]
    if hashlib.sha256((ROOT / 'PC/builder.zip').read_bytes()).hexdigest() != expected:
        raise ValueError('O arquivo do builder nao corresponde ao checksum fornecido.')
    return icon


def main():
    icon = validate()
    files = []
    for folder in ['APPS', 'Sources', 'docs', 'tools']:
        files.extend(p for p in (ROOT / folder).rglob('*') if p.is_file())
    files.extend(ROOT / n for n in ROOT_FILES + PC_FILES)
    files = sorted(p for p in files if p.name not in {'Arranque.txt', 'Jogo.txt'}
                   and '__pycache__' not in p.parts and p.name != 'test_runtime')
    version = (ROOT / 'VERSION').read_text().strip()
    if not re.fullmatch(r'\d+\.\d+\.\d+', version):
        raise ValueError('VERSION invalido.')
    output = ROOT / f'dist/MarioKartWii_RGDSPlus_v{version}.zip'
    output.parent.mkdir(exist_ok=True)
    temporary = output.with_suffix('.tmp')
    try:
        with zipfile.ZipFile(temporary, 'w', zipfile.ZIP_DEFLATED, compresslevel=6) as z:
            for p in files:
                info = zipfile.ZipInfo.from_file(p, p.relative_to(ROOT).as_posix())
                if p.name in {'MarioKartWii.sh', 'mkw3ds', 'graphics_probe', 'input_info', 'ld-linux-aarch64.so.1'}:
                    info.external_attr = (0o100755 << 16)
                    info.create_system = 3
                info.compress_type = zipfile.ZIP_DEFLATED
                z.writestr(info, p.read_bytes())
        with zipfile.ZipFile(temporary) as z:
            if z.testzip() is not None:
                raise ValueError('Erro na verificacao do ZIP.')
        os.replace(temporary, output)
    finally:
        temporary.unlink(missing_ok=True)
    print(f'ZIP criado: {output}')
    print('Icone incluido.' if icon.exists() else 'Icone opcional ausente. Adiciona APPS/Imgs/MarioKartWii.png e repete para o incluir.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, zipfile.BadZipFile) as error:
        print(f'ERRO: {error}')
        raise SystemExit(1)
