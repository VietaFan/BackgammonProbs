# backgammon-probs

This project contains code to estimate the probability that a player playing optimally will be able to bear off all checkers from a given state in the game Backgammon in a certain number of moves, after the game has progressed to the stage where it is no longer possible for the players to hit their opponents' checkers. It computes these probabilities using dynamic programming for each state in which all 15 checkers are in the nearer half of the board, then writing the data to a file in a compressed format. The project also contains the code for an interactive website to allow users to search this data, implemented in Python using the webapp2 framework and Google App Engine.

See http://gammon-probs.appspot.com.

