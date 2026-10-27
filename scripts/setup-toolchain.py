#!/usr/bin/env python3
"""Install the pinned project toolchain on Apple Silicon macOS; never flash hardware."""
import concurrent.futures
import hashlib
import json
import pathlib
import platform
import subprocess
import tarfile

ROOT = pathlib.Path(__file__).resolve().parents[1]
LOCK = json.loads((ROOT / "firmware/toolchain.json").read_text())
BASE = ROOT / ".tools/arduino"
DATA = BASE / "data"
DOWNLOADS = ROOT / ".tools/downloads"
CLI = ROOT / ".tools/bin/arduino-cli"
CONFIG = BASE / "config.yaml"


def download(url, dest):
    dest.parent.mkdir(parents=True, exist_ok=True)
    temp = dest.with_name(dest.name + ".partial")
    subprocess.run(["curl", "-fsSL", "--retry", "2", url, "-o", str(temp)], check=True)
    temp.replace(dest)


def digest(path, algorithm="sha256"):
    h = hashlib.new(algorithm)
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def mac_archive(tool):
    choices = [s for s in tool["systems"] if "apple-darwin" in s["host"]]
    return next((s for s in choices if "aarch64" in s["host"] or "arm64" in s["host"]), choices[0])


def cache_archive(item):
    category = "libraries" if item.get("library") else "packages"
    dest = BASE / "downloads" / category / item["archiveFileName"]
    algorithm, expected = item["checksum"].split(":", 1)
    if not dest.exists() or digest(dest, algorithm.lower()) != expected.lower():
        print("Downloading " + dest.name, flush=True)
        download(item["url"], dest)
    if digest(dest, algorithm.lower()) != expected.lower():
        dest.unlink()
        raise RuntimeError("Checksum mismatch: " + dest.name)
    print("Verified " + dest.name, flush=True)


def main():
    if platform.system() != "Darwin" or platform.machine() != "arm64":
        raise SystemExit("This bootstrap supports Apple Silicon macOS. See firmware/README.md for other hosts.")
    for path in (DATA, BASE / "downloads", BASE / "user", CLI.parent):
        path.mkdir(parents=True, exist_ok=True)
    version = LOCK["arduino_cli"]
    filename = f"arduino-cli_{version}_macOS_ARM64.tar.gz"
    release = f"https://github.com/arduino/arduino-cli/releases/download/v{version}"
    checks = DOWNLOADS / "cli-checksums.txt"
    download(f"{release}/{version}-checksums.txt", checks)
    expected = next(line.split()[0] for line in checks.read_text().splitlines() if line.split()[-1] == filename)
    archive = DOWNLOADS / filename
    if not archive.exists() or digest(archive) != expected:
        download(f"{release}/{filename}", archive)
    if digest(archive) != expected:
        raise RuntimeError("Arduino CLI checksum mismatch")
    subprocess.run(["tar", "-xzf", str(archive), "-C", str(CLI.parent), "arduino-cli"], check=True)
    CONFIG.write_text(json.dumps({
        "board_manager": {"additional_urls": [LOCK["additional_url"]]},
        "directories": {"data": str(DATA), "downloads": str(BASE / "downloads"), "user": str(BASE / "user")},
    }, indent=2) + "\n")
    # curl is used because CLI's own downloader cannot resolve hosts in some managed environments.
    for kind, url in (
        ("package", "https://downloads.arduino.cc/packages/package_index.tar.bz2"),
        ("library", "https://downloads.arduino.cc/libraries/library_index.tar.bz2"),
    ):
        archive = DOWNLOADS / f"{kind}_index.tar.bz2"
        download(url, archive)
        # Extract only known index files, never arbitrary archive paths.
        with tarfile.open(archive) as source:
            for suffix in (".json", ".json.sig"):
                name = f"{kind}_index{suffix}"
                stream = source.extractfile(name)
                if stream is None:
                    raise RuntimeError("Missing index " + name)
                (DATA / name).write_bytes(stream.read())
    download(LOCK["additional_url"], DATA / "package_rp2040_index.json")
    packages = json.loads((DATA / "package_index.json").read_text())["packages"]
    rp = json.loads((DATA / "package_rp2040_index.json").read_text())["packages"][0]
    core = next(p for p in rp["platforms"] if p["version"] == LOCK["core_version"])
    items = [core]
    for dep in core["toolsDependencies"]:
        items.append(mac_archive(next(t for t in rp["tools"] if t["name"] == dep["name"] and t["version"] == dep["version"])))
    builtin = next(p for p in packages if p["name"] == "builtin")
    seen = set()
    for tool in builtin["tools"]:
        if tool["name"] not in seen:
            items.append(mac_archive(tool))
            seen.add(tool["name"])
    libraries = json.loads((DATA / "library_index.json").read_text())["libraries"]
    lib = next(l for l in libraries if l["name"] == LOCK["library"] and l["version"] == LOCK["library_version"])
    items.append(dict(lib, library=True))
    with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool:
        list(pool.map(cache_archive, items))
    def cli(*args):
        subprocess.run([str(CLI), "--config-file", str(CONFIG), *args], check=True)
    cli("core", "install", LOCK["core"] + "@" + LOCK["core_version"])
    cli("lib", "install", LOCK["library"] + "@" + LOCK["library_version"])
    print("Toolchain installed. No hardware has been flashed.")


if __name__ == "__main__":
    main()
