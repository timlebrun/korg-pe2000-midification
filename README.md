# Korg PE2000 Midification

This repository should contain everything related to my attempt at adding a MIDI Input and Output to an old Korg PE2000 synthetizer.

## The Synth

`// TODO: Add pictures`
`// TODO: Add diagrams`

![Block Diagram of the Korg PE2000 Synth internal circuits](./docs/diagram-synth-blocks.png)

## The Plan

![Structure of the main plan for the MIDIficiation](./docs/diagram-main-plan.png)


## Roadmap

- [x] Synth internal reverse engineering
- [ ] Arduino Board MIDI Communication
- [ ] 48 key matrix Input from the Keyboard
- [ ] 48 output 15v switches to the Envelope Generators
- [ ] Proper Matrix board with connectors

# Resources

- [Service Manual / Schematics](./docs/schematics.pdf)
- [Tubbutec - Organ Donor](https://tubbutec.de/blog/midi-for-korg-pe-1000-organdonor/)