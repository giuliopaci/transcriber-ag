#! /bin/sh

#cat $1 | sed 's/( END_OF_TEXT_UNIT )//' \
#       | sed 's/( @[^ ]*\/CD )//' \
#       | sed 'sX\\/X/Xg' \
#       | sed 'sX\([^ ]*\)/\([^ ]*\)X(\2 \1)Xg' \
#       | sed 's/XXX/-NONE-/g'

cat $1 | sed 's/( END_OF_TEXT_UNIT )//' \
       | sed 'sX\\/X/Xg' \
       | sed 'sX\([^ ]*\)/\([^ ]*\)X(\2 \1)Xg' \
       | sed 's/XXX/-NONE-/g'
