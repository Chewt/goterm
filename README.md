# Goterm 
## A game of go/weiqi/baduk that can be played in the terminal.

# Features
Goterm currently has the following features:
 + Play a game against an engine
 + Play a game against another user over the network
 + Basic scoring at the end of the game
 + Different board sizes
 
Here is an example of how it looks in the terminal 
![Imgur](https://i.imgur.com/3MLY3fP.png)
 
# Install 
Install with `make install` and use `goterm --help` to see a list of command
line arguments goterm accepts.

You can also use the command `help` while running the program to see a list of available commands.

# Networking
To play against someone on the network, simple have on player run goterm with the command `goterm -h` to act as the host, and the other player use the command `goterm -c IPADDRESS` to connect to the host. You can also use the machine's hostname instead of the IP address if your network supports hostname resolution.

# Engines 
Goterm uses the Go Text Protocol to communicate with go engines. You
can specify a path to a compatible engine with `goterm -e PATH/TO/ENGINE`.
Optionally, if the engine supports command line arguments, you can add them by
surrounding the path and command line arguments in quotes like `goterm -e
"PATH/TO/ENGINE OPTIONS"`. 

For example, gnugo can be used as an engine by using the command `goterm -e "gnugo --mode gtp"`
