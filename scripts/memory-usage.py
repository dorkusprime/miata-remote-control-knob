#!/usr/bin/env python3
"""Report the selected RP2040 ELF's flash footprint and static main-RAM use."""
import argparse
import pathlib
import re
import struct

RAM_SECTIONS = {".data", ".bss", ".ram_vector_table", ".uninitialized_ram",
                ".uninitialized_data", ".tdata", ".tbss"}


def memory_regions(linker):
    pattern = (r"^\s*(FLASH|RAM)\s*\([^)]*\)\s*:\s*ORIGIN\s*=\s*(0x[\da-fA-F]+|\d+)"
               r"\s*,\s*LENGTH\s*=\s*(\d+)[ \t]*([kKmM]?)\b")
    regions = {}
    for name, origin, length, unit in re.findall(pattern, linker, re.MULTILINE):
        regions[name] = (int(origin, 0), int(length) * {"": 1, "k": 1024, "m": 1024 ** 2}[unit.lower()])
    if set(regions) != {"FLASH", "RAM"}:
        raise ValueError("Missing FLASH/RAM limits in generated linker script; rebuild first.")
    return regions


def usage(elf, linker):
    regions = memory_regions(linker)
    header = struct.unpack_from("<16sHHIIIIIHHHHHH", elf)
    ident, _, machine, _, _, phoff, shoff, _, _, phsize, phnum, shsize, shnum, strings_index = header
    if ident[:7] != b"\x7fELF\x01\x01\x01" or machine != 40 or phsize < 32 or shsize < 40:
        raise ValueError("Expected a 32-bit little-endian ARM ELF; rebuild for XIAO RP2040.")
    flash_origin, flash_limit = regions["FLASH"]
    flash_end = flash_origin
    for index in range(phnum):
        kind, _, _, physical, file_size, _, _, _ = struct.unpack_from("<IIIIIIII", elf, phoff + index * phsize)
        if kind == 1 and file_size and flash_origin <= physical < flash_origin + flash_limit:
            flash_end = max(flash_end, physical + file_size)
    if flash_end == flash_origin or flash_end > flash_origin + flash_limit:
        raise ValueError("Missing or oversized flash image in ELF.")
    if strings_index >= shnum:
        raise ValueError("Invalid ELF section-name table.")
    headers = [struct.unpack_from("<IIIIIIIIII", elf, shoff + index * shsize) for index in range(shnum)]
    string_header = headers[strings_index]
    strings = elf[string_header[4]:string_header[4] + string_header[5]]
    ram = 0
    ram_origin, ram_limit = regions["RAM"]
    for name_offset, _, _, address, _, size, _, _, _, _ in headers:
        name = strings[name_offset:].split(b"\0", 1)[0].decode("ascii", errors="replace")
        if name in RAM_SECTIONS:
            if size and not ram_origin <= address < ram_origin + ram_limit:
                raise ValueError("Static section outside configured main RAM.")
            ram += size
    if ram > ram_limit:
        raise ValueError("Static RAM exceeds configured capacity.")
    return {"flash": (flash_end - flash_origin, flash_limit), "ram": (ram, ram_limit)}


def report(result, name):
    lines = [f"Firmware memory — {name}"]
    for label, key in (("Flash image", "flash"), ("Static RAM", "ram")):
        used, capacity = result[key]
        lines.append(f"  {label:11s} {used:>9,} / {capacity:>9,} bytes "
                     f"({100 * used / capacity:.1f}%; {(capacity - used) / 1024:.1f} KiB remaining)")
    lines.append("  RAM remaining is for runtime heap/stack; peak runtime usage is not measured.")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=pathlib.Path)
    args = parser.parse_args()
    try:
        result = usage(args.elf.read_bytes(), (args.elf.parent / "memmap_default.ld").read_text())
    except (OSError, ValueError, struct.error) as error:
        parser.exit(1, f"Cannot report memory: {error}\nRebuild the selected sketch first.\n")
    print(report(result, args.elf.name))


if __name__ == "__main__":
    main()
