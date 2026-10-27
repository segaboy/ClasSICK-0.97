#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
"""Export the original synthetic sampling diagram;no hardware or emulator calls."""
import argparse
from pathlib import Path
import xml.etree.ElementTree as ET

SVG = '<!-- SPDX-License-Identifier: GPL-3.0-or-later; Copyright (C) 2026 Dean Howell. -->\n<svg xmlns="http://www.w3.org/2000/svg" width="1120" height="580" viewBox="0 0 1120 580" role="img" aria-labelledby="title desc">\n<title id="title">Same endpoints, opposite output</title>\n<desc id="desc">Original ClasSICK software-model example. Observing the three forward phase edges01,11,10,00 emits minus one X unit. Observing only01 then00 emits plus one. This is a synthetic model example,not a captured hardware trace.</desc>\n<rect width="1120" height="580" fill="white"/>\n<text x="50" y="61" font-family="Arial,sans-serif" font-size="36" font-weight="700" fill="#20252a">Same endpoints, opposite output</text>\n<text x="50" y="103" font-family="Arial,sans-serif" font-size="23" font-weight="400" fill="#20252a">A reproducible example from ClasSICK’s original phase decoder</text>\n<text x="50" y="183" font-family="Arial,sans-serif" font-size="25" font-weight="700" fill="#20252a">Every edge</text>\n<text x="50" y="214" font-family="Arial,sans-serif" font-size="21" font-weight="400" fill="#20252a">is observed</text>\n<rect x="330" y="150" width="104" height="74" rx="10" fill="#edf4f3" stroke="#1c7467" stroke-width="2"/>\n<text x="357" y="198" font-family="Arial,sans-serif" font-size="36" font-weight="700" fill="#20252a">01</text>\n<rect x="490" y="150" width="104" height="74" rx="10" fill="#edf4f3" stroke="#1c7467" stroke-width="2"/>\n<text x="517" y="198" font-family="Arial,sans-serif" font-size="36" font-weight="700" fill="#20252a">11</text>\n<rect x="650" y="150" width="104" height="74" rx="10" fill="#edf4f3" stroke="#1c7467" stroke-width="2"/>\n<text x="677" y="198" font-family="Arial,sans-serif" font-size="36" font-weight="700" fill="#20252a">10</text>\n<rect x="810" y="150" width="104" height="74" rx="10" fill="#edf4f3" stroke="#1c7467" stroke-width="2"/>\n<text x="837" y="198" font-family="Arial,sans-serif" font-size="36" font-weight="700" fill="#20252a">00</text>\n<path d="M441 187H471" fill="none" stroke="#545f67" stroke-width="3"/><path d="M481 187l-12 -7v14z" fill="#545f67"/>\n<path d="M601 187H631" fill="none" stroke="#545f67" stroke-width="3"/><path d="M641 187l-12 -7v14z" fill="#545f67"/>\n<path d="M761 187H791" fill="none" stroke="#545f67" stroke-width="3"/><path d="M801 187l-12 -7v14z" fill="#545f67"/>\n<text x="330" y="267" font-family="Arial,sans-serif" font-size="24" font-weight="700" fill="#1c7467">Three forward phase edges: emitted X = −1</text>\n<text x="50" y="375" font-family="Arial,sans-serif" font-size="25" font-weight="700" fill="#20252a">Only endpoints</text>\n<text x="50" y="406" font-family="Arial,sans-serif" font-size="21" font-weight="400" fill="#20252a">are observed</text>\n<rect x="330" y="340" width="104" height="74" rx="10" fill="#edf4f3" stroke="#1c7467" stroke-width="2"/>\n<text x="357" y="388" font-family="Arial,sans-serif" font-size="36" font-weight="700" fill="#20252a">01</text>\n<rect x="810" y="340" width="104" height="74" rx="10" fill="#edf4f3" stroke="#1c7467" stroke-width="2"/>\n<text x="837" y="388" font-family="Arial,sans-serif" font-size="36" font-weight="700" fill="#20252a">00</text>\n<path d="M445 377H790" fill="none" stroke="#545f67" stroke-width="3"/><path d="M800 377l-12 -7v14z" fill="#545f67"/>\n<text x="465" y="356" font-family="Arial,sans-serif" font-size="19" font-weight="400" fill="#20252a">intermediate states not sampled</text>\n<text x="330" y="459" font-family="Arial,sans-serif" font-size="24" font-weight="700" fill="#a3442b">The endpoint step emits X = +1</text>\n<text x="50" y="528" font-family="Arial,sans-serif" font-size="18" font-weight="400" fill="#20252a">Synthetic model example • X axis, invert 0, button up • No timing or hardware cause inferred</text>\n<text x="50" y="559" font-family="Arial,sans-serif" font-size="17" font-weight="400" fill="#20252a">Reproduced with pinned Clang and GCC • 27 October 2026 • Research, not release qualification</text>\n</svg>\n'

def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', required=True, type=Path)
    a = ap.parse_args()
    path = a.output.resolve()
    repo = Path(__file__).resolve().parents[2]
    if path.is_relative_to(repo) or path.exists() or not path.parent.is_dir():
        ap.error('output must be a new file in an existing directory outside the repo')
    ET.fromstring(SVG)
    with path.open('x', encoding='utf-8', newline='\n') as stream:
        stream.write(SVG)
    print(path)
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
