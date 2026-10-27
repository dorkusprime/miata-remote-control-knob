# Parts sourcing — one SSR plus digital pot

Current prototype: [LCC110 + AD8400ARZ50](sony-interface.md). User approved trying SOIC-8 hand soldering, with practice first. No new purchases reported. Listings researched 2026-10-27; advertised identity/package are not proof of authenticity or actual shipped contents. Prices, live stock and delivery were not verified. Some product pages failed to load; chip links are indexed candidates, not confirmed offers.

## Pot, adapter and practice

| Item | Qty needed | Amazon US candidate | Verification / selection |
| --- | ---: | --- | --- |
| AD8400ARZ50, 50 kohm, SOIC-8 | 1 | [Single advertised chip, B0F4F4SRPX](https://www.amazon.com/dp/B0F4F4SRPX); [four advertised chips, B0F7SCGRGT](https://www.amazon.com/dp/B0F7SCGRGT) | Indexed titles match exact part/package; seller/price/shipping not confirmed. The REEL suffix is packaging, not an electrical difference. Prefer an offer with credible actual-part photos. Do not choose the 10 kohm version. |
| Solder-on SOIC-8/SOP-8 to DIP-8 adapter | At least 2: practice + real | [Four-board kit, B096BD3JHV](https://www.amazon.com/dp/B096BD3JHV) | Listing advertises 1.27 mm SMD pitch, 2.54 mm header pitch, roughly 10.16 mm square boards and separate sockets. Check that the supplied side fits narrow 150 mil body SOIC-8. This is a candidate; inspect actual board before soldering. |
| Inexpensive practice IC in narrow SOIC-8 | 1–2 | [NE555D/NE555DR SOP-8, advertised 20 pieces, B0FP2G4XQY](https://www.amazon.com/dp/B0FP2G4XQY) | Indexed package matches the soldering geometry. Use only for unpowered soldering/continuity practice; it is not a pot substitute. Compare delivered package to the adapter before starting. |

Do not buy a 0.65 mm-only MSOP/TSSOP adapter, wide 5.4 mm body-only adapter, or large programming/ZIF socket for the permanent build. On dual-sided adapters use the 1.27 mm footprint. The AD8400 has approximately 3.9 mm body width and 1.27 mm lead pitch; footprint geometry, not only the ambiguous SOP/SSOP wording in a marketplace title, determines fit. Header row spacing/socket height and final protoboard placement remain physical dry-fit checks.

Practice package size can be verified from the actual NE555D body/lead spacing. A matching spare chip already on hand is equally useful; no need for a dedicated large practice kit.

## Supporting parts — check existing supplies first

| Part | Qty | Notes |
| --- | ---: | --- |
| LCC110 DIP-8 | 1 | Four already on hand; use one |
| 2N3904 TO-92 | 1 | Relay input driver; verify exact E/B/C pinout |
| 330 ohms, 1%, axial | 1 | Optical LED current |
| 1 kohm, 1%, axial | 1 | Transistor base current |
| 2.7 kohm, 1%, axial | 1 | Sony-side series resistor |
| 47 kohm, axial | 4 | Reset pull-downs; 1–5% acceptable |
| 100 nF ceramic, >=10 V | 1 | Pot bypass capacitor |
| 4 x 47 kohm bussed SIP-5 network | Optional replacement for four 47 kohm resistors | Bourns 4605X-101-473LF example; common pin 1 to GND. Not required for the first mock-up |

Use existing solderless boards, headers and sockets for the prototype. Supporting component shopping is not complete; inspect what is already on hand before buying kits. The old seventeen-resistor quantities below do not apply. A practical tool list and assembly order are in [SOIC-8 assembly](smd-assembly.md).

## Archived five-relay shopping list — not for this build

The following preserves previous sourcing evidence only. Do not buy five AQY212EH devices or the old resistor quantities for the current circuit.

Amazon US listings checked 2026-10-26 (client local date). Listing descriptions/indices are evidence of advertised parts, not independent proof of stock, authenticity, or delivery date. The relay product pages did not load reliably enough to confirm live price/shipping.

## PhotoMOS switches

Need **five AQY212EH**, normally-open, through-hole **DIP-4**. These exact-part ten-pack listings were found on Amazon US:

- [10 pcs AQY212EH DIP-4 — B0C2Z9DQMV](https://www.amazon.com/dp/B0C2Z9DQMV).
- [10pcs/lot AQY212EH AQY212 212EH DIP4 — B0CMR62QHR](https://www.amazon.com/dp/B0CMR62QHR).

One ten-pack supplies five used switches and five spares. The indexed titles match the required MPN and package. Price, delivery, seller, and actual shipped component remain unverified. Confirm the selected item is AQY212EH DIP-4 before purchase; variants in the AQY21 family have different ratings and lead forms. Electrical acceptance follows the meter procedure in [the prototype guide](sony-interface.md).

## Resistors

[**PANMILED 150 values / 1500 pieces, 1/4W 1% metal-film assortment — B0D265GBKM**](https://www.amazon.com/dp/B0D265GBKM).

The Amazon listing specifies ten of each value, axial 0.25W parts, and ±1% tolerance. Its value list includes all six values required here: 220 ohms, 2.2k, 3.3k, 4.7k, 6.8k, and 47k. One kit covers this build. If the user already has these values at 1%, no kit is necessary. Price/delivery were not reliably established.

| Value | Needed | Supplied by listed kit |
| --- | ---: | ---: |
| 220 ohms | 5 | 10 |
| 2.2k | 4 | 10 |
| 3.3k | 1 | 10 |
| 4.7k | 1 | 10 |
| 6.8k | 1 | 10 |
| 47k | 5 | 10 |

Total installed: 17 resistors. The ordinary small 17-value resistor assortments were not selected because the exact 3.3k/6.8k coverage cannot be assumed. SMD and 5%-tolerance kits do not match this BOM.

## Existing supplies / remaining mechanical items

The user has solderless socket-style prototyping boards and header/socket options, so the first mock-up can use existing jumpers. The permanent layout remains the previously selected ElectroCookie Mini; purchase/arrival is not independently confirmed. Removable cable pigtails, wire, insulation, and strain relief can be selected from existing supplies before buying more. No extra power module or mux is required for this interface.

All Sony-side testing initially happens with the receiver cable physically disconnected. The controller firmware still has no active Sony GPIO backend.
