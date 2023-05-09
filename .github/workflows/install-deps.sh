#!/bin/sh

echo $PATH
uname -a

if [ -x /usr/bin/apt-get ];then
	sudo apt-get -y install autotools-dev build-essential automake
	exit $?
fi
if [ -x /usr/bin/yum ];then
	sudo yum -y install autoconf automake gcc make
	exit $?
fi

if [ -x  /usr/sbin/pkg ];then
	#find /usr |grep pkg.conf
	#find /usr |grep pkg.conf | xargs cat
	echo 'pkg_env : {
	http_proxy: "http://squid:3128"
}' | sudo tee -a /usr/local/etc/pkg.conf
	sudo -E ASSUME_ALWAYS_YES=YES pkg install autotools
	exit $?
fi

P_AUTOMAKE=automake
P_AUTOCONF=autoconf
if [ -x /usr/sbin/pkg_info ];then
	uname -a | grep -qi openbsd
	if [ $? -eq 0 ];then
		echo "DEBUG: search automake"
		/usr/sbin/pkg_info -Q automake
		echo "DEBUG: search autotool"
		/usr/sbin/pkg_info -Q autotool
		echo "DEBUG: search autoconf"
		/usr/sbin/pkg_info -Q autoconf
		P_AUTOMAKE="$(/usr/sbin/pkg_info -Q automake | sort --version-sort | tail -n1)"
		echo "DEBUG: AUTOMAKE will be $P_AUTOMAKE"
		P_AUTOCONF="$(/usr/sbin/pkg_info -Q autoconf | grep 'autoconf-[0-9]' | sort --version-sort | tail -n1)"
		echo "DEBUG: AUTOCONF will be $P_AUTOCONF"
		echo "DEBUG: search md5sum"
		/usr/sbin/pkg_info -Q coreutils
	else
		/usr/sbin/pkg_info automake
		/usr/sbin/pkg_info autotool
		/usr/sbin/pkg_info coreutils
	fi
fi

which pkgin
if [ -x /usr/pkg/bin/pkgin ];then
	echo "DEBUG: using pkgin"
	/usr/pkg/bin/pkgin search autotool
	/usr/pkg/bin/pkgin search coreutils

	sudo -E /usr/pkg/bin/pkgin -y install autoconf automake
	exit $?
fi

if [ -x /usr/sbin/pkg_add ];then
	echo "DEBUG: using pkg_add to install $P_AUTOMAKE $P_AUTOCONF"
	sudo -E /usr/sbin/pkg_add $P_AUTOMAKE $P_AUTOCONF || exit $?

	find /usr/local/bin/ |grep aclocal
	exit 0
fi

exit 0
