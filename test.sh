#!/bin/sh

md5() {
	MD5="$(md5sum $1 | cut -d' ' -f1)"
}

compare() {
	md5 "$1"
	D1="$MD5"
	md5 "$2"
	D2="$MD5"
	case $3 in
	equal)
		if [ "$D1" != "$D2" ];then
			echo "ERROR: $1 ($D1) is not equal to $2 ($D2)"
			exit 1
		fi
		echo "GOOD: $D1 = $D2"
	;;
	diff)
		if [ "$D1" = "$D2" ];then
			echo "ERROR: $1 ($D1) is not differant than $2 ($D2)"
			exit 1
		fi
		echo "GOOD: $D1 != $D2"
	;;
	esac
}

dd if=/dev/zero of=test.source bs=4k count=16
dd if=/dev/urandom of=test.dest bs=4k count=16
dd if=/dev/zero of=test.dest bs=4k count=8 skip=4

./binrsync -s -d test.source test.dest

echo "TEST1 4K of 0 vs 4K of 1"
./testgen --size 1 --pattern 0 disk1.img || exit $?
./testgen --size 1 --pattern 1 disk2.img || exit $?
compare disk1.img disk2.img diff
./binrsync -g disk1.img disk2.img || exit $?
compare disk1.img disk2.img equal

echo "TEST2 4K of 0 vs 4K of 0"
./testgen --size 1 --pattern 0 disk1.img || exit $?
./testgen --size 1 --pattern 0 disk2.img || exit $?
compare disk1.img disk2.img equal
./binrsync -g disk1.img disk2.img || exit $?
compare disk1.img disk2.img equal
rm disk*

echo "TEST3 8K of 0 vs 4K of 1"
./testgen --size 2 --pattern 0 disk1.img || exit $?
./testgen --size 1 --pattern 1 disk2.img || exit $?
compare disk1.img disk2.img diff
./binrsync -g disk1.img disk2.img || exit $?
compare disk1.img disk2.img equal
# TODO check size
ls -l disk*
rm disk*

echo "TEST4 4K of 0 vs 8K of 1"
./testgen --size 1 --pattern 0 disk1.img || exit $?
./testgen --size 2 --pattern 1 disk2.img || exit $?
compare disk1.img disk2.img diff
./binrsync -g disk1.img disk2.img || exit $?
compare disk1.img disk2.img equal
# TODO check size
ls -l disk*
rm disk*
