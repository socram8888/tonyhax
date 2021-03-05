
tonyhax
=======

Software backup loader exploit thing for the Sony PlayStation 1.

How does this works?
--------------------

This backup loader uses a save game exploit present in multiple Tony Hawk's games for the PS1.

This exploit consists of a specially crafted save game with:

 - Highscores replaced with a first-stage payload of 144 bytes.
 - An abnormally long skater name, with the memory address of the first-stage payload inserted.

When entering the skater customization menu, the skater name is copied via a `sprintf` with no bounds check into the stack. This causes a buffer overflow, overwriting the function's return address, and jumping to our payload as soon as it is done.

This first stage payload is about 144 bytes, and its sole purpose is to load the secondary program loader (SPL) from an additional save file in the memory card. Once loaded, it jumps straight to it.

The secondary program loader is a much more complex program and it:

 - Resets the stack pointer.
 - Reinitializes the system kernel (RAM, devices...).
 - Reinitializes the GPU, which is used as a debug screen.
 - Unlocks the CD drive to accept discs missing the SCEx signature, leveraging the [CD BIOS unlock commands](https://problemkaputt.de/psx-spx.htm#cdromsecretunlockcommands) found by Martin Korth.
 - Waits for a new drive to be inserted
 - Loads the system configuration to configure once more the kernel.
 - Finally, executes the game.

TL;DR: It uses _magic_.

Installation
------------

To install this exploit, you'd need a means of copying the save file to a PS1 memory card. Personally, I've used a PS2 with [Free McBoot](https://www.ps2-home.com/forum/viewtopic.php?t=1248) and uLaunchELF.

All you have to do is copy the game's crafted save file and the `TONYHAX-SPL` file into the card. That's it.

Once installed, you can freely copy it to other cards using the PS1 and the memory card management menu, and distribute it freely amongst friends.

Usage
-----

Once installed, all you have to do is boot the game like you'd normally do.

Once you get to the main menu, it'll load the save game (it should say "Loading TONYHAX"). After it's done, go to the "CREATE SKATER" function and press X. After a couple seconds, tonyhax should boot.

Save games
----------

 * `BASLUS-01066TNHXG01`: Tony Hawk's Pro Skater 2 (NTSC-U) (SLUS-01066)
 * `BESLES-02908TNHXG01`: Tony Hawk's Pro Skater 2 (PAL-E) (SLES-02908)
 * `BESLES-03645TNHXG01`: Tony Hawk's Pro Skater 3 (PAL-E) (SLES-03645)

Compatibility
-------------

I've personally only attempted this with a PAL SCPH-102 PSone, but according to Martin Korth's documentation this should work with:

 * Every PAL console.
 * Every NTSC-U console **except** the very early SCPH-1000.
 * NetYaroze consoles.

However, this will **not** work with:

 * Japanese NTSC-J consoles (stubbed/bugged CD unlock).
 * NTSC-U SCPH-1000 consoles (BIOS predates the introduction of the CD unlock command).
