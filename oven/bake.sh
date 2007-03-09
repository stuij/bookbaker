#!/bin/sh

../gbfs/tools/gbfs text.gbfs text.txt > /dev/null
cat ../src/book.pre.gba text.gbfs > book.gba
rm text.gbfs
echo Your freshly baked ebook is waiting for you in this dir. Watch out!! It\'s still hot!