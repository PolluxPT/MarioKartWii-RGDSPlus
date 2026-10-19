#!/usr/bin/env python3
"""PT/EN front-end for the bundled, unmodified H700 asset builder. MIT."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import zipfile

PC = Path(__file__).resolve().parent
ROOT = PC.parent
BUILDER = PC / 'builder'
LANG = 'pt'
EXPECTED_CONTENT = {'cf69fd08237c86f0faa555dbecece969', '0a4c701b08480f9ca6a85abcf4c55f31'}


def say(pt, en):
    return en if LANG == 'en' else pt


def run(args, cwd=None):
    print('\n>', ' '.join(map(str, args)), flush=True)
    with subprocess.Popen(list(map(str, args)), cwd=cwd or BUILDER,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                          text=True, encoding='utf-8', errors='replace') as proc:
        for line in proc.stdout:
            print(line, end='', flush=True)
        code = proc.wait()
    if code:
        raise RuntimeError(say('Uma ferramenta terminou com erro. Consulta o registo acima.',
                               'A tool failed. See the log above.') + f' (exit {code})')


def unpack_builder():
    archive = PC / 'builder.zip'
    expected = (PC / 'builder.sha256').read_text().split()[0]
    actual = hashlib.sha256(archive.read_bytes()).hexdigest()
    if actual != expected:
        raise RuntimeError(say('builder.zip esta danificado. Extrai novamente a release.',
                               'builder.zip is damaged. Extract a fresh copy of the release.'))
    with zipfile.ZipFile(archive) as z:
        for item in z.infolist():
            destination = (BUILDER / item.filename).resolve()
            if not destination.is_relative_to(BUILDER.resolve()):
                raise RuntimeError('Unsafe builder archive path')
        # Restore only missing original files. Existing generated caches are kept.
        for item in z.infolist():
            target = BUILDER / item.filename
            if not item.is_dir() and not target.exists():
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(z.read(item))


def devkit_root():
    value = os.environ.get('DEVKITPRO', '')
    if os.name == 'nt':
        if len(value) > 3 and value[0] == '/' and value[2] == '/' and value[1].isalpha():
            return Path(value[1] + ':/' + value[3:])
        if value and not value.startswith('/'):
            return Path(value)
        return Path(os.environ.get('DEVKITPRO_WIN', 'C:/devkitPro'))
    return Path(value or '/opt/devkitpro')


def requirements():
    tex3ds = devkit_root() / 'tools/bin' / ('tex3ds.exe' if os.name == 'nt' else 'tex3ds')
    entries = [
        ('Python 3.11-3.14', (3, 11) <= sys.version_info[:2] < (3, 15), 'https://www.python.org/downloads/'),
        ('Git', bool(shutil.which('git')), 'https://git-scm.com/downloads'),
        ('Node.js', bool(shutil.which('node')), 'https://nodejs.org/'),
        ('devkitPro / tex3ds', tex3ds.is_file(), 'https://github.com/devkitPro/installer/releases/latest'),
    ]
    for name, found, url in entries:
        print(f'[{"OK" if found else "MISSING / EM FALTA"}] {name}' + ('' if found else f' -> {url}'))
    if not all(found for _, found, _ in entries):
        raise RuntimeError(say('Instala as ferramentas em falta conforme INICIO_PT.md. Fecha esta janela e repete.',
                               'Install the missing tools as described in START_HERE_EN.md. Close this window and retry.'))


def choose_disc(argument=None):
    if argument:
        disc = Path(argument).expanduser().resolve()
    else:
        candidates = sorted(p for p in (ROOT / 'ROM').iterdir()
                            if p.is_file() and p.suffix.lower() in {'.iso', '.wbfs', '.rvz'})
        if not candidates:
            raise RuntimeError(say('Coloca o teu ISO, WBFS ou RVZ na pasta ROM e volta a executar.',
                                   'Put your ISO, WBFS or RVZ in the ROM folder and run again.'))
        if len(candidates) == 1:
            disc = candidates[0]
        else:
            for i, p in enumerate(candidates, 1):
                print(f'{i}: {p.name}')
            value = input(say('Numero da ROM: ', 'ROM number: ')).strip()
            if not value.isdigit() or not 1 <= int(value) <= len(candidates):
                raise RuntimeError(say('Selecao invalida.', 'Invalid selection.'))
            disc = candidates[int(value) - 1]
    if not disc.is_file() or disc.suffix.lower() not in {'.iso', '.wbfs', '.rvz'}:
        raise RuntimeError(say('Ficheiro nao encontrado ou formato nao suportado.', 'File not found or unsupported format.'))
    print(say('Disco selecionado: ', 'Selected disc: ') + str(disc))
    return disc


def check_region(disc):
    with disc.open('rb') as f:
        header = f.read(512)
        if disc.suffix.lower() == '.wbfs':
            if len(header) < 13 or header[:4] != b'WBFS' or not 9 <= header[8] <= 20:
                raise RuntimeError(say('Cabecalho WBFS invalido.', 'Invalid WBFS header.'))
            slot = next((n for n in header[12:] if n), None)
            if slot is None:
                raise RuntimeError(say('WBFS sem disco.', 'WBFS contains no disc.'))
            f.seek(slot * (1 << header[8]))
            code = f.read(6)
        else:
            code = header[:6]
    if code != b'RMCE01':
        raise RuntimeError(say('E necessario Mario Kart Wii USA (RMCE01). Identificador encontrado: ',
                               'Mario Kart Wii USA (RMCE01) is required. Found disc ID: ') + repr(code))
    print(say('Regiao confirmada: RMCE01. O builder ainda verificara a revisao do disco.',
              'Region confirmed: RMCE01. The builder will also verify the disc revision.'))


def find_dolphin():
    options = [shutil.which('DolphinTool.exe'), shutil.which('dolphin-tool')]
    options += [str(p) for p in (PC / 'Dolphin').rglob('DolphinTool.exe')]
    options += [str(Path(os.environ.get('ProgramFiles', 'C:/Program Files')) / 'Dolphin/DolphinTool.exe')]
    tool = next((Path(p) for p in options if p and Path(p).is_file()), None)
    if tool:
        return tool
    print(say('Para RVZ, extrai o Dolphin completo para PC/Dolphin, ou indica DolphinTool.exe.',
              'For RVZ, extract the complete Dolphin distribution into PC/Dolphin, or provide DolphinTool.exe.'))
    print('https://dolphin-emu.org/download/')
    value = input(say('Caminho de DolphinTool.exe (Enter para cancelar): ',
                      'DolphinTool.exe path (Enter to cancel): ')).strip().strip('"').strip("'")
    if not value or not Path(value).is_file():
        raise RuntimeError(say('DolphinTool nao encontrado. Consulta INICIO_PT.md.',
                               'DolphinTool not found. See START_HERE_EN.md.'))
    return Path(value).resolve()


def convert_rvz(disc):
    tool = find_dolphin()
    identity = {'source': str(disc.resolve()), 'size': disc.stat().st_size, 'mtime_ns': disc.stat().st_mtime_ns}
    key = hashlib.sha256(json.dumps(identity, sort_keys=True).encode()).hexdigest()[:16]
    folder = PC / 'work'
    folder.mkdir(exist_ok=True)
    output = folder / f'{key}.iso'
    stamp = folder / f'{key}.json'
    if output.is_file() and stamp.is_file() and json.loads(stamp.read_text()) == identity:
        check_region(output)
        print(say('A reutilizar a conversao ISO anterior.', 'Reusing the previous ISO conversion.'))
        return output
    temporary = folder / f'{key}.partial.iso'
    temporary.unlink(missing_ok=True)
    print(say('Etapa 1/4: converter RVZ para ISO. O original nao e alterado.',
              'Stage 1/4: convert RVZ to ISO. The original is not modified.'))
    try:
        run([tool, 'convert', '-i', disc, '-o', temporary, '-f', 'iso'], PC)
        check_region(temporary)
        temporary.replace(output)
        stamp.write_text(json.dumps(identity), encoding='utf-8')
    except BaseException:
        temporary.unlink(missing_ok=True)
        raise
    return output


def prepare_environment():
    unpack_builder()
    venv = BUILDER / '.venv'
    python = venv / ('Scripts/python.exe' if os.name == 'nt' else 'bin/python')
    if not python.is_file():
        run([sys.executable, '-m', 'venv', venv])
    run([python, '-c', 'import sys; sys.exit(not ((3,11) <= sys.version_info[:2] < (3,15)))'])
    print(say('Etapa 2/4: preparar dependencias publicas (precisa de Internet).',
              'Stage 2/4: prepare public dependencies (Internet required).'))
    run([python, 'tools/bootstrap.py'])
    marker = venv / 'mkw-dependencies-installed'
    if not marker.exists():
        run([python, '-m', 'pip', 'install', '-r', 'requirements.txt'])
        marker.touch()
    return python


def assemble(content):
    if not (content / 'mkw-3ds/content.txt').is_file() or not (content / 'mkw-3ds/Race/Common.szs').is_file():
        raise RuntimeError(say('Faltam ficheiros convertidos. Consulta PC/Preparacao.txt.',
                               'Converted files are missing. See PC/Preparacao.txt.'))
    if (content / 'mkw-3ds/content.txt').read_text().strip() not in EXPECTED_CONTENT:
        raise RuntimeError(say('Os dados convertidos nao correspondem ao executavel incluido.',
                               'The converted data does not match the bundled executable.'))
    destination = ROOT / 'PARA_CONSOLA/APPS'
    print(say('Etapa 4/4: juntar a aplicacao final e os teus dados convertidos.',
              'Stage 4/4: assemble the final application and your converted data.'))
    # Do not inherit data from a previously populated source APPS directory.
    def skip_data(directory, names):
        return names if Path(directory).resolve() == (ROOT / 'APPS/MarioKartWii/3ds').resolve() else []
    shutil.copytree(ROOT / 'APPS', destination, dirs_exist_ok=True, ignore=skip_data)
    shutil.copytree(content, destination / 'MarioKartWii/3ds', dirs_exist_ok=True)
    print('\n' + say('PRONTO. Copia o CONTEUDO de PARA_CONSOLA/APPS para APPS do cartao TF2.',
                      'DONE. Copy the CONTENTS of PARA_CONSOLA/APPS into APPS on the TF2 card.'))
    print(say('Na RGDS+, abre APPS > MarioKartWii. Nao copies a ROM para a consola.',
              'On RGDS+, open APPS > MarioKartWii. Do not copy the disc image to the console.'))


def main():
    parser = argparse.ArgumentParser(description='MKW RGDS+ local disc preparation')
    parser.add_argument('--lang', choices=['pt', 'en'], default='pt')
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--disc')
    args = parser.parse_args()
    global LANG
    LANG = args.lang
    print(say('Mario Kart Wii for RGDS+ - Preparador PT', 'Mario Kart Wii for RGDS+ - EN preparation tool'))
    requirements()
    if args.check:
        print(say('Ferramentas base prontas. DolphinTool so e necessario para RVZ.',
                  'Base tools ready. DolphinTool is only needed for RVZ.'))
        return
    disc = choose_disc(args.disc)
    free = shutil.disk_usage(ROOT).free / 2**30
    print(say('Espaco livre: ', 'Free space: ') + f'{free:.1f} GiB')
    if free < 20:
        raise RuntimeError(say('Reserva pelo menos 20 GiB livres antes de comecar.',
                               'Please provide at least 20 GiB of free space before starting.'))
    if disc.suffix.lower() == '.rvz':
        disc = convert_rvz(disc)
    else:
        check_region(disc)
        print(say('Etapa 1/4: ISO/WBFS validado; nao e preciso converter o formato.',
                  'Stage 1/4: ISO/WBFS validated; no container conversion needed.'))
    python = prepare_environment()
    print(say('Etapa 3/4: converter dados do jogo; pode demorar uma hora ou mais.',
              'Stage 3/4: convert game assets; this may take an hour or longer.'))
    run([python, 'tools/release/build_h700.py', disc, '--jobs', '2'])
    assemble(BUILDER / 'build/release/sd-card/3ds')


if __name__ == '__main__':
    os.environ['PYTHONUTF8'] = '1'
    os.environ['PYTHONUNBUFFERED'] = '1'
    class Tee:
        def __init__(self, console, log): self.console, self.log = console, log
        def write(self, text):
            self.console.write(text); self.log.write(text); self.flush(); return len(text)
        def flush(self): self.console.flush(); self.log.flush()
    with (PC / 'Preparacao.txt').open('w', encoding='utf-8') as log:
        oldout, olderr = sys.stdout, sys.stderr
        sys.stdout, sys.stderr = Tee(oldout, log), Tee(olderr, log)
        try:
            main()
        except (OSError, ValueError, RuntimeError, zipfile.BadZipFile, EOFError) as error:
            print('\n' + say('ERRO: ', 'ERROR: ') + str(error), file=sys.stderr)
            print(say('Registo: PC/Preparacao.txt. A ROM original foi preservada.',
                      'Log: PC/Preparacao.txt. The original disc image was preserved.'), file=sys.stderr)
            sys.exit(1)
        except KeyboardInterrupt:
            print(say('\nInterrompido. Podes voltar a executar para retomar as etapas concluidas.',
                      '\nInterrupted. Run again to resume completed stages.'))
            sys.exit(130)
        finally:
            sys.stdout, sys.stderr = oldout, olderr
