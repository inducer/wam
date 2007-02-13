#! /bin/bash
TTF=base/creation/fonts/Vera.ttf
TTFBOLD=base/creation/fonts/VeraBd.ttf
OUT=base/fonts
sdlucid_ripttf ttf=$TTFBOLD out=$OUT/ingame.fnt size=10 --fg=0,128,255 --alpha
sdlucid_ripttf ttf=$TTF out=$OUT/standard.fnt size=8 --alpha 
sdlucid_ripttf ttf=$TTF out=$OUT/menu_dark.fnt size=20 --alpha --fg=192,192,192 --rim=2,96,96,96
sdlucid_ripttf ttf=$TTF out=$OUT/menu_light.fnt size=20 --alpha --fg=255,255,255 --rim=2,128,128,128
sdlucid_ripttf ttf=$TTF out=$OUT/credits.fnt size=20 --alpha --fg=128,128,128
sdlucid_ripttf ttf=$TTF out=$OUT/credits_bold.fnt size=20 --alpha --fg=255,255,255 --rim=2,128,128,128

