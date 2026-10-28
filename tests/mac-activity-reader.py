# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Dean Howell.
import importlib.util,struct,unittest
from pathlib import Path
p=Path(__file__).resolve().parents[1]/'scripts/Read-MacActivity.py'
s=importlib.util.spec_from_file_location('activity_reader',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
def load(name):
    p=Path(__file__).resolve().parents[1]/('scripts/'+name+'.py')
    s=importlib.util.spec_from_file_location(name,p);v=importlib.util.module_from_spec(s);s.loader.exec_module(v);return v
audit=load('Audit-MacActivityCheckpoint');registers=load('Read-MacActivityRegisters')
class Reader(unittest.TestCase):
    def setUp(self):self.v=[0x4D574131,1,80,6,2,0,3,0,1,6,2,1,1,0x80000800,0,0x80100000,0x800,0x800,6,6]
    def test_frozen_binding_trace(self):
        d=m.decode(struct.pack('>20I',*self.v));self.assertEqual(d['endpoints']['first_previous'],{'valid':True,'PB':16,'RR0A':0,'RR0B':0});self.assertEqual(d['nonzero_queued'],2)
    def test_refusals(self):
        for size in (0,4,79,81,100,128):
            with self.subTest(size=size),self.assertRaises(ValueError):m.decode(b'\0'*size)
        for idx,value in [(0,0x4D575031),(0,0x4D575231),(1,2),(2,128),(13,0x01000000),(14,1),(15,0xFF000000),(16,0x1000000),(18,7),(19,7),(4,7),(18,0)]:
            v=self.v.copy();v[idx]=value
            with self.subTest(idx=idx,value=value),self.assertRaises(ValueError):m.decode(struct.pack('>20I',*v))
    def test_zero_and_saturation(self):
        v=[0x4D574131,1,80]+[0]*17;self.assertEqual(m.decode(struct.pack('>20I',*v))['pairs'],0)
        v[3:13]=[0xFFFFFFFF]*10;self.assertEqual(m.decode(struct.pack('>20I',*v))['losses'],0xFFFFFFFF)
    def test_register_subset_and_refusals(self):
        r=dict(zip(['d'+str(i) for i in range(8)]+['a'+str(i) for i in range(7)],
            [0x4D415231,6,2,0,3,0,1,2,1,1,0x80100000,0x800,0x800,6,6]));r['pc']=0x400248
        d=registers.decode(r,0x400248);self.assertEqual(d['first_previous'],0x80100000)
        self.assertNotIn('stable_pairs',d);self.assertNotIn('previous_accepted',d)
        for k,v in [('d0',0x4D575231),('pc',0x400246),('d2',7),('a2',1),('a3',0x1000000),('a5',7),('a6',7),('a1',0),('d1',True)]:
            bad=r.copy();bad[k]=v
            with self.subTest(k=k,v=v),self.assertRaises(ValueError):registers.decode(bad,0x400248)
    def test_checkpoint_auditor(self):
        # Independent literal opcode fixture for the worker's 74-byte leaf.
        code=bytes.fromhex('007c07002c7c00000c00203c4d415231222e000c242e0010262e0014282e00182a2e001c2c2e00202e2e0028206e002c226e0030246e003c266e0040286e00442a6e00482c6e004c60fe')
        self.assertEqual(len(code),74);image=bytearray(65536);image[0x200:0x24a]=code
        syms={'cs_mac_activity_checkpoint':0x400200,'cs_mac_activity_spin':0x400248,'cs_mac_activity_trace':0xc00}
        self.assertEqual(audit.audit(image,syms,True)['bytes'],74)
        for i in range(74):
            bad=image.copy();bad[0x200+i]^=1
            with self.subTest(byte=i),self.assertRaises(ValueError):audit.audit(bad,syms,True)
        for k,v in [('cs_mac_activity_checkpoint',0x400201),('cs_mac_activity_spin',0x400246),('cs_mac_activity_trace',0xc01),('cs_mac_activity_trace',0)]:
            bad=syms.copy();bad[k]=v
            with self.subTest(k=k,v=v),self.assertRaises(ValueError):audit.audit(image,bad,True)
        with self.assertRaises(ValueError):audit.audit(image,syms,False)
        self.assertIsNone(audit.audit(image,{},False))
if __name__=='__main__':unittest.main()
