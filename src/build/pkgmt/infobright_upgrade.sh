#!/bin/sh

help()
{
  echo "Usage: $0 [-u <user>] [-p <password>]"
  exit 1
}

parse_arguments() 
{
  for arg do
    case "$arg" in
      # these get passed explicitly to mysqld
      --socket=*)  SOCKET=`echo "$arg" | sed -e "s;--socket=;;"` ;;
      --port=*)    PORT=`echo "$arg" | sed -e "s;--port=;;"` ;;
    esac
  done
}

USER="root"
PASSWORD=""

while getopts u:p: opt ; do
  case "$opt" in
    u) USER="$OPTARG";;
    p) PASSWORD="$OPTARG";;
    \?) help;;
  esac
done

PREFIX=`pwd`
if test ! -f $PREFIX/bin/mysqld
then
  echo "Please run this script from inside the installed folder."
  echo "--------------------------------------"
  exit 1
fi

if test ! -x $PREFIX/bin/mysql
then
  echo "Error: mysql client $PREFIX/bin/mysql was not found."
  exit 1
fi

cmd="$PREFIX/bin/mysql --user=$USER"
if [ -n "$PASSWORD" ]; then
	cmd="$cmd --password=$PASSWORD"
fi

# Get first arguments from the my-ib.cnf file, groups [mysqld]
if test -x $PREFIX/bin/my_print_defaults
then
  print_defaults="$PREFIX/bin/my_print_defaults"
else
  echo "Error: conf file parser tool $PREFIX/bin/my_print_defaults was not found."
  exit 1
fi

my_conf="/etc/my-ib.cnf"
parse_arguments `$print_defaults --defaults-file=$my_conf mysqld`

if test -z "$SOCKET" -a -z "$PORT"
then
  echo "Error: socket and port has empty value in the $my_conf file. They must be set properly."
  exit 1
fi

if [ -n "$SOCKET" ]; then
	cmd="$cmd --socket=$SOCKET"
else
	cmd="$cmd --port=$PORT"
fi

$cmd < $PREFIX/share/infobright/infobright_upgrade.sql
if [ $? != 0 ];
then
  echo "Error during running $PREFIX/share/infobright/infobright_upgrade.sql";  
  exit 1
fi

echo "Upgrade finished successfully"
