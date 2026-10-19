"""Front-end tests use synthetic headers and stubbed cooking, not a Nintendo ROM."""
import base64
import contextlib
import hashlib
import importlib.util
import io
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from types import SimpleNamespace
from unittest.mock import patch
import zipfile

PROJECT = Path(__file__).resolve().parents[1]


def load_module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class PreparationTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.pc = self.root / 'PC'
        self.pc.mkdir()
        (self.root / 'ROM').mkdir()
        app = self.root / 'APPS/MarioKartWii'
        app.mkdir(parents=True)
        (self.root / 'APPS/MarioKartWii.sh').write_text('launcher fixture')
        (app / 'mkw3ds').write_text('executable fixture')
        (self.root / 'APPS/Imgs').mkdir()
        self.icon = self.root / 'APPS/Imgs/MarioKartWii.png'
        self.icon.write_bytes(base64.b64decode('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+aT1sAAAAASUVORK5CYII='))
        self.m = load_module('prep_test', PROJECT / 'PC/preparar.py')
        self.m.ROOT, self.m.PC = self.root, self.pc
        self.m.BUILDER = self.pc / 'builder'

    def cooked(self, stamp='cf69fd08237c86f0faa555dbecece969'):
        result = self.m.BUILDER / 'build/release/sd-card/3ds'
        (result / 'mkw-3ds/Race').mkdir(parents=True, exist_ok=True)
        (result / 'mkw-3ds/content.txt').write_text(stamp)
        (result / 'mkw-3ds/Race/Common.szs').write_bytes(b'synthetic test data')
        return result

    def test_complete_iso_frontend_pt_and_en(self):
        for lang in ['pt', 'en']:
            disc = self.root / 'ROM/Mario Kart Wii with spaces.iso'
            disc.write_bytes(b'RMCE01' + b'\0' * 512)
            original = disc.read_bytes()
            def fake_cooker(args, cwd=None):
                self.assertEqual(args[1], 'tools/release/build_h700.py')
                self.assertEqual(Path(args[2]), disc)
                self.cooked()
            output = io.StringIO()
            with patch.object(self.m, 'requirements'), patch.object(self.m, 'prepare_environment', return_value=Path('python')):
                with patch.object(self.m, 'run', side_effect=fake_cooker), patch.object(self.m.shutil, 'disk_usage', return_value=SimpleNamespace(free=50 * 2**30)):
                    with patch.object(sys, 'argv', ['preparar.py', '--lang', lang]), contextlib.redirect_stdout(output):
                        self.m.main()
            self.assertIn('PRONTO' if lang == 'pt' else 'DONE', output.getvalue())
            self.assertEqual(disc.read_bytes(), original)
            ready = self.root / 'PARA_CONSOLA/APPS'
            self.assertTrue((ready / 'MarioKartWii/3ds/mkw-3ds/content.txt').is_file())
            self.assertEqual((ready / 'Imgs/MarioKartWii.png').read_bytes(), self.icon.read_bytes())
            self.assertFalse(list(ready.rglob('*.iso')))

    def test_wbfs_header_and_wrong_region(self):
        path = self.root / 'ROM/test.wbfs'
        header = bytearray(512)
        header[:4], header[8], header[12] = b'WBFS', 9, 1
        path.write_bytes(header + b'RMCE01')
        self.m.check_region(path)
        path.write_bytes(header + b'RMCP01')
        with self.assertRaises(RuntimeError): self.m.check_region(path)

    def test_rvz_conversion_and_cache(self):
        path = self.root / 'ROM/with spaces.rvz'
        path.write_bytes(b'RVZ synthetic fixture')
        calls = []
        def converter(args, cwd=None):
            calls.append(args)
            Path(args[args.index('-o') + 1]).write_bytes(b'RMCE01' + b'\0' * 512)
        with patch.object(self.m, 'find_dolphin', return_value=Path('DolphinTool.exe')):
            with patch.object(self.m, 'run', side_effect=converter):
                result = self.m.convert_rvz(path)
                self.assertEqual(self.m.convert_rvz(path), result)
        self.assertEqual(len(calls), 1)
        self.assertEqual(calls[0][-2:], ['-f', 'iso'])
        self.assertEqual(path.read_bytes(), b'RVZ synthetic fixture')
        self.assertEqual(list((self.root / 'ROM').iterdir()), [path])

    def test_wrong_content_never_assembles(self):
        content = self.cooked(stamp='wrong version')
        with self.assertRaises(RuntimeError): self.m.assemble(content)
        self.assertFalse((self.root / 'PARA_CONSOLA').exists())

    def test_bundled_archive_integrity_and_restore(self):
        for name in ['builder.zip', 'builder.sha256']:
            shutil.copy2(PROJECT / 'PC' / name, self.pc / name)
        self.m.unpack_builder()
        self.assertTrue((self.m.BUILDER / 'tools/release/build_h700.py').is_file())
        archive = self.pc / 'builder.zip'
        archive.write_bytes(b'damaged')
        with self.assertRaises(RuntimeError): self.m.unpack_builder()

    def test_empty_rom_and_missing_tools_have_messages(self):
        self.m.LANG = 'en'
        with self.assertRaisesRegex(RuntimeError, 'ROM folder'): self.m.choose_disc()
        with patch.object(self.m.shutil, 'which', return_value=None), contextlib.redirect_stdout(io.StringIO()):
            with self.assertRaisesRegex(RuntimeError, 'Install the missing tools'): self.m.requirements()


class PackageTests(unittest.TestCase):
    def test_complete_release_and_private_data_exclusion(self):
        m = load_module('pack_test', PROJECT / 'tools/package_release.py')
        with tempfile.TemporaryDirectory() as t:
            root = Path(t) / 'repo'
            shutil.copytree(PROJECT, root, ignore=shutil.ignore_patterns('dist', '__pycache__'))
            m.ROOT, m.APP = root, root / 'APPS/MarioKartWii'
            # Local caches/output must never be selected for a public ZIP.
            for name in ['PC/work/private.iso', 'PC/builder/private.txt', 'PARA_CONSOLA/private.txt']:
                p = root / name; p.parent.mkdir(parents=True, exist_ok=True); p.write_text('PRIVATE')
            icon = root / 'APPS/Imgs/MarioKartWii.png'
            icon.write_bytes(base64.b64decode('iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAwMCAO+aT1sAAAAASUVORK5CYII='))
            with contextlib.redirect_stdout(io.StringIO()): m.main()
            with zipfile.ZipFile(root / 'dist/MarioKartWii_RGDSPlus_v1.1.0.zip') as z:
                names = set(z.namelist())
                self.assertTrue({'PC/builder.zip', 'PC/preparar.py', 'PREPARAR_ROM.bat', 'INICIO_PT.md', 'START_HERE_EN.md', 'APPS/Imgs/MarioKartWii.png'} <= names)
                self.assertFalse(any('private' in n for n in names))
            (root / 'ROM/user.rvz').write_text('PRIVATE')
            with self.assertRaises(ValueError): m.validate()


if __name__ == '__main__':
    unittest.main()
