# ICS2021 PA(riscv64)

> # ICS2021 Programming Assignment
>
> This project is the programming assignment of the class ICS(Introduction to Computer System)
> in Department of Computer Science and Technology, Nanjing University.
>
> For the guide of this programming assignment,
> refer to http://nju-ics.gitbooks.io/ics2021-programming-assignment/content/
>
> To initialize, run
> ```bash
> bash init.sh subproject-name
> ```
> See `init.sh` for more details.
>
> The following subprojects/components are included. Some of them are not fully implemented.
> * [NEMU](https://github.com/NJU-ProjectN/nemu)
> * [Abstract-Machine](https://github.com/NJU-ProjectN/abstract-machine)
> * [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
> * [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)

**NOT a NJU Student,** I did this only to kill time.

- This is a `RISCV64` version (quite similar to `RV32`, though not compatible in binary).
- All Required tasks (those yellow ones) completed and tested (maybe), except some related to `PAL`(I can't find a perfectly workable copy of PAL data, so only simple tests have been done about `PAL`)
- Optimized speed by reduce device update frequency (God knows why the original framework call `gettimeofday` when device-update every cycle)
- No support for audio \| disk \| sdcard \| full-rtc 
- Difftest for only PA1 and PA2 (Because I didn't implement Privilege U or Interrupt sync to spike (even no detach nor snapshot))

> This repo has a almost workable PAL: https://github.com/huoshan12345/SdlpalEdit/tree/master/pal
>
> Remember to change all these filenames to lowercase, sdlpal is sensitive to case (but MS-DOS don't care)

