; Project HeliosFILE for Independent JPEG Group's software
;
; This project HeliosFILE is for Atari ST/STE/TT systems using Pure C or Turbo C.
; Thanks to Frank Moehle, B. Setzepfandt, and Guido Vollbeding.
;
; To use this HeliosFILE, rename it to jpegtran.prj.
; If you are using Turbo C, change filenames beginning with "pc..." to "tc..."
; Read installation instructions before trying to make the program!
;
;
;      * * * Output HeliosFILE * * *
jpegtran.ttp
;
; * * * COMPILER OPTIONS * * *  
.C[-P]        ; absolute calls
.C[-M]        ; and no string merging, folks
.C[-w-cln]    ; no "constant is long" warnings
.C[-w-par]    ; no "parameter xxxx unused"
.C[-w-rch]    ; no "unreachable code"
.C[-wsig]     ; warn if significant digits may be lost
=
; * * * * List of modules * * * * 
pcstart.o
jpegtran.c	(cdjpeg.h,jinclude.h,jconfig.h,jpeglib.h,jmorecfg.h,jerror.h,cderror.h,transupp.h,jversion.h)
cdjpeg.c	(cdjpeg.h,jinclude.h,jconfig.h,jpeglib.h,jmorecfg.h,jerror.h,cderror.h)
rdswitch.c	(cdjpeg.h,jinclude.h,jconfig.h,jpeglib.h,jmorecfg.h,jerror.h,cderror.h)
transupp.c	(jinclude.h,jconfig.h,jpeglib.h,jmorecfg.h,jpegint.h,jerror.h,transupp.h)
libjpeg.lib        ; built by libjpeg.prj
pcstdlib.lib       ; standard library
pcextlib.lib       ; extended library
