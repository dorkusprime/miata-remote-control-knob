#!/usr/bin/env python3
"""Generate a provisional IC pin-placement study, not a routed soldering map."""
import argparse
import html
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
TARGET = ROOT / 'hardware/sony-remote/placement.svg'
PARTS = [
    ('U1', 'MCP41050-I/P', 1, 4, '#0f766e',
     ['CS', 'SCK', 'SI', 'GND'], ['+5V', 'PA: NC', 'PW', 'GND']),
    ('U2', 'SN74HCT125N', 6, 7, '#1d4ed8',
     ['GND', 'CS in', 'CS out', 'GND', 'SCK in', 'SCK out', 'GND'],
     ['+5V', '+5V', 'GND', 'NC', 'GND', 'MOSI in', 'MOSI out']),
    ('K1', 'LCC110', 14, 4, '#7c3aed',
     ['NC', 'LED +', 'LED minus', 'NC'], ['NC contact', 'NC contact', 'PW', 'R3 / input']),
]


def render():
    s = ['<svg xmlns="http://www.w3.org/2000/svg" width="1260" height="1080" viewBox="0 0 1260 1080">',
         '<title>Current Sony interface — proposed DIP placement and pin reference</title>',
         '<desc>Provisional top view. U1 rows 1–4, U2 rows 6–12, K1 rows 14–17. Passives, sockets, mounting keepouts and jumpers are unplaced. Not a soldering map.</desc>',
         '<rect width="1260" height="1080" fill="#f8fafc"/>',
         '<style>text{font-family:Arial,sans-serif;fill:#172033}</style>']

    def text(x, y, label, size=15, color='#172033'):
        s.append(f'<text x="{x}" y="{y}" font-size="{size}" style="fill:{color}">{html.escape(label)}</text>')

    def rect(x, y, w, h, fill, stroke='none', radius=0):
        s.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{radius}" fill="{fill}" stroke="{stroke}"/>')

    def line(x, y, x2, y2, color, width=2):
        s.append(f'<path d="M{x},{y} L{x2},{y2}" fill="none" stroke="{color}" stroke-width="{width}"/>')

    def circle(x, y, radius, fill, stroke='none'):
        s.append(f'<circle cx="{x}" cy="{y}" r="{radius}" fill="{fill}" stroke="{stroke}"/>')

    def xy(col, row):
        return (55 + (5.08 + ('ABCDEFGHIJ'.index(col) + (2 if col >= 'F' else 0)) * 2.54) * 12,
                235 + (5.08 + (row - 1) * 2.54) * 12)

    text(35, 43, 'Sony remote interface · proposed DIP placement', 27)
    text(35, 74, 'MCP41050 + HCT125 + LCC110 · TOP / COMPONENT VIEW', 18)
    rect(35, 96, 1190, 83, '#fff3cd', radius=8)
    text(52, 122, 'PROVISIONAL FIT STUDY — not a soldering or jumper map.', 19, '#92400e')
    text(52, 147, 'IC pin positions only. Actual bodies, sockets and mounting-hole keepouts still need a dry-fit.', 16, '#92400e')
    text(52, 169, 'All seven resistors, Q1, three capacitors and cable connections remain to be placed.', 15, '#92400e')
    text(55, 213, 'Nominal ElectroCookie Mini · 38.1 × 50.8 mm', 16)
    rect(55, 235, 38.1 * 12, 50.8 * 12, '#e6edf3', '#94a3b8', 16)
    for row in range(1, 18):
        for group in ('ABCDE', 'FGHIJ'):
            x, y = xy(group[0], row)
            x2, y2 = xy(group[-1], row)
            line(x, y, x2, y2, '#b3c4d0', 5)
        for col in 'ABCDEFGHIJ':
            x, y = xy(col, row)
            circle(x, y, 4, '#f8fafc', '#71869b')
        text(64, xy('A', row)[1] + 4, str(row), 12)
    for col in 'ABCDEFGHIJ':
        text(xy(col, 1)[0] - 5, 263, col, 13)
    for ref, name, start, n, color, left, right in PARTS:
        x, y = xy('E', start)
        xf, _ = xy('F', start)
        _, yb = xy('E', start + n - 1)
        rect(x + 7, y - 12, xf - x - 14, yb - y + 24, color, radius=6)
        text(x + 15, (y + yb) / 2 + 6, ref, 19, '#fff')
        circle(x + 18, y - 3, 3, '#fff')
        for i in range(n):
            xx, yy = xy('E', start + i)
            line(xx, yy, xx + 7, yy, color)
            text(xx - 23, yy + 4, str(i + 1), 11, color)
            xx, yy = xy('F', start + i)
            line(xx - 7, yy, xx, yy, color)
            text(xx + 9, yy + 4, str(2 * n - i), 11, color)
        cy = {1: 215, 6: 405, 14: 695}[start]
        rect(555, cy, 670, 72 + n * 22, '#fff', '#cbd5e1', 8)
        text(572, cy + 28, f'{ref} · {name} · rows {start}–{start + n - 1}', 18, color)
        text(572, cy + 53, 'LEFT: pin / pad / net', 13, '#64748b')
        text(886, cy + 53, 'RIGHT: pin / pad / net', 13, '#64748b')
        for i in range(n):
            text(572, cy + 77 + i * 22, f'{i + 1:2}   E{start + i:<2}   {left[i]}', 14)
            text(886, cy + 77 + i * 22, f'{2 * n - i:2}   F{start + i:<2}   {right[i]}', 14)
    text(55, 876, 'Gray bars = factory-connected five-pad strips.', 14)
    text(55, 901, 'Dot = pin 1. All notches face the top of the image.', 14)
    text(55, 926, 'Body rectangles are symbols, not clearance envelopes.', 14)
    rect(35, 959, 1190, 91, '#e2e8f0', radius=8)
    text(52, 984, 'Before soldering: check real board geometry, mounting holes, socket overlap and underside access.', 15)
    text(52, 1009, 'Rows 5 and 13 are spacer rows. Side pads are already joined to the corresponding IC pin.', 15)
    text(52, 1034, 'NC = leave unconnected; NC contact = unused normally-closed contact. No routing or fit is validated.', 15)
    s.append('</svg>')
    data = '\n'.join(s) + '\n'
    ET.fromstring(data)
    return data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    data = render()
    if args.check:
        if TARGET.read_text() != data:
            raise SystemExit('Regenerate: python3 scripts/placement-study.py')
        print('Placement SVG matches generator; physical/electrical acceptance remains pending.')
    else:
        TARGET.write_text(data)
        print('Wrote hardware/sony-remote/placement.svg (provisional fit study).')


if __name__ == '__main__':
    main()
