
ibhttppost=$(dirname $0)/support-files/ibhttppost

# infobright web
infobright_homeurl="www.infobright.org"
inforbight_port=80
infobright_registration_page="http://www.infobright.org/Downloads/ICE/"

# Infobright server to receive http post data
infobright_httpserver="loopfuse.net"
infobright_page="/webrecorder/post"


exists() {
	which "${1%% *}" 2> /dev/null > /dev/null
	return $?
}

# Run default browser
RunDefaultBrowser_Linux()
{
	if [ -x /usr/bin/gconftool-2 -a -x /usr/bin/gnome-default-applications-properties ]; then
		default_browser=$(gconftool-2 -g /desktop/gnome/url-handlers/http/command 2>/dev/null | sed -e 's/%s//; s/\"\"//; s/^\ *//; s/\ *$//')
		
		echo "$default_browser" | grep -q "htmlview" && return 1;
		echo "$default_browser" | grep -q "gnome-open" && return 1;
		
		[ ! -f "$default_browser" ] && return 1
		$default_browser "$1"
		exit 0
	fi
	return 1
}

defaultsRegistrationMsg()
{
	#echo "Register your copy of ICE!"
	#echo "Receive a free copy of the User Manual (a \$50 value) or other benefits."
	echo "Register now $infobright_registration_page."
}

UserRegistration()
{	
	input="no"
	issunos=`uname -s | grep SunOS`
	# solaris default echo does not have -n option need to use \c
	if  test "x$issunos" != "x" ; then
		ECHO="echo"
		eop="\c"
	else
		ECHO="echo -n"
		eop=""
	fi
	
	if test ! -f "$ibhttppost"
	then
		defaultsRegistrationMsg
		return 0
	fi
	echo "Register your copy of ICE and receive a free copy of the User Manual (a \$50 value) as well as a copy of the Bloor Research Spotlight Report \"What's Cool About Columns\" which explains the differences and benefits of a columnar versus row database."
	$ECHO "Registration will require opening an HTTP connection to Infobright, do you wish to register now? [Y/N]: "
  	read input
	input=`echo $input | tr '[:upper:]' '[:lower:]'`
	if [ "$input" != "y" ]; then
		defaultsRegistrationMsg
		return 0
	fi
	
	$ibhttppost --checkinternet $infobright_homeurl $inforbight_port "none" && internet_status="yes"
	if [ "$internet_status" != "yes" ]; then
		echo "We were unable to reach to infobright server."
	fi
  
    if [ ! -z $DISPLAY ] &&  [ -x /usr/bin/gnome-open ]; then
		$ECHO "Can we try to open your default browser to take you to Infobright registration page? [Y/N]: "
		read input
		input=`echo $input | tr '[:upper:]' '[:lower:]'`
		if [ "$input" = "y" ]; then
			uname -a | grep -q GNU && RunDefaultBrowser_Linux $infobright_registration_page
		fi
		echo "We could not find a suitable browser to open."
	fi

	if [ "$internet_status" != "yes" ]; then
		defaultsRegistrationMsg
		return 0
	fi

	$ECHO "Do you want to tell us a bit about yourself? We will try to send your information to our server. [Y/N]: "
	read input
	input=`echo $input | tr '[:upper:]' '[:lower:]'`
	if [ "$input" = "y" ]; then
		$ibhttppost --register $infobright_httpserver $inforbight_port $infobright_page && registrared="yes"
		if test "$registrared" = "yes" 
        then
		   echo "We successfully submitted your information to our server. Thank you for registration."
		   return 0
        else
		   echo "We failed to send your information."
		fi
	fi

	defaultsRegistrationMsg
	return 0
}
