import contextlib
import importlib.util
import io
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]
SPEC = importlib.util.spec_from_file_location('sony_interface', ROOT / 'scripts/sony-interface.py')
interface = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(interface)


class ArchivedPhotoMosInterfaceTests(unittest.TestCase):
    def setUp(self):
        self.netlist = json.loads((ROOT / 'hardware/archive/sony-remote-photomos/prototype.json').read_text())

    def checked(self):
        with contextlib.redirect_stdout(io.StringIO()):
            interface.check(self.netlist)

    def test_nominal_layout(self):
        self.checked()

    def test_ground_domains_must_stay_separate(self):
        self.netlist['jumpers'].append({'pads': ['A17', 'J17']})
        with self.assertRaises(AssertionError): self.checked()

    def test_shortened_ladder_is_detected(self):
        self.netlist['jumpers'].append({'pads': ['H1', 'H2']})
        with self.assertRaises(AssertionError): self.checked()

    def test_missing_sony_ground_link_is_detected(self):
        self.netlist['jumpers'] = [j for j in self.netlist['jumpers'] if j['pads'] != ['I15', 'H17']]
        with self.assertRaises(AssertionError): self.checked()

    def test_wrong_ladder_value_is_detected(self):
        next(r for r in self.netlist['resistors'] if r['ref'] == 'R5')['ohms'] = 3900
        with self.assertRaises(AssertionError): self.checked()

    def test_i2c_gpio_cannot_be_reused(self):
        self.netlist['relays'][0]['gpio'] = self.netlist['j1'][1]['gpio'] = 6
        with self.assertRaises(AssertionError): self.checked()

    def test_wrong_led_resistor_is_detected(self):
        self.netlist['resistors'][0]['ohms'] = 330
        with self.assertRaises(AssertionError): self.checked()

    def test_duplicate_component_pad_is_detected(self):
        self.netlist['resistors'][1]['pads'][0] = 'B1'
        with self.assertRaises(AssertionError): self.checked()


if __name__ == '__main__': unittest.main()
