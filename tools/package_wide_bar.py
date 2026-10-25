"""Package reviewed source and record changes against the user-supplied ZIP.

Usage: python tools/package_wide_bar.py BASELINE.zip OUTPUT_DIRECTORY
Run from the package root. Build products are deliberately excluded.
"""
import hashlib
import json
from pathlib import Path
import sys
import zipfile

root = Path(__file__).resolve().parents[1]
baseline = Path(sys.argv[1])
output = Path(sys.argv[2]).resolve()
output.mkdir(parents=True, exist_ok=True)

def source_files():
    for path in sorted(root.rglob("*")):
        if not path.is_file():
            continue
        relative = path.relative_to(root)
        if any(part.startswith("build") or part in {"bin", "obj", ".vs", "__pycache__"}
               for part in relative.parts[:-1]):
            continue
        if relative.parts[0] == "validation" and path.suffix not in {".png", ".log"}:
            continue
        if path.suffix in {".exe", ".obj", ".pdb", ".ilk", ".lib", ".idb", ".zip", ".pyc"}:
            continue
        if path.suffix == ".log" and relative.parts[0] != "validation":
            continue
        yield path, relative.as_posix()

with zipfile.ZipFile(baseline) as original:
    old = {name.removeprefix("Neutrino/"): original.read(name)
           for name in original.namelist() if not name.endswith("/")}

changes = []
for path, name in source_files():
    if name == "WIDE_BAR_SOURCE_CHANGES.json":
        continue
    data = path.read_bytes()
    if old.get(name) != data:
        changes.append({"path": name, "change": "modified" if name in old else "added",
                        "sha256": hashlib.sha256(data).hexdigest()})
(root / "WIDE_BAR_SOURCE_CHANGES.json").write_text(json.dumps({
    "baseline_sha256": hashlib.sha256(baseline.read_bytes()).hexdigest(),
    "files": changes,
}, indent=2) + "\n", encoding="utf-8")

archive = output / "Neutrino_WideBar_Source.zip"
with zipfile.ZipFile(archive, "w", zipfile.ZIP_DEFLATED) as packaged:
    for path, name in source_files():
        packaged.write(path, "Neutrino/" + name)
with zipfile.ZipFile(archive) as packaged:
    assert packaged.testzip() is None
    names = packaged.namelist()
    assert all("/build/" not in name and "/bin/" not in name for name in names)
    for required in ("tests/wide_bar_tests.cpp", "docs/WIDE_BAR.md",
                     "examples/neutrino_test_23_wide_bar.cpp",
                     "examples/neutrino_test_24_main_window_menu.cpp"):
        assert "Neutrino/" + required in names
print(f"{archive.name}: {len(names)} files, {archive.stat().st_size} bytes")
print("SHA256:", hashlib.sha256(archive.read_bytes()).hexdigest())
