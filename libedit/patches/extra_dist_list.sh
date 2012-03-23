#!/bin/bash -e

## update EXTRA_DIST list in Makefile.am

f="Makefile.am"
""cd ..

## remove old list
ed $f <<EOF
/^## GENERATED BY SCRIPT/,\$d
w
EOF

## insert new list
echo "## GENERATED BY SCRIPT - DO NOT EDIT BELOW" >> $f
echo "" >> $f

echo "EXTRA_DIST += patches/README \\" >> $f
for i in $(ls -1 patches/*.patch);do printf '%*s%s \\\n' 13 ' ' $i >> $f;done
echo "             patches/cvs_export.sh \\
             patches/patches_make.sh \\
             patches/patches_apply.sh \\
             patches/patches_check.sh \\
             patches/extra_dist_list.sh \\
             patches/update_version.sh \\
             patches/update_dist.sh \\
             patches/timestamp.cvsexport" >> $f

echo "" >> $f
