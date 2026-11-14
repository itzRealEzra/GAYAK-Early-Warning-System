"""Colored startup display and source-tree view for the GAYAK console."""

from __future__ import annotations

import ctypes
import os
import sys
from pathlib import Path


_ANSI_ENABLED = False
_COLORS = {
    "violet": "\x1b[38;5;141m",
    "cyan": "\x1b[38;5;81m",
    "green": "\x1b[38;5;114m",
    "yellow": "\x1b[38;5;221m",
    "red": "\x1b[38;5;203m",
    "muted": "\x1b[38;5;245m",
    "bold": "\x1b[1m",
}
_RESET = "\x1b[0m"
_SKIP_DIRECTORIES = {".git", ".venv", "venv", "__pycache__", ".obsidian"}


def enable_ansi() -> bool:
    """Enable colored terminal sequences when stdout is an interactive console."""
    global _ANSI_ENABLED
    if not sys.stdout.isatty():
        return False

    if os.name == "nt":
        try:
            kernel32 = ctypes.windll.kernel32
            stdout_handle = kernel32.GetStdHandle(-11)
            current_mode = ctypes.c_uint()
            if not kernel32.GetConsoleMode(stdout_handle, ctypes.byref(current_mode)):
                return False
            if not kernel32.SetConsoleMode(stdout_handle, current_mode.value | 0x0004):
                return False
        except (AttributeError, OSError):
            return False

    _ANSI_ENABLED = True
    return True


def color(text: str, name: str) -> str:
    """Return text styled for the console, or plain text when colors are unavailable."""
    code = _COLORS.get(name)
    if not _ANSI_ENABLED or code is None:
        return text
    return f"{code}{text}{_RESET}"


def status(label: str, message: str) -> None:
    """Print a color-coded console status line."""
    label_colors = {
        "READ": "violet",
        "READY": "green",
        "INFO": "cyan",
        "WARN": "yellow",
        "ERROR": "red",
    }
    print(f"{color(f'[{label}]', label_colors.get(label, 'muted'))} {message}", flush=True)


def _tree_entries(directory: Path) -> list[Path]:
    entries = [
        child for child in directory.iterdir()
        if not (child.is_dir() and child.name in _SKIP_DIRECTORIES)
    ]
    return sorted(entries, key=lambda child: (not child.is_dir(), child.name.casefold()))


def print_project_tree(root: Path) -> int:
    """Print every project source file in a compact tree, excluding local runtime data."""
    print(color(root.name, "cyan"))
    file_count = 0

    def walk(directory: Path, prefix: str = "") -> None:
        nonlocal file_count
        entries = _tree_entries(directory)
        for index, entry in enumerate(entries):
            last = index == len(entries) - 1
            branch = "`-- " if last else "|-- "
            suffix = "/" if entry.is_dir() else ""
            name = color(entry.name + suffix, "cyan" if entry.is_dir() else "muted")
            print(f"{prefix}{branch}{name}")
            if entry.is_dir():
                walk(entry, prefix + ("    " if last else "|   "))
            else:
                file_count += 1

    walk(root)
    return file_count


def print_startup(port: str) -> None:
    """Print the GAYAK banner and the project's source tree."""
    enable_ansi()
    bar = "============================================================"
    print()
    print(color(bar, "violet"))
    print(color("                         G A Y A K", "bold"))
    print(color("                 EARLY WARNING SYSTEM", "cyan"))
    print(color(bar, "violet"))
    print(f"Developer: {color('Ezra', 'violet')}")
    print(f"Dashboard: {color(f'http://127.0.0.1:{port}', 'cyan')}")
    status("READY", "Python environment is ready.")
    status("READ", "Project files:")
    count = print_project_tree(Path(__file__).resolve().parent)
    print(color(f"{count} project files listed.", "muted"))
    print()


def main(argv: list[str] | None = None) -> int:
    args = sys.argv[1:] if argv is None else argv
    if not args:
        print_startup(os.environ.get("GAYAK_PORT", "5000"))
        return 0

    command = args[0].lower()
    if command == "startup":
        print_startup(args[1] if len(args) > 1 else os.environ.get("GAYAK_PORT", "5000"))
        return 0
    if command == "status":
        enable_ansi()
        if len(args) < 3:
            print("Usage: console_ui.py status LABEL MESSAGE")
            return 2
        status(args[1].upper(), " ".join(args[2:]))
        return 0

    print("Usage: console_ui.py [startup [PORT] | status LABEL MESSAGE]")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
