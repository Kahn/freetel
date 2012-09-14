#!/bin/sh
# network-static.cgi
# Ian Roberts 15 Sept 2012
#
# CGI to return the output from an /etc/init.d/network-static command 
#
# valid action are start, stop, enable, disable
#
cat <<EOF
Content-type: text/html

<html>
<head>
<meta http-equiv="cache-control" content="no-cache">
<meta http-equiv="pragma" content="no-cache">
<meta http-equiv="expires" content="-1">
</head>
<body>
EOF

ARG=`echo "$QUERY_STRING" | sed -n "s/.*action=//p" | sed -n "s/%20/ /pg"`
echo "$ARG"
/etc/init.d/network-static "$ARG"
cat <<EOF
</body>
</html>
EOF
