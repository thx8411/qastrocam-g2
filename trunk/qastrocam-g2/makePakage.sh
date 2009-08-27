lineChanged=forceBuild
test -f forceBuild || lineChanged=`cvs -q diff -r HEAD | grep -v "^? .*" | wc -l`
echo there is "$lineChanged" line changes
test "$lineChanged" = 0 && echo sources not updated && exit 0 

cvs update
PKG_VERSION=4.0.1pre`date +%Y%m%d`
#PKG_VERSION=4.0.1
dch -v $PKG_VERSION
dpkg-buildpackage -nc -ICVS -rfakeroot -e"$EMAIL" #&& dpkg-buildpackage -S -ICVS -rfakeroot -e"$EMAIL"

test -d ~/Web/3demi/www.3demi.net/htdocs/debian/debs/ && cp ../qastrocam*_"$PKG_VERSION"* ~/Web/3demi/www.3demi.net/htdocs/debian/debs/ && rm -f forceBuild
