# backgammon-probs

This project contains code to estimate the probability that a player playing optimally will be able to bear off all checkers from a given state in the game Backgammon in a certain number of moves, after the game has progressed to the stage where it is no longer possible for the players to hit their opponents' checkers. It computes these probabilities using dynamic programming for each state in which all 15 checkers are in the nearer half of the board, then writing the data to a file in a compressed format. The project also contains the code for an interactive website to allow users to search this data, implemented in Python using the webapp2 framework and Google App Engine.

To find the probability distribution of the number of moves required tp bear off all checkers from a specified game state, assuming a strategy that minimizes the expected value of the number of moves required is used, see http://gammon-probs.appspot.com.

To see the top 10 moves from a specified position with a specified roll that minimize the expected number of moves to bear off all checkers, go to http://gammon-probs.appspot.com/topmoves.

