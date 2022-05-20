# GhettoDSP
 
GhettoDSP is a family of DSP, bluetooth and power supply boards for building (or converting to) wireless bluetooth speakers.  

Primary components are the Atmega 328p MCU, Microchip BM64 BT module, and ADAU1701 DSP chip.   The BT module can be omitted for standalone 2x4 or 2x8 DSP operation.

The BT audio pipeline is I2S (24-bit, 48khz, 64fs).   In BT or aux-in mode, unlimited number of boards can be paired together for wireless stereo or "party" mode.   This is possible through the nSPK feature of the BM64 module.

Optional 20x2 or 20x4 I2C LCD module and rotary encoder for DSP and BT settings.

Optional power supply module (GhettoPWR) for buck/boost and lithium battery BMS.

Base DSP config is created using SigmaDSP, and is programmed directly by the MCU at power on.  A standard socket is provided for a USBi adapter for realtime testing with a Dayton ICP3 (ect.).

There are currently four boards:

* GhettoDSP - Bluetooth I2S receiver with up to 4 channels of DSP (ADAU1701-based)
* GhettoDAC - Dual PCM1502A DAC daughter card for GhettoDSP providing additional 4 outputs @ 2V RMS (total 8-ch)
* GhettoPWR - Power control, boost and buck converter, and BMS / charging for lithium batteries (4-8 cells)
* HooptyDSP - Stand-alone, 12V powered GhettoDSP with integrated power control for mobile/car audio use

This Arduino code is for both GhettoDSP and HooptyDSP boards.

Additional documentation, example sigmaDSP project, ect, is a work in progress.

Special thanks to [MCUDude](https://github.com/MCUdude/SigmaDSP) and [konikoni428](https://github.com/konikoni428/BM64_arduino) for helping to make all this possible.

[EasyEDA project page for GhettoDSP board](https://oshwlab.com/k6rp/mega328p-adau1701_copy_copy_copy_copy_copy_copy_copy_copy_copy)

