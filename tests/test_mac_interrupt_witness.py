# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('irq_analysis', Path(__file__).resolve().parents[1] / 'scripts/Analyze-MacInterruptWitness.py')
analysis = importlib.util.module_from_spec(spec)
spec.loader.exec_module(analysis)

TRACE = '''BHMACMOUSE2 0 100 100 7 0 1 1 0 0 0
seq cycle kind address reg value
0 0 cpu_mask 400 8 0
1 10 cpu_mask 200 0 7
2 10 cpu_step 200 0 7
3 14 cpu_step 204 0 7
4 20 cpu_step 300 0 7
5 30 cpu_mask 306 7 0
6 30 cpu_step 400 0 0
'''
FUNCTIONS = [(200, 'veneer'), (300, 'callee'), (400, 'foreground')]


class InterruptAnalysisTests(unittest.TestCase):
    def test_complete_span_accounts_for_every_modeled_cycle(self):
        result = analysis.analyze(TRACE, FUNCTIONS, 200)
        self.assertEqual(result['cycles'], 20)
        self.assertEqual(result['scc_spans'], 1)
        self.assertEqual(result['functions'], [
            {'name': 'veneer', 'cycles': 10, 'execute_operations': 2},
            {'name': 'callee', 'cycles': 10, 'execute_operations': 1}])

    def test_incomplete_coverage_rejected(self):
        for trace in (TRACE.replace('7 0 1 1', '7 2 0 1'),
                      TRACE.replace('7 0 1 1', '7 0 1 0'),
                      TRACE.replace('7 0 1 1', '8 0 1 1'),
                      TRACE.replace('200 0 7\n2', '200 8 7\n2'),
                      TRACE.replace('306 7 0', '306 2 0'),
                      TRACE.replace('7 0 1 1', '6 0 1 1').replace('6 30 cpu_step 400 0 0\n', ''),
                      TRACE.replace('3 14 cpu_step 204 0 7', '3 14 cpu_step 204 0 0')):
            with self.subTest(trace=trace), self.assertRaises(ValueError):
                analysis.analyze(trace, FUNCTIONS, 200)


if __name__ == '__main__':
    unittest.main()
