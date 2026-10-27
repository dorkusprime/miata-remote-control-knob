# Parts and purchase status

The user confirmed the DigiKey order on 2026-10-27 after reviewing the revised shopping-cart printout. Quantities intentionally include inexpensive spares for future projects. **Ordered does not mean received or inspected.** The local cart PDF is outside this public repository; do not copy account, shipping or order-identifying information into Git.

The [current circuit](sony-interface.md) is entirely through-hole. These exact parts supersede earlier AD8400/SMD and five-relay shopping lists.

| Part | Per controller | Ordered | Package / purpose |
| --- | ---: | ---: | --- |
| [MCP41050-I/P](https://www.digikey.com/en/products/detail/microchip-technology/MCP41050-I-P/362080) | 1 | 2 | 50 kohm digital pot, DIP-8 |
| [SN74HCT125N](https://www.digikey.com/en/products/detail/texas-instruments/SN74HCT125N/376860) | 1 | 2 | SPI translation, DIP-14 |
| [2N3904BU](https://www.digikey.com/en/products/detail/onsemi/2N3904BU/1413) | 1 | 5 | NPN optical-input driver, TO-92 |
| [MFR-25FBF52-330R](https://www.digikey.com/en/products/detail/yageo/MFR-25FBF52-330R/9138136) | 1 | 10 | 330 ohm, 1%, 1/4 W axial |
| [MFR-25FBF52-1K](https://www.digikey.com/en/products/detail/yageo/MFR-25FBF52-1K/13011) | 1 | 10 | 1 kohm, 1%, 1/4 W axial |
| [MFR-25FBF52-2K7](https://www.digikey.com/en/products/detail/yageo/MFR-25FBF52-2K7/9138110) | 1 | 10 | 2.7 kohm, 1%, 1/4 W axial |
| [MFR-25FBF52-47K](https://www.digikey.com/en/products/detail/yageo/MFR-25FBF52-47K/9138173) | 4 | 10 | 47 kohm, 1%, 1/4 W axial |
| [K104K15X7RF5TL2](https://www.digikey.com/en/products/detail/vishay-beyschlag-draloric-bc-components/K104K15X7RF5TL2/286538) | 2 | 5 | 100 nF, 50 V X7R radial ceramic |
| [ECA-1CM100B](https://www.digikey.com/en/products/detail/panasonic-industry/ECA-1CM100B/14301375) | 1 | 5 | 10 uF, 16 V radial electrolytic |

## Existing supplies and mechanical items

- Four LCC110s are on hand; one is required per controller.
- Existing solderless protoboards, headers and sockets support the first mock-up. Optional IC sockets: two DIP-8 and one DIP-14 per circuit.
- ElectroCookie Mini is the selected no-cut permanent board candidate. Actual package/socket/passive fit remains unverified; do not order a larger board until a dry-fit establishes need.
- Housing, wire, connectors, insulation and strain relief require a final mechanical check. Six conductors serve the interface; a separate four-wire cable serves the Twist.
- A known 10 kohm resistor is useful as a simulated-source bench aid; use existing stock. It is not part of the installed circuit.

## Additional power item in the order

F23J5V3A1S-C (DigiKey 4169-F23J5V3A1S-C-ND), quantity one: advertised 8–23 V input, fixed 5 V / 3 A USB-C output. This is a 15 W converter, separate from the interface BOM. It was listed as an Envistia Mall marketplace item with possible separate shipping. The cart does not establish automotive transient qualification or USB Power Delivery support. An existing 15 W converter is already on hand; final converter assignment is not recorded.

The revised printout estimated $18.71 subtotal, $8.49 shipping and $1.57 tariff, totaling $28.77. These are cart estimates, not a verified final invoice or current price quote. Arrival and shipped contents remain to be checked.

## Substitutions

Do not replace HCT125 with HC125: the TTL-compatible input threshold matters for 3.3 V control at a 5 V supply. Do not replace the 50 kohm pot with the 10 kohm version, or assume another MCP41xx has the same pins, logic thresholds, wiper limits or command protocol. Verify transistor lead ordering against the actual manufacturer. The LCC110's normally-open contact is pins 5/6; its normally-closed contact is not used.

Historical pricing and abandoned SMD/AQY searches are retained in the [archive](archive/README.md), outside the active purchasing path.
