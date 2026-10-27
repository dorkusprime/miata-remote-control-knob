#!/usr/bin/env python3
"""Check and draw the complete proposed MCP41050 interface pad allocation."""
import argparse
import html
import json
import math
from pathlib import Path
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
FOLDER = ROOT / 'hardware/sony-remote'
# Device-terminal contract from docs/sony-interface.md, independent of pad allocation.
PINS = {
 'U1':dict(zip(map(str,range(1,9)),['CS_OUT','SCK_OUT','MOSI_OUT','GND','GND','PW','NC_U1_7','+5V'])),
 'U2':dict(zip(map(str,range(1,15)),['GND','CS_IN','CS_OUT','GND','SCK_IN','SCK_OUT','GND','MOSI_OUT','MOSI_IN','GND','NC_U2_11','GND','+5V','+5V'])),
 'K1':dict(zip(map(str,range(1,9)),['NC_K1_1','LED_A','LED_K','NC_K1_4','SONY_R','PW','NC_K1_7','NC_K1_8'])),
 'Q1':{'E':'GND','B':'BASE','C':'LED_K'},
 'R1':{'1':'LED_A','2':'+5V'},'R2':{'1':'RELAY_EN','2':'BASE'},'R3':{'1':'SONY_IN','2':'SONY_R'},
 'R4':{'1':'CS_IN','2':'GND'},'R5':{'1':'SCK_IN','2':'GND'},'R6':{'1':'MOSI_IN','2':'GND'},'R7':{'1':'BASE','2':'GND'},
 'C1':{'1':'+5V','2':'GND'},'C2':{'1':'+5V','2':'GND'},'C3':{'+':'+5V','-':'GND'},
}
CONTACTS={'J1.1':'GND','J1.2':'+5V','J1.3':'SCK_IN','J1.4':'MOSI_IN','J1.5':'CS_IN','J1.6':'RELAY_EN','J2.1':'SONY_IN','J2.2':'GND'}
RESISTORS={'R1':330,'R2':1000,'R3':2700,'R4':47000,'R5':47000,'R6':47000,'R7':47000}
COLORS={'GND':'#64748b','+5V':'#dc2626','CS_OUT':'#059669','SCK_OUT':'#2563eb','MOSI_OUT':'#9333ea','PW':'#d97706'}


def pad_xy(p):
    assert p[0] in 'ABCDEFGHIJ' and 1 <= int(p[1:]) <= 17, f'Invalid pad {p}'
    return (5.08+('ABCDEFGHIJ'.index(p[0])+(2 if p[0]>='F' else 0))*2.54,
            5.08+(int(p[1:])-1)*2.54)


def segment_hits_box(a,b,box,margin=0.2):
    x,y,w,h=box; lo=[x-w/2-margin,y-h/2-margin]; hi=[x+w/2+margin,y+h/2+margin]
    start,end=0.,1.
    for axis in (0,1):
        delta=b[axis]-a[axis]
        if abs(delta)<1e-9:
            if not lo[axis] <= a[axis] <= hi[axis]: return False
        else:
            t0,t1=sorted(((lo[axis]-a[axis])/delta,(hi[axis]-a[axis])/delta))
            start=max(start,t0);end=min(end,t1)
            if start>end:return False
    return True


def orthogonal(points, owner):
    assert len(points)>=2, f'Empty route: {owner}'
    for a,b in zip(points,points[1:]):
        assert abs(a[0]-b[0])<1e-6 or abs(a[1]-b[1])<1e-6, f'Diagonal route: {owner}'


def component_routes(p):
    if p.get('style')=='dip' or p['ref'] in ('U1','U2','K1'):return {}
    if p.get('style')=='transistor':return p['lead_routes']
    pins=list(p['pins'].values())
    return {'leads':p.get('lead_route',[pad_xy(pins[0]),pad_xy(pins[1])])}


