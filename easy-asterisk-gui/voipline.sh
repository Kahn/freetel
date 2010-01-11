#!/bin/sh
# voipline.sh
# David Rowe 12 Jan 2010
# VOIP Line screen for Easy Asterisk GUI

# check we are logged in

echo $HTTP_COOKIE | grep "loggedin" > /dev/null
if [ $? -eq 1 ]; then
    echo "<html>"
    echo "<head>"
    echo '<meta http-equiv="REFRESH" content="0;url=http:login.sh">'
    echo "</head>"
    echo "</html>"
    exit
fi

# Construct the web page -------------------------------

cat <<EOF
<script src="prototype.js"></script>
<link href="astman.css" media="all" rel="Stylesheet" type="text/css" />
<script type="text/javascript" src="tooltip.js"></script>
<link rel="stylesheet" href="tooltip.css" type="text/css" />
EOF

cat << EOF
<html>
<title>Easy Asterisk - VOIP Line</title>
EOF

cat tooltips.html
echo '<table align="center" width=800>'
cat banner.html
echo "    <tr>"
cat menu.html    
cat <<EOF

    <td valign="top">

    <table align="right" width=600>
      <tr><td>&nbsp</td></tr>
      <form action="voipline.sh" method="get">
      <tr><td align="left" valign="top"><h2>VOIP Line</h2></td></tr>

      <tr><td>&nbsp</td></tr>
EOF
./voipline.pl

cat <<EOF
      <tr><td>&nbsp</td></tr>
      <tr><td><input type="submit" value="OK"></td></tr>

      </form>

    </table>
    </form>

    </td>

    </tr>

</table>
</body>
</html>
EOF

