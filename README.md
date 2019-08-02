# FirmwareTY71M
Firmware for NUC122 based keyboards (eg. drevo calibur)

## The project state
State UNFINISHED, neither USB or Bluetooth are working at the moment, (USB has still an error with device initialisation wip..)

## Contents of this repo
This repository contains the necessary development tools for the firmware. The IDE and compiler is included to have relative paths and make it more straightforward to get started with development.

## Project goal
This project aims to create a simple firmware for the ty71 keyboard board used for example in the drevo calibur.
If possible the firmware sould be kept as simple as possible, no realtime os, no dynamic configuration files, so the user can edit the c files and adapt it to their own configuration / keymaping needs.
Because of license issues the Nuvoton standard libary can't be used (they do not have provided any open source license).

## Howto use
please read HOWTOSTARUP.md file first and DON'T START CODEBLOCKS DIRECTELY because it will not have the necessary compiler configuration for arm then.
TLDR: Start START_codeblocks_with_gdb.bat, compile, start debugger which will upload the firmware.