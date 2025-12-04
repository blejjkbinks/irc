ALLOWED COMMANDS

Everything in C++ 98.
socket, close, setsockopt, getsockname,
getprotobyname, gethostbyname, getaddrinfo,
freeaddrinfo, bind, connect, listen, accept,
htons, htonl, ntohs, ntohl, inet_addr, inet_ntoa,
inet_ntop, send, recv, signal, sigaction,
sigemptyset, sigfillset, sigaddset, sigdelset,
sigismember, lseek, fstat, fcntl, poll (or
equivalent)


FEATURES TO IMPLEMENT

auth, set username, set nickname, join channel, send/receive private messages
user privileges (operator/regular)
commands (operator): kick, invite, topic(change or view)
mode: (i: invite only, t: change topic permission, k: set/change/remove channel passowrd, o: give/take privilege, l: channel user limit)
mode i (yes/no, 1/0), optionnal channel name? or affects current channel
mode t : same(yes/no) same(arg or current)
mode k : newpassword for set, empty for remove OR just "mode k" and then prompt user to enter password and then confirm
mode o : user_to_change, (yes/no)
mode l : n to set limit, empty to remove


each channel has:
	- list of users
	- invite only mode
	- topic can only be changed by operator mode
	- a password (can be empty)
	- a limit (can be empty)

each user has:
	- a username
	- a nickname

create channel???
where to keep track of privilege? each user has a list of channel he is operator in? or each channel has a list of user that are operators in it?
what happens if a user leaves a channel? keep track of privilege or lose it?
who can become operator? who can create a new channel? delete channel? channel-wide privilege vs server-wide privilege

each channel keeps track of active users connected, echo all received lines to all users in that channel


3 classes: server, channel, user (??)

handle encryption, password safety?
handle internet connection or just local on 1 machine?
difference between some client and a bare terminal doing 'nc'?

- ft_split received line
- if first word starts with '/' then process command
- otherwise just send line to channel

use dynamic arrays, make our own ft_realloc in cpp?



focus:

server has an array of channels and an array of clients
test adding and removing channels
test connecting and disconnecting clients
server terminal reads its stdin for debug requests

lookup channel and clients?



next steps:

force user to auth, username and nick cant be empty
implement command logic

figure out irc standards,
format strings sent back to client to make it readable for legacy clients



handle pingpong
send periodically, respond when questionned
nc says broken pipe when server quits first
password checker is too crude, only works for nc but needs rethinking for irssi
numerics 001-004 for connection
332
353
366

notice
join, part, quit
privmsg
cap ls/ cap end??

https://datatracker.ietf.org/doc/html/rfc2812
https://datatracker.ietf.org/doc/html/rfc1459
https://defs.ircdocs.horse/defs/numerics.html
https://www.rfc-editor.org/rfc/rfc2812.txt


known issues, to fix:

sending back empty/incomplete cap ls response

never sending PASS password

random JOIN : at the start

check how NICK and USER work really, default config username clash?

PING PONG just echoed, fixed by itself??? lucky

server needs to send ERROR when quitting
sigint vs quit commend?

hang and flush????? IMPORTANT

handshake all done, implement actual features


===========

MARDI, issues to fix:

[x] "/user myname" send [userhost myname] instead of [USER myname], need to change command parsing

[x] forbidden fcntl function call with the flags, need remove

[ ] add MODE feedback/debug

[ ] testing checklist


can ignore (?):

[ ] random [JOIN :] and [MODE usernname +i] during connection, how to handle? just ignore and send back errors?

[x] [PONG ircserv ircserv] instead of [PONG ircserv], doesnt seem to matter anyways??

[ ] [MODE #channel b], explicitly ignore

[ ] after changing nickname, sent messages still display old nickname on sender's client

[ ] all logic in one(1) single cpp file, paris kids get mad??

[ ] after server "quit"s, irssi seems stuck, not responding to "/help", maybe needs specific signals to register server shutdown
	similarly, nc terminal on ubuntu sometimes shows "broken pipe", not good


MERCREDI, issues to fix:

[ ] Server.cpp, line:179, use of ERRNO after recv() call, eval sheet says not to. MARK 0!!!!
	important because it's used to detect disconnect rn

[ ] review implementation of kick + invite + topic + mode

[ ] KICK

[ ] INVITE

[ ] TOPIC

[ ] MODE

[ ] replicate ctrl+D test from subject??

[ ] NOTICE command

[ ] include algorithm and vector in Channel.hpp, allowed???

[ ] duplicate nicks not allowed, used to id a single client. nick command needs to check for duplicates and give error if relevant