def check(d):
    assert d['board']['rows']==17 and d['board']['strip_groups']==['ABCDE','FGHIJ']
    assert len(d['parts'])==len(PINS) and {p['ref'] for p in d['parts']}==set(PINS)
    parent={}
    def find(p):
        parent.setdefault(p,p)
        if parent[p]!=p:parent[p]=find(parent[p])
        return parent[p]
    def join(a,b):parent[find(a)]=find(b)
    for row in range(1,18):
        for group in ('ABCDE','FGHIJ'):
            for c in group:join(group[0]+str(row),c+str(row))
    used={}
    def occupy(p,owner):
        pad_xy(p)
        assert p not in used, f'Pad collision {p}: {owner}, {used.get(p)}'
        used[p]=owner
    for part in d['parts']:
        ref=part['ref'];assert set(part['pins'])==set(PINS[ref]),ref
        for pin,pad in part['pins'].items():occupy(pad,f'{ref}.{pin}')
        if ref in RESISTORS:assert part['ohms']==RESISTORS[ref],ref
    assert len(d['contacts'])==len(CONTACTS) and {c['ref'] for c in d['contacts']}==set(CONTACTS)
    for c in d['contacts']:
        assert c['net']==CONTACTS[c['ref']]
        occupy(c['pad'],c['ref'])
    assert len({w['ref'] for w in d['jumpers']})==len(d['jumpers'])
    for w in d['jumpers']:
        for pad in w['pads']:occupy(pad,w['ref'])
        assert len(w['pads'])>=2, w['ref']
        assert all(any(math.dist(pad_xy(p),point)<1e-6 for point in w['route']) for p in w['pads']), f'Missing tap on route: {w["ref"]}'
        assert math.dist(pad_xy(w['pads'][0]),w['route'][0])<1e-6 and math.dist(pad_xy(w['pads'][-1]),w['route'][-1])<1e-6, w['ref']
        for p in w['pads'][1:]:join(w['pads'][0],p)
    root_names={}; net_roots={}
    def label(pad,net):
        root=find(pad)
        assert root not in root_names or root_names[root]==net, f'Short: {net} / {root_names.get(root)} at {pad}'
        root_names[root]=net;net_roots.setdefault(net,set()).add(root)
    for p in d['parts']:
        for pin,pad in p['pins'].items():label(pad,PINS[p['ref']][pin])
    for c in d['contacts']:label(c['pad'],c['net'])
    for w in d['jumpers']:
        for pad in w['pads']:label(pad,w['net'])
    for net,roots in net_roots.items():assert len(roots)==1, f'Open net: {net}'
    for w in d['jumpers']:orthogonal(w['route'],w['ref'])
    for p in d['parts']:
        assert p['side']=='top', f'Component not on top: {p["ref"]}'
        for pin,route in component_routes(p).items():
            orthogonal(route,p['ref']+'.'+pin)
            assert all(0<=xy[0]<=38.1 and 0<=xy[1]<=50.8 for xy in route), f'Off-board lead: {p["ref"]}'
            if p.get('style')=='transistor':
                assert math.dist(route[-1],pad_xy(p['pins'][pin]))<1e-6, f'Lead endpoint: {p["ref"]}.{pin}'
            else:
                endpoints=list(p['pins'].values())
                assert math.dist(route[0],pad_xy(endpoints[0]))<1e-6 and math.dist(route[-1],pad_xy(endpoints[1]))<1e-6, f'Lead endpoint: {p["ref"]}'
    # Nominal body rectangles are a layout check, not certified manufacturer/socket clearance.
    for i,p in enumerate(d['parts']):
        x,y,w,h=p['body'];assert x-w/2>=0 and x+w/2<=38.1 and y-h/2>=0 and y+h/2<=50.8,p['ref']
        for q in d['parts'][i+1:]:
            if p['side']!=q['side']:continue
            xx,yy,ww,hh=q['body']
            assert abs(x-xx)>=(w+ww)/2 or abs(y-yy)>=(h+hh)/2, f'Body overlap: {p["ref"]}/{q["ref"]}'
    for w in d['jumpers']:
        for p in d['parts']:
            if p['side']=='bottom':
                assert not any(segment_hits_box(a,b,p['body']) for a,b in zip(w['route'],w['route'][1:])),f'Jumper {w["ref"]} crosses body {p["ref"]}'
    for p in d['parts']:
        for route in component_routes(p).values():
            for q in d['parts']:
                if q['ref']==p['ref'] or q['side']!=p['side']:continue
                assert not any(segment_hits_box(a,b,q['body']) for a,b in zip(route,route[1:])), f'Lead {p["ref"]} crosses body {q["ref"]}'
    print('Current layout: connected nets; no shorts or pad reuse; all bodies on top; all routes orthogonal; nominal bodies and other-component lead clearances checked.')


