#!/usr/bin/env python3
"""Check the resistor-interface netlist and generate its schematic, placement, BOM."""
import argparse
import csv
import heapq
import html
import io
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
FOLDER = ROOT / 'hardware/sony-remote'
EXPECTED = {'mute_att': 4400, 'next_track': 8800, 'previous_track': 12100,
            'volume_up': 16800, 'volume_down': 23600}


def check(d):
    assert d['board']['rows'] == 17 and d['board']['columns'] == 'ABCDEFGHIJ'
    assert d['board']['connected_strips'] == ['ABCDE', 'FGHIJ']
    assert len(d['relays']) == 5 and len(d['j1']) == 6 and len(d['j2']) == 2
    gpio_labels = {'D0': 26, 'D1': 27, 'D2': 28, 'D3': 29, 'D6': 0}
    for relay in d['relays']:
        assert relay['part'] == 'AQY212EH'
        assert gpio_labels[relay['xiao']] == relay['gpio']
    parent = {}
    def find(x):
        parent.setdefault(x, x)
        if parent[x] != x: parent[x] = find(parent[x])
        return parent[x]
    def join(a,b): parent[find(a)] = find(b)
    for row in range(1,18):
        for group in ('ABCDE','FGHIJ'):
            for c in group: join(group[0]+str(row), c+str(row))
    used = set()
    def pad(p):
        assert p[0] in 'ABCDEFGHIJ' and 1 <= int(p[1:]) <= 17, p
    for part in d['relays']:
        for p in part['pins'].values():
            pad(p); assert p not in used, f'Double component occupancy {p}'; used.add(p)
    for part in d['resistors']:
        for p in part['pads']:
            pad(p); assert p not in used, f'Double component occupancy {p}'; used.add(p)
    for wire in d['jumpers']:
        a,b = wire['pads']; pad(a); pad(b); join(a,b)
    logic = find(d['j1'][0]['pad']); signal, ground = [find(x['pad']) for x in d['j2']]
    assert logic != ground, 'Ground domains accidentally joined'
    edges = []
    for r in d['resistors']:
        a,b = map(find,r['pads']); assert a!=b, f'{r["ref"]} bypassed by strip/jumper'
        edges.append((a,b,r['ohms']))
    def resistance(a,b,extra=()):
        graph = {}
        for x,y,r in [*edges,*extra]:
            graph.setdefault(x,[]).append((y,r)); graph.setdefault(y,[]).append((x,r))
        queue = [(0,a)]; visited=set()
        while queue:
            distance,node=heapq.heappop(queue)
            if node==b:return distance
            if node in visited:continue
            visited.add(node)
            for target,r in graph.get(node,[]):heapq.heappush(queue,(distance+r,target))
        return math.inf
    assert math.isinf(resistance(signal, ground)), 'Output must be open with all switches off'
    assert math.isinf(resistance(logic,ground)), 'Resistors join logic/Sony grounds'
    for r, connector in zip(d['relays'],d['j1'][1:]):
        assert connector['gpio']==r['gpio'] and r['gpio'] not in (6,7)
        pins={n:find(p) for n,p in r['pins'].items()}
        assert pins['2']==logic and pins['3']==ground
        assert resistance(find(connector['pad']),pins['1'])==220
        assert resistance(find(connector['pad']),logic)==47000
        value=resistance(signal,ground,[(pins['3'],pins['4'],0)])
        assert value==EXPECTED[r['action']], (r['ref'],value)
        assert math.isinf(resistance(logic,ground,[(pins['3'],pins['4'],0)]))
    assert len({r['gpio'] for r in d['relays']})==5
    print('Netlist checks passed: five exact nominal taps, open release, separate grounds, control resistors, pad occupancy.')


