# FirmwareTY71M
Firmware for NUC122 based keyboards (eg. drevo calibur)

## The project is UNFINISHED, at the current state of development your keyboard will become a brick ( with controllable rgb-led's xD ) if you flash this firmware

## Contents of this repo
This repository contains the necessary development tools for the firmware. The IDE and compiler is included to have relative paths and make it easyer to get started with development.

## Project goal
This project aims to create a simple firmware for the ty71 keyboard board used for example in the drevo calibur.
If possible the firmware sould be kept as simple as possible, no realtime os, no dynamic configuration files, so the user can edit the c files and adapt it to their own configuration / keymaping needs.
The CMSIS and nuvoton NUC122 std libaries will be used.

## Howto use
please read HOWTOSTARUP.md first and DON'T START CODEBLOCKS DIRECTELY because it will not have the necessary compiler configuration for arm then.