class Svg:
    def __init__(self):
        self.s=['<svg xmlns="http://www.w3.org/2000/svg" width="1760" height="1300" viewBox="0 0 1760 1300">','<title>Complete proposed Sony interface placement: separate top and mirrored underside</title>','<rect width="1760" height="1300" fill="#f8fafc"/>','<style>text{font-family:Arial,sans-serif;fill:#172033}</style>']
    def text(self,x,y,t,size=16,color='#172033',anchor='start'):
        self.s.append(f'<text x="{x}" y="{y}" font-size="{size}" text-anchor="{anchor}" style="fill:{color}">{html.escape(str(t))}</text>')
    def rect(self,x,y,w,h,fill,stroke='none',radius=0,dash=False):
        self.s.append(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{radius}" fill="{fill}" stroke="{stroke}"'+(' stroke-dasharray="6 4"' if dash else '')+'/>')
    def circle(self,x,y,r,fill,stroke='none'):
        self.s.append(f'<circle cx="{x}" cy="{y}" r="{r}" fill="{fill}" stroke="{stroke}"/>')
    def path(self,points,color,width=2,dash=False):
        orthogonal(points,'SVG path')
        path='M'+' L'.join(f'{x:.2f},{y:.2f}' for x,y in points)
        self.s.append(f'<path d="{path}" fill="none" stroke="{color}" stroke-width="{width}" stroke-linejoin="round"'+(' stroke-dasharray="5 4"' if dash else '')+'/>')
    def finish(self):
        data='\n'.join(self.s+['</svg>'])+'\n';ET.fromstring(data);return data


def draw(d):
    s=Svg();scale=15
    s.text(35,43,'Sony remote interface · ALL COMPONENTS',29)
    s.text(35,74,'MCP41050 + HCT125 + LCC110 · proposed no-cut ElectroCookie Mini layout',18)
    s.rect(35,94,1690,66,'#fff3cd',radius=8)
    s.text(52,120,'ELECTRICAL PAD ALLOCATION CHECKED · PHYSICAL FIT / SOCKETS / MOUNTING STILL UNVERIFIED',18,'#92400e')
    s.text(52,145,'All component bodies on TOP. Six underside wire pieces. All lead and wire routes use horizontal / vertical segments.',16,'#92400e')
    for side,bx in [('top',55),('bottom',705)]:
        by=220
        def pos(pt):return (bx+(38.1-pt[0] if side=='bottom' else pt[0])*scale,by+pt[1]*scale)
        def pad(p):return pos(pad_xy(p))
        s.text(bx,191,'TOP · component view' if side=='top' else 'UNDERSIDE · board flipped left-to-right',21)
        s.rect(bx,by,38.1*scale,50.8*scale,'#e6edf3','#94a3b8',14)
        for row in range(1,18):
            for group in ('ABCDE','FGHIJ'):s.path([pad(group[0]+str(row)),pad(group[-1]+str(row))],'#c5d0d9',4)
            for col in 'ABCDEFGHIJ':
                x,y=pad(col+str(row));s.circle(x,y,4,'#fff','#8a9aaa')
            s.text(bx+8,pad('A'+str(row))[1]+4,row,11)
        for col in 'ABCDEFGHIJ':s.text(pad(col+'1')[0],by+28,col,12,anchor='middle')
        if side=='bottom':
            for w in d['jumpers']:
                points=[pos(pt) for pt in w['route']];color=COLORS[w['net']]
                # White edging keeps insulated crossing wires visually separate.
                s.path(points,'#f8fafc',5);s.path(points,color,2.5)
                for p in w['pads']:
                    x,y=pad(p);s.circle(x,y,4,color)
                x,y=points[0];s.text(x+5,y-7,w['ref'],10,color)
        for p in d['parts']:
            if p['side']!=side:continue
            ref=p['ref'];style=p.get('style','dip');cx,cy,bw,bh=p['body'];x,y=pos((cx,cy))
            if style=='dip':
                color={'U1':'#0f766e','U2':'#1d4ed8','K1':'#7c3aed'}[ref]
                s.rect(x-bw*scale/2,y-bh*scale/2,bw*scale,bh*scale,color,radius=5)
                s.text(x,y,ref,19,'#fff','middle');s.circle(*pos((cx-2,cy-bh/2+1)),3,'#fff')
                for n,location in p['pins'].items():
                    xx,yy=pad(location);edge=x-bw*scale/2 if location[0]=='E' else x+bw*scale/2
                    s.path([(xx,yy),(edge,yy)],color)
                    s.text(xx+(-12 if location[0]=='E' else 12),yy+4,n,10,color,'middle')
                continue
            if style=='transistor':
                routes=p['lead_routes']
                for name,points in routes.items():
                    s.path([pos(pt) for pt in points],'#374151',2)
                    xx,yy=pad(p['pins'][name]);s.text(xx-5,yy-7,name,10)
                s.rect(x-2.5*scale,y-2.5*scale,5*scale,5*scale,'#334155',radius=12)
                s.text(x,y+5,ref,15,'#fff','middle');continue
            pins=list(p['pins'].values())
            route=p.get('lead_route',[pad_xy(pins[0]),pad_xy(pins[1])])
            s.path([pos(pt) for pt in route],'#926329',2.5)
            if style=='axial':
                # Filled body lies on the stated lead route, not over a second component.
                angle=p['angle']*(-1 if side=='bottom' else 1)
                s.s.append(f'<rect x="{-3.4*scale}" y="{-1.3*scale}" width="{6.8*scale}" height="{2.6*scale}" rx="5" fill="#f1cf85" stroke="#815a21" transform="translate({x},{y}) rotate({angle})"/>')
                s.text(x+(35 if ref=='R1' else -25),y-12,ref,17,'#815a21')
            elif style=='upright':
                s.circle(x,y,1.3*scale,'#f1cf85','#815a21');s.text(x-26,y+6,ref,15,'#815a21')
            elif style=='ceramic':
                s.rect(x-bw*scale/2,y-bh*scale/2,bw*scale,bh*scale,'#c77c29',radius=9)
                s.text(x-50,y+5,ref,15,'#8b4f13')
            elif style=='electrolytic':
                s.circle(x,y,2.5*scale,'#1e293b');s.text(x,y+5,ref,15,'#fff','middle')
                xx,yy=pad(p['pins']['+']);s.text(xx+12,yy+8,'+',18,'#dc2626')
        # Termination markers: distributed pigtails, not contiguous headers.
        if side=='top':
            for c in d['contacts']:
                x,y=pad(c['pad']);s.circle(x,y,5,'#0e7490');s.text(x-4,y-10,c['ref'].replace('J',''),10,'#0e7490')
        s.text(bx,1010,'Bodies shown on this face only. Pads retain the same IDs after flipping.',14)
    x=1325;s.text(x,195,'TOP',20)
    for i,t in enumerate(['U1   MCP41050-I/P','U2   SN74HCT125N','K1   LCC110','Q1   2N3904BU, formed leads','R1 + R4–R7   upright resistors','C1/C2   100 nF','C3   10 µF, + J7 / − J8','R2/R3   outer-edge axial resistors']):s.text(x,226+i*27,t,16)
    s.text(x,450,'UNDERSIDE',20)
    for i,t in enumerate(['No component bodies underneath.','BG   one wire, 7 ground taps','BV   one wire, 4 power taps','W10–W13   four signal wires','Six pieces; 19 solder landings.','Sleeve buses between marked taps.']):s.text(x,481+i*27,t,16)
    s.text(x,690,'J1 · six-wire controller pigtail',18)
    for i,t in enumerate(['1 GND → B4','2 +5 V → J6','3 SCK → A10','4 MOSI → I11','5 CS → A7','6 ENABLE → B5']):s.text(x,720+i*25,t,16)
    s.text(x,910,'J2 · Sony pigtail',18)
    s.text(x,939,'1 Input 1 → I5',16);s.text(x,966,'2 ST Remote GND → H4',16)
    s.rect(35,1040,1690,216,'#e2e8f0',radius=10)
    notes=[
      'Gray bars are factory-connected strips: A–E and F–J for each row. No copper cuts. No shared component/jumper holes.',
      'R1/R4–R7 stand upright; Q1 leads land at B12 (E), C13 (B), D16 (C). Insulate formed leads and check actual transistor marking.',
      'R1 returns to F13 through the gap between U2 and K1. BV now lands at H13. Check actual socket clearance.',
      'C1/C2 leads need careful forming. C3 positive goes to J7 and negative to J8. Do not reverse electrolytic polarity.',
      'BG/BV tack to the marked underside pads; they do not thread through every hole. Sleeve all sections between taps.',
      'Nominal body envelopes do not certify sockets, mounting holes or final height. Dry-fit the actual board before soldering.',
      'See pad-map.md and layout.json for every pin, terminal and jumper. Electrical mapping is checked; hardware remains untested.',
    ]
    for i,t in enumerate(notes):s.text(52,1070+i*26,t,16)
    return s.finish()


def tables(d):
    s=['# Complete proposed pad allocation','', '**Electrically checked design, not a physically verified build.** Use the [circuit guide](../../docs/sony-interface.md) for device limits and acceptance. [All-component image](placement-full.svg) separates top and horizontally mirrored underside views. No firmware outputs were enabled.','', '## Component terminals','', '| Ref | Part / side | Terminal → pad |','| --- | --- | --- |']
    for p in d['parts']:s.append(f'| {p["ref"]} | {p["part"]} / {p["side"]} | '+', '.join(f'{n} → {pad}' for n,pad in p['pins'].items())+' |')
    s+=['','## Distributed pigtails','', 'These are wire landing pads, not two in-line headers. Fit the existing removable connectors off-board. J1 numbering matches the current harness. Sony Input 2 stays insulated.','', '| Contact | Net | Pad |','| --- | --- | --- |']
    for c in d['contacts']:s.append(f'| {c["ref"]} | {c["net"]} | {c["pad"]} |')
    s+=['','## Underside wire pieces and bus taps','', 'Crossings are not junctions. Use insulated wire; route clear of resistor bodies and keep leads sleeved over unrelated pads. Coordinates are endpoint and routing references, not validated three-dimensional wire bends.','', '| Wire | Net | Solder pads in order |','| --- | --- | --- |']
    for w in d['jumpers']:s.append(f'| {w["ref"]} | {w["net"]} | '+ ' → '.join(w['pads'])+' |')
    s+=['', 'BG and BV are each one continuous conductor with multiple solder taps, not chains of separate jumpers. Expose only the listed tap pads; sleeve all intervening sections. Untapped pads along the drawn route stay insulated. Tack the middle taps to the underside pads rather than threading a wire repeatedly through holes. This reduces wire pieces from 13 to 6 and wire solder landings from 26 to 19; it does not remove electrical nets. Four signal wires remain separate.', '', 'If making continuous tapped buses is awkward, separate insulated segments following the same net connections remain electrically equivalent; they restore some of the original cut/strip work.', '', '## Assembly and physical conditions','',
        '- Fit sockets and top components first, without solder. The IC-only image is now a subset of this candidate.',
        '- R1 and R4–R7 stand upright. R1 is at C15 and its sleeved return lead follows the gap between U2 and K1 to F13. The power bus now ends at H13. This is a formed-lead proposal, not a validated socket clearance or guaranteed supplied lead length.',
        '- Q1 has a formed-lead footprint, not the default three adjacent TO-92 holes. E=B12, B=C13, C=D16. Q1 moves to the lower-left corner; R7 moves to C12/D13. Check the actual part orientation and preform without force at the body.',
        '- All component bodies are now on top. R2 follows the top left edge; R3 follows the top right edge with a jog to x=37 mm past C3. Sleeve extended leads and dry-fit before trimming.',
        '- Check supplied lead lengths before trimming. If a lead cannot follow its proposed route safely, revise the model or use an insulated extension; do not force the component.',
        '- C1/C2 leads are spread to the listed pads, rather than matching the stock 2.5 mm lead pitch. Avoid stress at the ceramic coating. C3 is polarized.',
        '- Body envelopes are provisional. The resistor envelope allows 6.8 × 2.6 mm, based on Yageo MFR-25 maximum body dimensions. Socket outlines, capacitor lead availability, mounting-hole locations and assembled height still require actual measurements.',
        '- All component bodies are on top; only the six wire pieces and solder joints are underneath. Distributed pigtails preserve the no-cut board. Mounted height and center mounting-hole access remain unverified.',
        '- On the actual board, verify A–E / F–J strip continuity and orientation first. Do not solder until the dry-fit and receiver-disconnected electrical checks pass.',
        '', '## Verification scope','',
        '`python3 scripts/full-placement.py --check` verifies the 14 component references, terminal connectivity against an independent device-pin contract, all resistor values, factory strips, no accidental net joins, no reused holes, pigtail assignments, nominal body separation, all-top component placement, orthogonal routes, and all component-lead routes avoiding other component bodies. It also verifies that the SVG/table match the model. This is not a timing, analog-current, power-sequencing or physical-fit simulation.',
        '', 'Regenerate with `python3 scripts/full-placement.py`. Tests deliberately introduce a rail short, an open jumper, a transistor-pin swap, pad reuse, a resistor-value error, a body overlap a missing bus tap, a diagonal wire, and a component lead crossing another body.',
        '', 'Source for resistor envelope: [Yageo MFR datasheet, dimensions](https://yageogroup.com/content/datasheet/asset/file/YAGEO-MFR_DATASHEET). Other envelopes are conservative drawing assumptions, not new manufacturer guarantees.','']
    return '\n'.join(s)


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--check',action='store_true');args=parser.parse_args()
    d=json.loads((FOLDER/'layout.json').read_text());check(d)
    for name,data in {'placement-full.svg':draw(d),'pad-map.md':tables(d)}.items():
        p=FOLDER/name
        if args.check:assert p.read_text()==data,f'Regenerate {name}'
        else:p.write_text(data)
    print('Generated files match.' if args.check else 'Wrote placement-full.svg and pad-map.md.')


if __name__=='__main__':main()
