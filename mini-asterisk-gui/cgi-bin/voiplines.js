function changeProvider() {
	provider = $('provider').value;
	$('user').value = users[provider];
	$('pass').value = passwords[provider];
	$('host').value = hosts[provider];
        $('comment').innerHTML = comments[provider];
}
