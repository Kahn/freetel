#!/usr/bin/speedy -w
# -*-perl-*-

use lib qw(/usr/share/perl5/smokeping);
use CGI::Carp qw(fatalsToBrowser);

use Smokeping 2.003006;

Smokeping::cgi("/etc/smokeping/config");

=head1 NAME

smokeping.cgi - SmokePing webfrontend

=head1 OVERVIEW

This script acts as a 'website' for your SmokePing monitoring operation. It
presents the targets you are looking at in a tree structure and draws graphs
as they are required by people looking at the pages.

=head1 DESCRIPTION

To get B<smokeping.cgi> going, you need a webserver which allows you to run
cgi scripts. The system must be setup so that the B<smokeping.cgi> is
allowed to write to the image caching area as defined in the config file.

This script runs with normal perl. B<BUT> it will appear to be very slow,
because it does a lot of things when starting up. So if the script has to be
started a fresh on every click, this is both slow and a tough thing for your
webserver. I therefore strongly recomment using SpeedyCGI.

Please refer to the installation document for detailed setup instructions.

=head1 SETUP

When installing SmokePing, this file has to be adjusted to fit your
local system. Three paths have to be entered.

 use lib qw(/usr/pack/rrdtool-1.0.33-to/lib/perl);

One pointing to your B<rrdtool> installation

 use lib qw(/home/oetiker/public_html/smokeping/lib);

One pointing to the place where you have installed the SmokePing libraries

 use Smokeping;

 Smokeping::cgi("/home/oetiker/.smokeping/config");

The third path is the argument to the Smokeping::cgi command. It points to
the SmokePing configuration file.

=head1 COPYRIGHT

Copyright (c) 2001 by Tobias Oetiker. All right reserved.

=head1 LICENSE

This program is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the GNU General Public License for more
details.

You should have received a copy of the GNU General Public
License along with this program; if not, write to the Free
Software Foundation, Inc., 675 Mass Ave, Cambridge, MA
02139, USA.

=head1 AUTHOR

Tobias Oetiker E<lt>tobi@oetiker.chE<gt>

=cut
