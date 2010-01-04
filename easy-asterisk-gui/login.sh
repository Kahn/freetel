#!/bin/sh
# login.sh
# David Rowe 4 Jan 2010
# CGI for Easy Asterisk login GUI

user=`echo "$QUERY_STRING" | grep -oe "user=[^&?]*" | sed -n "s/user=//p"`
pass=`echo "$QUERY_STRING" | grep -oe "pass=[^&?]*" | sed -n "s/pass=//p"`

echo $QUERY_STRING | grep user > /dev/null
if [ $? -eq 1 ]; then
    # Display form -------------------------------

    cat <<EOF
    <script src="prototype.js"></script>
    <link href="astman.css" media="all" rel="Stylesheet" type="text/css" />
    <script type="text/javascript" src="tooltip.js"></script>
    <link rel="stylesheet" href="tooltip.css" type="text/css" />
EOF
    cat << EOF
    <html>
    <title>Easy Asterisk - Login</title>
    <form action="/cgi-bin/login.sh" method="get">
    <table align="center" width=600>
    <tr><td colspan="2" align="left"><h2>Login</h2></td>
    <tr>
      <tr><td>User:</td><td><input type="text" name="user" ></td></tr>
      <tr><td>Password:</td><td><input type="text" name="pass" ></td></tr>
      <tr><td><input type="submit" value="Login"></td></tr>
    </tr>
    <tr><td></td><td>Default is root/uClinux</td>
    </table>
    </form>
    </html>
EOF
else
    testuser $user $pass
    if [ $? -eq 0 ]; then

	# login sucessful
	cat <<EOF
	<html>
	<head>
	<title>Easy Asterisk - Login</title>
	<meta http-equiv="REFRESH" content="0;url=http:dashboard.sh">
	<body>
	Please wait a few seconds.....
	</body>
	</head>
	</html>
EOF
    else
	# login failed
	cat <<EOF
	<html>
	<head>
	<title>Easy Asterisk - Login</title>
	<meta http-equiv="REFRESH" content="0;url=http:login.sh">
	<body>
	Please wait a few seconds.....
	</body>
	</head>
	</html>
EOF
    fi
fi

