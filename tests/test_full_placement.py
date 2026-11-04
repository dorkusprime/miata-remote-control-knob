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
        self.data['jumpers'].append({'ref':'FAULT','pads':['H1','H2'], 'net':'+5V',
                                    'route':[layout.pad_xy('H1'),layout.pad_xy('H2')]})
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

    def test_diagonal_jumper_rejected(self):
        w = next(w for w in self.data['jumpers'] if w['ref'] == 'W13')
        w['route'].insert(1,self.part('R1')['body'][:2])
        with self.assertRaisesRegex(AssertionError,'Diagonal'):self.check()

    def test_unused_relay_contact_cannot_become_ground(self):
        self.data['jumpers'].append({'ref':'FAULT','pads':['G14','A4'],'net':'GND',
                                    'route':[layout.pad_xy('G14'),layout.pad_xy('A4')]})
        with self.assertRaisesRegex(AssertionError,'Short'):self.check()

    def test_missing_ground_jumper(self):
        self.data['jumpers'] = [w for w in self.data['jumpers'] if w['ref'] != 'BG5']
        with self.assertRaisesRegex(AssertionError,'Open net'):self.check()

    def test_jumper_route_must_reach_endpoint(self):
        wire = next(w for w in self.data['jumpers'] if w['ref'] == 'BG5')
        wire['route'].pop()
        with self.assertRaisesRegex(AssertionError,'Missing tap'):self.check()

    def test_jumper_cannot_pass_over_occupied_pad(self):
        wire = next(w for w in self.data['jumpers'] if w['ref'] == 'W10')
        wire['route'] = [layout.pad_xy(pad) for pad in wire['pads']]
        with self.assertRaisesRegex(AssertionError,'passes over'):self.check()

    def test_parallel_jumpers_cannot_share_lane(self):
        wire = next(w for w in self.data['jumpers'] if w['ref'] == 'W11')
        for point in wire['route'][1:-1]:
            point[0] = 5.715
        with self.assertRaisesRegex(AssertionError,'Crowded parallel routes'):self.check()

    def test_top_resistor_lead_cannot_cross_capacitor(self):
        self.part('R3')['lead_route'] = [[33.02,15.24],[33.02,21.59],[35.56,21.59],[35.56,45.72],[33.02,45.72]]
        with self.assertRaisesRegex(AssertionError,'Lead R3 crosses body'):self.check()

    def test_r1_cannot_move_back_underneath(self):
        self.part('R1')['side'] = 'bottom'
        with self.assertRaisesRegex(AssertionError,'Component not on top'):self.check()

    def test_diagonal_component_lead_rejected(self):
        part = self.part('C2')
        part['lead_route'] = [layout.pad_xy('I6'), [28.5,20], layout.pad_xy('I10')]
        with self.assertRaisesRegex(AssertionError,'Diagonal'):self.check()

    def test_drawn_transistor_lead_must_reach_its_actual_pad(self):
        self.part('Q1')['lead_routes']['E'][-1] = [5.08,33.02]
        with self.assertRaisesRegex(AssertionError,'Lead endpoint'):self.check()

    def test_camera_input_cannot_keep_old_ground_tie(self):
        self.data['jumpers'].append({'ref':'FAULT','pads':['G8','J7'],'net':'GND',
                                    'route':[layout.pad_xy('G8'),layout.pad_xy('J7')]})
        with self.assertRaisesRegex(AssertionError,'Short'):self.check()

    def test_camera_enable_cannot_keep_old_power_tie(self):
        self.data['jumpers'].append({'ref':'FAULT','pads':['J7','H1'],'net':'+5V',
                                    'route':[layout.pad_xy('J7'),layout.pad_xy('H1')]})
        with self.assertRaisesRegex(AssertionError,'Short'):self.check()

    def test_mirrored_image_retains_physical_pad_ids(self):
        source = layout.draw(self.data)
        self.assertIn('board flipped left-to-right',source)
        self.assertIn('J10',source)
        self.check()


if __name__ == '__main__':unittest.main()