class Svg:
    def __init__(self,w,h,title):
        self.parts=[f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}">',
                    f'<title>{html.escape(title)}</title>',
                    '<style>text{font-family:Arial,sans-serif;fill:#172033} .small{font-size:12px} .wire{fill:none;stroke-width:2.4;stroke-linejoin:round}</style>',
                    f'<rect width="{w}" height="{h}" fill="white"/>']
    def text(self,x,y,t,size=16,color=None,anchor='start'):
        style=f'font-size:{size}px'+(f';fill:{color}' if color else '')
        self.parts.append(f'<text x="{x}" y="{y}" text-anchor="{anchor}" style="{style}">{html.escape(t)}</text>')
    def line(self,x1,y1,x2,y2,color='#26354a',dash=False):
        self.parts.append(f'<path class="wire" stroke="{color}"'+(' stroke-dasharray="6 5"' if dash else '')+f' d="M{x1},{y1} L{x2},{y2}"/>')
    def rect(self,x,y,w,h,fill='#fff',stroke='#26354a',radius=0):
        self.parts.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{radius}" fill="{fill}" stroke="{stroke}"/>')
    def dot(self,x,y,color='#26354a',r=4):self.parts.append(f'<circle cx="{x}" cy="{y}" r="{r}" fill="{color}"/>')
    def resistor(self,x1,y1,x2,y2,label,color='#26354a',show=True):
        dx,dy=x2-x1,y2-y1; length=math.hypot(dx,dy); ux,uy=dx/length,dy/length
        mx,my=(x1+x2)/2,(y1+y2)/2; half=min(15,length*.3)
        self.line(x1,y1,mx-ux*half,my-uy*half,color);self.line(mx+ux*half,my+uy*half,x2,y2,color)
        angle=math.degrees(math.atan2(dy,dx))
        self.parts.append(f'<rect x="{-half}" y="-5" width="{2*half}" height="10" fill="white" stroke="{color}" stroke-width="2" transform="translate({mx},{my}) rotate({angle})"/>')
        if show:self.text(mx+9,my-8,label,12,color)
    def finish(self): return '\n'.join(self.parts+['</svg>'])+'\n'


def schematic(d):
    s=Svg(1120,1010,'Sony steering-remote prototype: isolated five-tap resistor ladder')
    s.text(35,40,'Sony steering-remote prototype',26)
    s.text(35,65,'DESIGN ONLY - bench with Sony disconnected; no hardware measurements yet',14,'#a33')
    s.text(35,110,'Each of five control inputs',19)
    s.text(35,135,'Repeat for U1 through U5; GPIO HIGH closes that switch.',13)
    s.text(40,210,'GPIO',14);s.line(90,205,145,205,'#087f8c')
    s.resistor(145,205,280,205,'220 ohm','#087f8c')
    s.dot(115,205,'#087f8c');s.resistor(115,205,115,330,'47k pulldown','#087f8c')
    s.line(280,205,315,205,'#087f8c');s.rect(315,175,130,135,'#eef6f8','#087f8c',5)
    s.text(330,200,'1  LED anode',12);s.text(330,280,'2  cathode',12)
    s.line(315,285,280,285,'#087f8c');s.line(280,285,280,330,'#087f8c');s.line(115,330,280,330,'#087f8c')
    s.text(120,355,'XIAO GND only',14,'#087f8c')
    s.text(35,410,'Logic drive',18)
    for i,t in enumerate(['3.3V GPIO; configure 12mA drive for this future backend.',
                          '220 ohm gives about 9.8mA LED current at nominal voltage.',
                          'One command active at a time; LOW / reset = open.',
                          'PhotoMOS input is LED-driven; no VCC rail or capacitor.',
                          'Manual mock-up: GPIO cable unplugged; jumper 3V3',
                          'to one J1 command input, with J1-1 tied to XIAO GND.']):s.text(35,440+i*23,t,13)
    s.text(35,625,'Cable J1 - controller side',18)
    for i,c in enumerate(d['j1']):s.text(40,655+i*25,f'{c["pin"]}: {c["xiao"]}  /  {c["signal"]}',13)
    s.text(35,840,'Keep input/output wiring separate on the board.',14)
    s.text(35,864,'Do not join XIAO GND to Sony remote ground here.',14)
    s.text(35,888,'Vehicle supplies may share ground elsewhere.',13)
    s.line(515,105,515,925,'#bac3d0',True)
    s.text(570,110,'Sony side - passive resistance output',19)
    x=625; top=170; s.text(570,145,'J2-1: Input 1',14,'#b45309');s.dot(x,top,'#b45309')
    positions=[170,230,290,350,410,500,590,680]
    values=[2200,2200,2200,2200,3300,4700,6800]
    for i,value in enumerate(values):s.resistor(x,positions[i],x,positions[i+1],f'R{i+1}: {value/1000:g}k','#b45309')
    taps=[290,410,500,590,680]
    labels=[('U1','Mute / ATT','4.40k'),('U2','Next track','8.80k'),('U3','Previous track','12.10k'),('U4','Volume up','16.80k'),('U5','Volume down','23.60k')]
    groundx=1010
    for y,(ref,action,resistance) in zip(taps,labels):
        s.dot(x,y,'#b45309');s.line(x,y,790,y,'#b45309')
        s.rect(790,y-20,100,40,'#fff8ed','#b45309',4)
        s.text(840,y+5,ref+'  N.O.',13,'#b45309','middle')
        s.text(790,y-30,f'{action}: {resistance}',13)
        s.text(775,y+5,'4',11);s.text(900,y+5,'3',11)
        s.line(890,y,groundx,y,'#b45309');s.dot(groundx,y,'#b45309')
    s.line(groundx,taps[0],groundx,760,'#b45309');s.text(785,790,'J2-2: ST Remote GND',15,'#b45309')
    s.text(570,835,'All U1-U5: AQY212EH, normally-open PhotoMOS (DIP-4).',13)
    s.text(570,860,'Pins 1/2 are the isolated LED input; 3/4 are the output contact.',13)
    s.text(570,885,'Input 2 stays insulated and unused. No vehicle 12V connection.',13)
    s.text(35,960,'Seven 1% ladder resistors produce the five reference values. Actual readings also include contact resistance and leakage.',13)
    s.text(35,985,'This prototype covers steering-remote audio commands; camera-trigger circuitry is a separate design.',13)
    return s.finish()


