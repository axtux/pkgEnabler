#!/bin/bash

cd $PS3DEV/work/allowpkg
make pkg

ftp -inv <<END
open $1
user axtux ps3
bin
delete /dev_hdd0/game/AXTUX0010/USRDIR/EBOOT.BIN
put /home/axtux/Bureau/ps/tools/work/allowpkg/build/pkg/USRDIR/EBOOT.BIN /dev_hdd0/game/AXTUX0010/USRDIR/EBOOT.BIN
site EXITAPP
close
exit
END

make clean
