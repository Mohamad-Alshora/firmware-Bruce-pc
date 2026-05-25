#!/usr/bin/env python3
"""
Patch script to fix LovyanGFX bool redefinition issue with Arduino 3.3.6
This fixes the compilation error in lgfx_qrcode.h
"""
import os
import re

def patch_qrcode_header():
    """Patch the lgfx_qrcode.h file to fix bool redefinition"""
    qrcode_header = os.path.join(
        os.environ.get('PROJECT_LIBDEPS_DIR', '.pio/libdeps'),
        'lilygo-t-display-ttgo',
        'LovyanGFX',
        'src',
        'lgfx',
        'utility',
        'lgfx_qrcode.h'
    )

    if not os.path.exists(qrcode_header):
        print(f"[PATCH] lgfx_qrcode.h not found at {qrcode_header}")
        return

    with open(qrcode_header, 'r', encoding='utf-8') as f:
        content = f.read()

    # Check if already patched
    if '#include <stdbool.h>' in content:
        print("[PATCH] lgfx_qrcode.h already patched")
        return

    # Add stdbool.h include before the problematic typedef
    original = content

    # Find the problematic lines and comment them out
    content = re.sub(
        r'typedef\s+unsigned\s+char\s+bool;',
        '// typedef unsigned char bool;  // PATCHED: Already defined in stdbool.h',
        content
    )
    content = re.sub(
        r'static\s+const\s+bool\s+false\s*=\s*0;',
        '// static const bool false = 0;  // PATCHED: Already defined in stdbool.h',
        content
    )
    content = re.sub(
        r'static\s+const\s+bool\s+true\s*=\s*1;',
        '// static const bool true = 1;  // PATCHED: Already defined in stdbool.h',
        content
    )

    # Add stdbool.h include at the beginning if not present
    if '#include <stdbool.h>' not in content:
        # Find the first include and add stdbool.h before it
        include_match = re.search(r'(#ifndef.*?\n)', content)
        if include_match:
            pos = include_match.end()
            content = content[:pos] + '#include <stdbool.h>\n' + content[pos:]

    if content != original:
        with open(qrcode_header, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"[PATCH] Successfully patched lgfx_qrcode.h")
    else:
        print("[PATCH] No changes needed")

if __name__ == '__main__':
    patch_qrcode_header()