def placement(d):
    s=Svg(1070,800,'ElectroCookie Mini resistor-interface placement: top view')
    s.text(25,30,'ElectroCookie Mini - TOP VIEW',23)
    s.text(25,53,'38.1 x 50.8mm; no board cuts. Verify strip continuity on your actual board first.',13,'#a33')
    def xy(p):
        c='ABCDEFGHIJ'.index(p[0]); row=int(p[1:])
        return (70+c*30.48+(60.96 if c>=5 else 0),126+(row-1)*30.48)
    s.rect(9,65,457,610,'#f3f7fb','#637187',20)
    for row in range(1,18):
        for group in ('ABCDE','FGHIJ'):
            a,b=xy(group[0]+str(row)),xy(group[-1]+str(row));s.line(*a,*b,'#c4d1df')
        for c in 'ABCDEFGHIJ':
            x,y=xy(c+str(row));s.dot(x,y,'#a5b3c3',4)
        s.text(32,xy('A'+str(row))[1]+4,str(row),11)
    for c in 'ABCDEFGHIJ':s.text(xy(c+'1')[0],110,c,11,anchor='middle')
    for row in (1,17):s.dot(237.64,xy('A'+str(row))[1],'white',19)
    for j in d['jumpers']:
        color='#087f8c' if j['pads'][0][0] in 'ABCDE' else '#b45309'
        s.line(*xy(j['pads'][0]),*xy(j['pads'][1]),color)
    for r in d['resistors']:
        color='#237d35' if r['role']!='ladder' else '#b45309'
        s.resistor(*xy(r['pads'][0]),*xy(r['pads'][1]),r['ref'],color,False)
    for u in d['relays']:
        x,y=xy(u['pins']['1']); xf,y2=xy(u['pins']['3'])
        s.rect(x-7,y-13,xf-x+14,y2-y+26,'#e8e9ef','#38405a',3)
        s.text((x+xf)/2,y+18,u['ref'],13,anchor='middle');s.dot(x+8,y-5,'#111',3)
        for n,p in u['pins'].items():s.text(xy(p)[0]+(3 if n in ('1','2') else -10),xy(p)[1]+4,n,10)
    for c in d['j1']:s.dot(*xy(c['pad']),'#087f8c',6)
    for c in d['j2']:s.dot(*xy(c['pad']),'#b45309',6)
    s.text(25,715,'Teal: controller ground / J1. Orange: Sony-side wiring / J2.',13)
    s.text(25,738,'Green: input resistors. Gray rows show factory-connected groups of five pads.',13)
    s.text(25,761,'Use insulated jumpers; crossings are not connections. Some resistors stand upright.',13)
    s.text(510,95,'Relay placement / pin orientation',18)
    s.text(510,120,'Pin 1 dot at upper left. Each package spans the center gap.',13)
    for i,u in enumerate(d['relays']):
        s.text(510,150+i*25,f'{u["ref"]}: {u["action"]}  |  '+', '.join(f'{n}={p}' for n,p in u['pins'].items()),12)
    s.text(510,310,'Resistors: ref / value / pad endpoints',18)
    for i,r in enumerate(d['resistors']):
        val=f'{r["ohms"]/1000:g}k' if r['ohms']>=1000 else str(r['ohms'])
        s.text(510,337+i*18,f'{r["ref"]:4s}  {val:5s}   {r["pads"][0]} - {r["pads"][1]}',12)
    s.text(510,682,'J1 pigtail: 1=A17, 2=A1, 3=A4, 4=A7, 5=A10, 6=A13',12)
    s.text(510,707,'J2 vertical 2-pin header: 1=J16 (Input 1), 2=J17 (Sony GND)',12)
    s.text(510,732,'Use corner mounts; leave central M3 holes unused near relays.',12)
    s.text(510,757,'Dry-fit actual lead lengths / insulation / enclosure before soldering.',12)
    return s.finish()


