"""Fault injection for the current circuit's proposed pad layout."""
import contextlib
import importlib.util
import io
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('full_placement', ROOT/'scripts/full-placement.py')
layout = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(layout)


class CurrentLayoutTests(unittest.TestCase):
    def setUp(self):
        self.data = json.loads((ROOT/'hardware/sony-remote/layout.json').read_text())

    def check(self):
        with contextlib.redirect_stdout(io.StringIO()):layout.check(self.data)

    def part(self, ref):
        return next(p for p in self.data['parts'] if p['ref'] == ref)

    def test_complete_current_layout(self):
        self.check()

    def test_power_ground_short(self):
        self.data['jumpers'].append({'ref':'FAULT','pads':['H1','I10'], 'net':'+5V',
                                    'route':[layout.pad_xy('H1'),layout.pad_xy('I10')]})
        with self.assertRaisesRegex(AssertionError,'Short'):self.check()

    def test_missing_clock_jumper(self):
        self.data['jumpers'] = [w for w in self.data['jumpers'] if w['ref'] != 'W11']
        with self.assertRaisesRegex(AssertionError,'Open net'):self.check()

    def test_transistor_pin_swap(self):
        q = self.part('Q1')['pins']
        q['E'],q['C'] = q['C'],q['E']
        with self.assertRaisesRegex(AssertionError,'Short'):self.check()

    def test_same_hole_used_twice(self):
        self.part('C1')['pins']['1'] = self.part('U1')['pins']['8']
        with self.assertRaisesRegex(AssertionError,'Pad collision'):self.check()

    def test_wrong_series_resistance(self):
        self.part('R3')['ohms'] = 270
        with self.assertRaises(AssertionError):self.check()

    def test_component_body_overlap(self):
        self.part('C1')['body'] = self.part('U1')['body'][:]
        with self.assertRaisesRegex(AssertionError,'Body overlap'):self.check()

    def test_jumper_crossing_underside_resistor_body(self):
        w = next(w for w in self.data['jumpers'] if w['ref'] == 'W13')
        w['route'].insert(1,self.part('R1')['body'][:2])
        with self.assertRaisesRegex(AssertionError,'crosses body'):self.check()

    def test_unused_relay_contact_cannot_become_ground(self):
        self.data['jumpers'].append({'ref':'FAULT','pads':['G14','D4'],'net':'GND',
                                    'route':[layout.pad_xy('G14'),layout.pad_xy('D4')]})
        with self.assertRaisesRegex(AssertionError,'Short'):self.check()

    def test_missing_middle_ground_bus_tap(self):
        bus = next(w for w in self.data['jumpers'] if w['ref'] == 'BG')
        bus['pads'].remove('J10')
        with self.assertRaisesRegex(AssertionError,'Open net'):self.check()

    def test_bus_tap_must_be_explicit_on_route(self):
        bus = next(w for w in self.data['jumpers'] if w['ref'] == 'BG')
        tap = layout.pad_xy('I8')
        bus['route'] = [xy for xy in bus['route'] if abs(xy[0]-tap[0])+abs(xy[1]-tap[1]) > 1e-6]
        with self.assertRaisesRegex(AssertionError,'Missing tap'):self.check()

    def test_top_resistor_lead_cannot_cross_capacitor(self):
        self.part('R3')['lead_route'].insert(1,self.part('C3')['body'][:2])
        with self.assertRaisesRegex(AssertionError,'Lead R3 crosses body'):self.check()

    def test_mirrored_image_retains_physical_pad_ids(self):
        source = layout.draw(self.data)
        self.assertIn('board flipped left-to-right',source)
        self.assertIn('J7',source)
        self.check()


if __name__ == '__main__':unittest.main()
