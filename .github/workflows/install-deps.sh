#!/bin/sh

if [ -x /usr/bin/apt-get ];then
	sudo apt-get -y install autotools-dev build-essential automake libssl-dev
	exit 0
fi
if [ -x /usr/bin/yum ];then
	sudo yum -y install autoconf automake gcc make openssl-devel
	exit 0
fi
exit 1