def bom():
    rows=[['Item','Quantity','Specification','Notes'],
          ['PhotoMOS switch',5,'Panasonic AQY212EH DIP-4 through hole, normally open','Do not substitute EHA surface mount or normally closed parts'],
          ['Ladder resistor',4,'2.2k ohm, 1%, axial 0.25W','R1-R4'],
          ['Ladder resistor',1,'3.3k ohm, 1%, axial 0.25W','R5'],
          ['Ladder resistor',1,'4.7k ohm, 1%, axial 0.25W','R6'],
          ['Ladder resistor',1,'6.8k ohm, 1%, axial 0.25W','R7'],
          ['LED series resistor',5,'220 ohm, 1%, axial 0.25W','RL1-RL5; stand upright where needed'],
          ['Control pulldown',5,'47k ohm, 1%, axial 0.25W','RP1-RP5'],
          ['Protoboard',1,'ElectroCookie Mini 38.1 x 50.8mm','Existing choice; connected-row layout, no trimming'],
          ['Controller connector',1,'6-contact removable pigtail, 2.54mm or existing equivalent','Five GPIO leads plus logic ground; optional 8-way reserves camera/spare'],
          ['Sony connector',1,'2-contact removable connector, 2.54mm','Input 1 and ST Remote GND; pitch matches J16/J17'],
          ['Hookup wire','as needed','Insulated 26-28AWG suitable for through-hole wiring','Separate logic and Sony-side bundles; strain relief'],
          ['Enclosure / mounting','as needed','Insulating support using board corner mounts','No support by signal wires; final fit untested']]
    out=io.StringIO();csv.writer(out,lineterminator='\n').writerows(rows);return out.getvalue()


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    d=json.loads((FOLDER/'prototype.json').read_text());check(d)
    outputs={'schematic.svg':schematic(d),'placement.svg':placement(d),'bom.csv':bom()}
    for name,text in outputs.items():
        if name.endswith('.svg'):ET.fromstring(text)
        path=FOLDER/name
        if args.check:assert path.read_text()==text, f'Regenerate {name}: python3 scripts/sony-interface.py'
        else:path.write_text(text)
    print('Generated artifacts are current.' if args.check else 'Wrote schematic.svg, placement.svg, bom.csv.')


if __name__=='__main__':main()
