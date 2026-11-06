"""Small parser regression tests for GAYAK's two UART record types.

Run from repository root: python -m unittest discover -s tests -v
PySerial must be installed: pip install -r requirements.txt
"""

import unittest

from serial_reader import SerialReader


class SerialReaderTests(unittest.TestCase):
    def setUp(self):
        self.reader = SerialReader()

    def test_data_and_quake_records_coexist(self):
        self.reader._handle_line(
            "DATA,29.5,65,33.0,CAUTION,410,390.5,20,MONITORING,0,0,1,1"
        )
        self.reader._handle_line("QUAKE,1,1,0.185,STRONG")
        readings = self.reader.get_snapshot()["readings"]
        self.assertEqual(readings["temperature"], 29.5)
        self.assertEqual(readings["mq2_baseline"], 390.5)
        self.assertEqual(readings["quake_online"], True)
        self.assertEqual(readings["quake_alert"], True)
        self.assertAlmostEqual(readings["quake_rms_g"], 0.185)
        self.assertEqual(readings["quake_level"], "STRONG")

        # DATA does not overwrite the independently updated MPU fields.
        self.reader._handle_line(
            "DATA,30.0,66,34.0,CAUTION,420,392.5,28,MONITORING,1,0,1,1"
        )
        readings = self.reader.get_snapshot()["readings"]
        self.assertEqual(readings["temperature"], 30.0)
        self.assertEqual(readings["quake_alert"], True)

    def test_legacy_four_field_quake_record(self):
        self.reader._handle_line("QUAKE,1,0,0.030")
        readings = self.reader.get_snapshot()["readings"]
        self.assertTrue(readings["quake_online"])
        self.assertIsNone(readings["quake_level"])

    def test_intense_acceleration_classification(self):
        self.reader._handle_line("QUAKE,1,1,0.420,INTENSE")
        readings = self.reader.get_snapshot()["readings"]
        self.assertEqual(readings["quake_level"], "INTENSE")
        self.assertTrue(readings["quake_alert"])

    def test_offline_quake(self):
        self.reader._handle_line("QUAKE,0,0,null")
        readings = self.reader.get_snapshot()["readings"]
        self.assertFalse(readings["quake_online"])
        self.assertFalse(readings["quake_alert"])
        self.assertIsNone(readings["quake_rms_g"])
        self.assertIsNone(readings["quake_level"])

    def test_bad_quake_ignored(self):
        self.reader._handle_line("QUAKE,notvalid,1,0.7")
        self.assertIsNone(self.reader.get_snapshot()["readings"]["quake_online"])

    def test_legacy_data_still_supported(self):
        self.reader._handle_line("DATA,28.0,55,29.0,NORMAL,350,335,15,MONITORING,0")
        readings = self.reader.get_snapshot()["readings"]
        self.assertEqual(readings["temperature"], 28.0)
        self.assertFalse(readings["fire_alert"])


if __name__ == "__main__":
    unittest.main()
