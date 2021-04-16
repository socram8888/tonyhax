
tonyhax
=======

Software backup loader exploit thing for the Sony PlayStation 1.

For more information, look at [its section at orca.pet](https://orca.pet/tonyhax/).

For using tonyhax you have two options:

| Method                                 | Game exploit          | FreePSXBoot           |
|----------------------------------------|-----------------------|-----------------------|
| Needs an original game?                | Yes                   | No                    |
| Needs a memory card?                   | Yes                   | Yes                   |
| Can the memory card store other saves? | Yes                   | No                    |
| Time from off to playing a backup      | ~1m30s                | ~1m                   |

Regarding compatibility:

| Method                                 | Game exploit          | FreePSXBoot           |
|----------------------------------------|-----------------------|-----------------------|
| Compatible with European PS1?          | Yes                   | Yes                   |
| Compatible with American PS1?          | Yes                   | Yes                   |
| Compatible with Japanese PS1?          | No                    | No                    |
| Compatible with European PS2?          | ≤ SCPH-39000 only     | No                    |
| Compatible with American PS2?          | ≤ SCPH-39000 only     | No                    |
| Compatible with Japanese PS2?          | No                    | No                    |

Both behave identically the same feature-wise, so use whichever suits you more.

Launch using a game exploit
---------------------------

The original, well-tested method, which leverages an exploit in supported games to launch tonyhax.

Thid method requires copying to the memory card:
 * The common loader program, available inside the `loader/` folder.
 * One or more game-specific saves, available inside the `entrypoints/` folder.

These files behave like regular game save files:
 * You can keep the memory card plugged after launching the exploit.
 * You can keep using the same memory card for other games just fine.
 * You can have more than one exploit save per memory card.
 * tonyhax can be easily copied to other memory cards by just copying the save files.
 * tonyhax can be uninstalled easily by removing the save files.

When installing:
 * If you are using any sort of visual memory card editor (such as [OrionSoft's PS1 Memory Card Manager](http://onorisoft.free.fr/retro.htm?psx/psx.htm), Dexdrive, etc...), consider using the MCS file.
 * If you are copying it using a PS2 console and uLaunchELF, you'll need to use the raw file. **The name is important - do not rename them**.

These have been tested to work on real hardware:

| Game                                   | Region | Code       | MCS file          | Raw file             |
|----------------------------------------|--------|------------|-------------------|----------------------|
| tonyhax SPL **required**               | -      | -          | tonyhax.mcs       | BESLEM-99999TONYHAX  |
| Brunswick Circuit Pro Bowling          | NTSC-U | SLUS-00571 | brunswick1-us.mcs | BASLUS-00571         |
| Brunswick Circuit Pro Bowling          | PAL-E  | SLES-01376 | brunswick1-eu.mcs | BESLES-01376         |
| Brunswick Circuit Pro Bowling 2        | NTSC-U | SLUS-00856 | brunswick2-us.mcs | BASLUS-00856         |
| Brunswick Circuit Pro Bowling 2        | PAL-E  | SLES-02618 | brunswick2-eu.mcs | BESLES-02618         |
| Castlevania Chronicles                 | NTSC-U | SLUS-01384 | cc-us.mcs         | BASLUS-01384DRACULA  |
| Castrol Honda Superbike Racing         | NTSC-U | SLUS-00882 | castrolsb-us.mcs  | BASLUS-00882CHSv1    |
| Castrol Honda Superbike Racing         | PAL-E  | SLES-01182 | castrolsb-eu.mcs  | BESLES_01182CHSv1    |
| Castrol Honda VTR                      | PAL-E  | SLES-02942 | castrolvtr-eu.mcs | BESLES-02942CHSVTRv1 |
| Cool Boarders 4                        | NTSC-U | SCUS-94559 | coolbrd4-us.mcs   | BASCUS-9455916       |
| Cool Boarders 4                        | PAL-E  | SCES-02283 | coolbrd4-eu.mcs   | BESCES-0228316       |
| Crash Bandicoot 2: Cortex Strikes Back | NTSC-U | SCUS-94154 | crash2-us.mcs     | BASCUS-9415400047975 |
| Crash Bandicoot 2: Cortex Strikes Back | PAL-E  | SCES-00967 | crash2-eu.mcs     | BESCES-0096700765150 |
| Crash Bandicoot 3: Warped              | NTSC-U | SCUS-94244 | crash3-us.mcs     | BASCUS-9424400000000 |
| Crash Bandicoot 3: Warped              | PAL-E  | SCES-01420 | crash3-eu.mcs     | BESCES-0142000000000 |
| Sports Superbike                       | PAL-E  | SLES-03057 | superbike1-eu.mcs | BESLES-03057SSBv1    |
| Sports Superbike 2                     | PAL-E  | SLES-03827 | superbike2-eu.mcs | BESLES-03827SSII     |
| Tony Hawk's Pro Skater 2               | NTSC-U | SLUS-01066 | thps2-us.mcs      | BASLUS-01066TNHXG01  |
| Tony Hawk's Pro Skater 2               | PAL-DE | SLES-02910 | thps2-de.mcs      | BESLES-02910TNHXG01  |
| Tony Hawk's Pro Skater 2               | PAL-E  | SLES-02908 | thps2-eu.mcs      | BESLES-02908TNHXG01  |
| Tony Hawk's Pro Skater 2               | PAL-FR | SLES-02909 | thps2-fr.mcs      | BESLES-02909TNHXG01  |
| Tony Hawk's Pro Skater 3               | NTSC-U | SLUS-01419 | thps3-us.mcs      | BASLUS-01419TNHXG01  |
| Tony Hawk's Pro Skater 3               | PAL-DE | SLES-03647 | thps3-de.mcs      | BESLES-03647TNHXG01  |
| Tony Hawk's Pro Skater 3               | PAL-E  | SLES-03645 | thps3-eu.mcs      | BESLES-03645TNHXG01  |
| Tony Hawk's Pro Skater 3               | PAL-FR | SLES-03646 | thps3-fr.mcs      | BESLES-03646TNHXG01  |
| Tony Hawk's Pro Skater 4               | NTSC-U | SLUS-01485 | thps4-us.mcs      | BASLUS-01485TNHXG01  |
| Tony Hawk's Pro Skater 4               | PAL-DE | SLES-03955 | thps4-de.mcs      | BESLES-03955TNHXG01  |
| Tony Hawk's Pro Skater 4               | PAL-E  | SLES-03954 | thps4-eu.mcs      | BESLES-03954TNHXG01  |
| Tony Hawk's Pro Skater 4               | PAL-FR | SLES-03956 | thps4-fr.mcs      | BESLES-03956TNHXG01  |
| XS Moto                                | NTSC-U | SLUS-01506 | xsmoto-us.mcs     | BASLUS-01506XSMOTOv1 |
| XS Moto                                | PAL-E  | SLES-04095 | xsmoto-eu.mcs     | BESLES-04095XSMOTO   |

Launch using FreePSXBoot
------------------------

tonyhax supports being launched on compatible consoles using the [FreePSXBoot](https://github.com/brad-lin/FreePSXBoot) exploit developed by brad-lin.

This method requires an entire memory card for itself, and thus:
 * The memory card *must be removed after launching the exploit, or games will crash*.
 * The memory card cannot be used to save any game's progress.
 * The memory card depends on a specific version of the BIOS. If plugged on another console, chances are the exploit will get nuked and you'll have to reprogram the memory card.
 * The exploit cannot be copied to another memory card using the console - you'll need a PC with DexDrive or a PS2.
 * The exploit cannot be uninstalled using the console alone either.

If you want to go this route, you'll need to flash the memory card using one of the images available at `freepsxboot/`, depending on your console's BIOS version:

| Model     | BIOS version       |
|-----------|--------------------|
| SCPH-1001 | v2.2               |
| SCPH-1002 | v2.0, v2.1 or v2.2 |
| SCPH-3500 | v2.1               |
| SCPH-5001 | v3.0               |
| SCPH-5501 | v3.0               |
| SCPH-5502 | v3.0               |
| SCPH-5503 | v3.0               |
| SCPH-5903 | v2.2               |
| SCPH-7001 | v4.1               |
| SCPH-7002 | v4.1               |
| SCPH-7501 | v4.1               |
| SCPH-7502 | v4.1               |
| SCPH-9001 | v4.1               |
| SCPH-9002 | v4.1               |
| SCPH-101  | v4.4 or v4.5       |
| SCPH-102  | v4.4 or v4.5       |

Development
-----------

This repository uses git submodules for building certain parts, so after cloning, make sure you run:
```
git submodule init
git submodule update --recursive
```

For compiling tonyhax, you will need a standard MIPS compiler, a bash interpreter and Make, all of which can be installed on Debian/Ubuntu using:
```
sudo apt-get install build-essential gcc-10-mips-linux-gnu
```

After installing all dependencies, you can compile and package it by running `make` on the root of the project, which (hopefully) will result in a .zip file with all the required files created at the root.
