# Goterm A game of go that can be played in the terminal.

# Usage Use `goterm --help` to see a list of command line arguments goterm
accepts.

# Engines Goterm uses the Go Text Protocol to communicate with go engines. You
can specify a path to a compatible engine with `goterm -e PATH/TO/ENGINE`.
Optionally, if the engine supports command line arguments, you can add them by
surrounding the path and command line arguments in quotes like `goterm -e
"PATH/TO/ENGINE OPTIONS"`. 

For example, gnugo can be used as an engine by using the command `goterm -e "gnugo --mode gtp"`
