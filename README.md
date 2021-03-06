# batquery

Battery status interface made for statusbars.

## run

There are no dependencies required to build this program, other than `make`.
Display of icons requires the use of a [Nerd Font](https://www.nerdfonts.com/font-downloads).

```sh
make
./batquery /sys/class/power_supply/BAT0
make install
```

## usage

```
usage: batquery [-i] [-p | -t] <battery_path>
                 -i: show battery status icon
                 -p: show battery percent
                 -t: show time remaining
```

## statusbar 

`batquery` writes all non-error information to the standard output stream. Statusbars which operate by embedding a program's standard output stream are likely to be fully compatible with this program. For example, [dwmblocks](https://github.com/torrinfail/dwmblocks) works as expected, as does [tmux](https://en.wikipedia.org/wiki/Tmux).
