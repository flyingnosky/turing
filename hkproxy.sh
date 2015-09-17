#!/bin/bash
#A special git clone tool which can automatically help you export some temporary envrionment variables and create\remove some script files, by this way, you needn't write your password into some files which maybe lead to leak your password out.
#You can just use it like command "git", and then input your domain password according to its hint. 
#e.g.:	mygit clone git://git.kernel.org/pub/scm/linux/kernel/git/next/linux-next.git

#USEFUL TIPS: you must change the USERID into your own domain username before you use it

#Author: Justin.Zhao
#Date: 19 Dec. 2014

#Change the USERID into your domain username firstly
USERID=c00284940

if [ $# > 0 ];then
	#Get domain password
	echo -n "Input your domain password:"
	read -s TMPDOMAINPW
	export TMPDOMAINPW

	#Create a temporary script file
	#TMPPROXY="/tmp/"`cat /dev/urandom|sed 's/[^a-zA-Z0-9]//g'|strings -n 6|head -n 1`
	TMPPROXY=`mktemp`
	#echo $TMPPROXY
	echo '#!/bin/bash' > $TMPPROXY
	echo '/usr/bin/socat - PROXY:proxyhk.huawei.com:$1:$2,proxyport=8080,proxyauth='$USERID':$TMPDOMAINPW' >> $TMPPROXY
	chmod 700 $TMPPROXY

	#export variable
	export http_proxy="http://$USERID:$TMPDOMAINPW@proxyhk.huawei.com:8080"
	export https_proxy="https://$USERID:$TMPDOMAINPW@proxyhk.huawei.com:8080"
	export GIT_PROXY_COMMAND=$TMPPROXY

	#echo $1
	#echo $#
	$@ 
	
	echo ""
	echo "Running..."

	#Remove all temporary files and enviroment variables
	sleep 0.5
	unset TMPDOMAINPW 
	unset http_proxy
	unset https_proxy
	unset GIT_PROXY_COMMAND
	rm -rf $TMPPROXY
fi
