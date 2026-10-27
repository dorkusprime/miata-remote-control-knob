import importlib.util
import pathlib
import struct
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location("memory_usage", ROOT / "scripts/memory-usage.py")
memory = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(memory)


class MemoryTests(unittest.TestCase):
    def test_linker_limits_with_mixed_units(self):
        source = """
          FLASH(rx) : ORIGIN = 0x10000000, LENGTH = 2093056
          RAM(rwx) : ORIGIN = 0x20000000, LENGTH = 256k
        """
        self.assertEqual(memory.memory_regions(source), {
            "FLASH": (0x10000000, 2093056), "RAM": (0x20000000, 262144)})

    def test_missing_limits_fail(self):
        with self.assertRaisesRegex(ValueError, "Missing"):
            memory.memory_regions("MEMORY {}")

    def test_truncated_elf_fails(self):
        with self.assertRaises(struct.error):
            memory.usage(b"bad", "FLASH(rx): ORIGIN=0x10000000, LENGTH=2M\nRAM(rwx): ORIGIN=0x20000000, LENGTH=256K\n")

    def test_real_build_flash_matches_uploaded_binary(self):
        # Independent build artifact includes boot metadata and .data load image.
        checked = 0
        for target in ("controller", "diagnostic"):
            folder = ROOT / "build" / (target + "-picotool")
            elf, binary = folder / (target + ".ino.elf"), folder / (target + ".ino.bin")
            if not elf.exists() or not binary.exists():
                continue
            with self.subTest(target=target):
                result = memory.usage(elf.read_bytes(), (folder / "memmap_default.ld").read_text())
                self.assertEqual(result["flash"][0], binary.stat().st_size)
                self.assertGreater(result["ram"][0], 0)
                self.assertLess(result["ram"][0], result["ram"][1])
                self.assertIn("peak runtime usage is not measured", memory.report(result, target))
                checked += 1
        if not checked:
            self.skipTest("Build a sketch to compare ELF with generated binary")


if __name__ == "__main__":
    unittest.main()
