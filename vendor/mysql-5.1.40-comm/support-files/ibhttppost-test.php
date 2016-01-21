<?php
// This php page is used to test ibhttppost.cpp. 
// ibhttppost comamnd line tool posts user registration information though http 
// to web server (page ibhttppost-test.php)

// Here we display field values.

// Installation: install apache, php5
// add following lines 
#LoadModule php5_module modules/libphp5.so
#
#<FilesMatch \.php$>
#    SetHandler application/x-httpd-php
#</FilesMatch>
#AddType application/x-httpd-php .php
#AddType application/x-httpd-php-source .phps
// in /etc/apache2/httpd.conf file
// Copy this file in /var/www/

// run apache
// run ibhttppost tool
//./ibhttppost --register localhost 80 /ibhttppost-test.php --verbose

	echo $_POST["formid"];
	print "<br>";
	echo $_POST["cid"];
	print "<br>";
	echo $_POST["lead_source"];
	print "<br>";
	echo $_POST["first_name"];
	print "<br>";
	echo $_POST["last_name"];
	print "<br>";
	echo $_POST["company"];
	print "<br>";
	echo $_POST["title"];
	print "<br>";
	echo $_POST["city"];
	print "<br>";
	echo $_POST["country"];
	print "<br>";
	echo $_POST["email"];
	print "<br>";
	echo $_POST["phone"];
	print "<br>";
	echo $_POST["industry"];
	print "<br>";
	echo $_POST["project"];
?> 
