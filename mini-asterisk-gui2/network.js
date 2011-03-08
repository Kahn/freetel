/*
  network.js
  David Rowe 
  March 7 2011

  Network screen for Mini Asterisk GUI.
*/

var dhcp;

// Called when we load page

function initialise() {
    mainMenu();

    // kick off cascading CGIs, initialisePage() will be called when complete

    // determine if DHCP or static

    downloadUrl("/cgi-bin/getdhcp.cgi", processDhcp);
}


function greyout(dhcp) {

    if (dhcp == "yes") {
	document.network.static.checked = 0;
	document.network.dhcp.checked = 1;
	document.network.ipaddress.disabled = 1;
	document.network.netmask.disabled = 1;
	document.network.gateway.disabled = 1;
	document.network.dns.disabled = 1;
    }
    else {
	document.network.static.checked = 1;
	document.network.dhcp.checked = 0;
	document.network.ipaddress.disabled = 0;
	document.network.netmask.disabled = 0;
	document.network.gateway.disabled = 0;
	document.network.dns.disabled = 0;
    }
}

// called when DHCP CGI returns

function processDhcp(doc,status) {
    dhcp = "no";

    loadHtmlTextFile(doc, function(line) {
	    parseNetwork(line);
	}
	);

    greyout(dhcp);

    if (dhcp == "no") {
        downloadUrl("/cgi-bin/getconf.cgi?file=../init.d/network-static", processStatic);
    }
}


// called when static IP CGI returns

function processStatic(doc,status) {
    loadHtmlTextFile(doc, function(line) {
	    parseNetwork(line);
	}
	);
        
    downloadUrl("/cgi-bin/getemergencyip.cgi", processEmergencyIp);
}

// called when Emergency CGI returns

function processEmergencyIp(doc,status) {
    loadHtmlTextFile(doc, function(line) {
	    parseNetwork(line);
	}
	);
        
    // fire off fping CGI

    downloadUrl("/cgi-bin/fpingnodes.cgi?ip=google.com", processFping);
}

function parseNetwork(line) {

    if (line.indexOf("DHCP=") != -1) {
      var s = line.split('"');
      dhcp = s[1];
    }

    if (line.indexOf("IPADDRESS=") != -1) {
      var s = line.split('"');
      var ipaddress = s[1];
      document.network.ipaddress.value = ipaddress;
    }

    if (line.indexOf("NETMASK=") != -1) {
      var s = line.split('"');
      var netmask = s[1];
      document.network.netmask.value = netmask;
    }

    if (line.indexOf("GATEWAY=") != -1) {
      var s = line.split('"');
      var gateway = s[1];
      document.network.gateway.value = gateway;
    }

    if (line.indexOf("DNS=") != -1) {
      var s = line.split('"');
      var dns = s[1];
      document.network.dns.value = dns;
   }

   if (line.indexOf("BACKDOOR=") != -1) {
      var s = line.split('"');
      var backdoor = s[1];
      document.network.backdoor.value = backdoor;
   }
}


// http://moblog.bradleyit.com/2009/06/javascript-ip-address-validation.html

function isIP(obj) {
    var ary = obj.value.split(".");
    var ip = true;

    ip = (ary.length == 4); 

    if (ip) {
	for (var i=0; i<4; i++) { 
		ip = (!ary[i].match(/^\d{1,3}$/) || (Number(ary[i]) > 255)) ? false : ip; 
    	} 
    }

    if (!ip) {    
	// the value is NOT a valid IP address
	obj.style.background = "red";
	obj.select();
    } 
    else { obj.style.background = ""; } // the value IS a valid IP address

    return ip;
}
