/*
  menu.js
  David Rowe 
  Jan 4 2010

  Function(s) to create main menu.  Interesting to compare this to static HTML.
*/

var menuItems = [
    '<a href="phones.html">Phone System</a>',
    '<a href="about.html">About</a>',
    '<a href="system.html">System Info</a>',
    '<a href="admin.sh">Admin</a>',
    '<a href="faq.sh">FAQ</a>',
    '<a href="ipphones.sh">IP Phones</a>',
    '<a href="logout.sh">Logout</a>',
    '<a href="network.html">Network</a>',
    '<a href="voiplines.sh">VOIP Line</a>'
];

var menuTips = [
    'phones_tip',
    'about_tip',
    'system_tip',
    'admin_tip',
    'faq_tip',
    'ipphones_tip',
    'logout_tip',
    'network_tip',
    'voipline_tip'
];

var menuToolTips = [
    '<div id="phones_tip" class="tip">Lists your phones and phone lines</div>',
    '<div id="about_tip" class="tip">Software version and links to web site</div>',
    '<div id="system_tip" class="tip">Cpu, memory, loadavg info , linux version and installed packages</div>',
    '<div id="admin_tip" class="tip">Change your password, reset the default settings, upgrade software</div>',
    '<div id="faq_tip" class="tip">Frequently asked questions and links to further information</div>',
    '<div id="ipphones_tip" class="tip">Set up your IP phones</div>',
    '<div id="logout_tip" class="tip">I think you can work this one out....</div>',
    '<div id="network_tip" class="tip">Connect the phone system to your network and the Internet</div>',
    '<div id="voipline_tip" class="tip">Set up your VOIP phone line</div>'
];

function mainMenu() {
    var html = '';

    for(var i=0; i<menuToolTips.length; i++) {
	html += menuToolTips[i] + "\n";
    }

    html += '<td valign="top" >';
    html += '<table align="right" width=200>';
    html += '<tr><td><h2>&nbsp;</h2></td></tr>';
 
    for(var i=0; i<menuItems.length; i++) {
	var tooltip  = 'onMouseOver="popUp(event,' + "'" + menuTips[i] + "'" + ')" ';
	    tooltip += 'onMouseOut="popUp(event,' + "'" + menuTips[i] + "'" + ')"';

	html += '<tr ' + tooltip + '><td>' + menuItems[i] + '</td></tr>' + "\n";
    }

    html += '</table>';
				
    document.getElementById('menu').innerHTML = html;
}
