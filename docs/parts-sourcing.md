# Parts sourcing — Sony resistor interface

Original AQY212EH design retained. The user has four LCC110 devices on hand; they are not used in this build. No purchases have been reported for the items below.

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
