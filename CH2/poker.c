/*
 * poker.c
 *
 *  Created on: Apr 26, 2019
 *      Author: cunnin66
 */

/*
 * poker.c
 *
 *  Created on: Mar 30, 2019
 *      Author: James
 */

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdlib.h>
#include "poker.h"
#include "LoadCell.h"
#include "rfid.h"
#include "xbee.h"
#include "home.h"

uint8_t BROADCAST = 0;

void playGame(void) {
	while (games.numPlayers > 1) {
		dealHands();
		bet(0,0);
		dealToTable(3);
		bet(0,0);
		dealToTable(1);
		bet(0,0);
		dealToTable(1);
		bet(0,0);
		resolveHand();
	}
}

void initGame(int num_players, int init_bank) {
	games.numPlayers = num_players;
	games.activePlayers = num_players;

	games.pot = 0;
	for (int i = 0; i < num_players; i++) {
		games.players[i].currentBet = 0;
		games.players[i].gameStatus = 1;

		//games.players[i].holdings = getStartHoldings();
		games.players[i].holdings = init_bank;
		games.players[i].address = 1 << i;
		BROADCAST |= 1<<i; //dictate who all is in


	}

	roundReset();
	games.dealer = 0;
	//set all to active
	int status = 1;
	xbee_send(BROADCAST, 4, &status, 1);
	//send init bank info to all
	nano_wait(500*1000*1000);
	xbee_send(BROADCAST, 9, &init_bank, 1);

}

void roundReset(void) {
	for (int i = 0; i < 4; i++) {
		games.players[i].currentBet = 0;
		games.players[i].handStrength = 0;
		//hand resets?
		if (games.players[i].gameStatus > -1) {
			games.players[i].gameStatus = 1;
		}
	}
	games.pot = 0;
	games.activePlayers = games.numPlayers;
	games.dealer = (games.dealer + 1) % games.numPlayers; //advance dealer
}

int getStartHoldings(void) {
	return(50);
}

void dealHands() {
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < games.activePlayers; j++) { // check active players
			//Brian-- you'll see me use this mod thing
			//its to ensure we start the deal from the left of the dealer
			led(3,0);
			int index = (games.dealer + 1 + j)%games.numPlayers;
			if (games.players[index].gameStatus > 0) {
				// Turn on LED
				int new_card = getCard();
				led(3,1);
				choice_update(1); //advance screen
				xbee_send(games.players[index].address, 5, &new_card, 1);
				nano_wait(1000*1000*1000);
			}
		}
	}
}

void dealToTable(char numCards) {
	for (int i = 0; i < numCards; i++) {
		int new_card = getCard();
		//send to everybody
		xbee_send(BROADCAST,5, &new_card, 1);
	}
}

void bet(int start, int playerTurn) {
	for (int i = 0; i < 4; i++) {
		if (games.activePlayers > 1) {
			if (++playerTurn > 3) {
				playerTurn = 0;
			}
			if (games.players[playerTurn].gameStatus == 1) {
				games.players[playerTurn].currentBet = getBet();
				games.players[playerTurn].holdings -= games.players[playerTurn].currentBet;
				if (games.players[playerTurn].currentBet > start) {
					bet(games.players[playerTurn].currentBet, playerTurn);
					return;
				}
				if (games.players[playerTurn].currentBet <= start) {
					if (games.players[playerTurn].holdings == 0) {
						games.players[playerTurn].gameStatus = 2;
					}
					else {
						games.players[playerTurn].gameStatus = 0;
						games.activePlayers--;
					}
				}

			}
		}
	}
}

int getBet(void) {
	return(weigh(0));
}

void resolveHand() {
	int winner;
	int ties[5];				// ties[0] represents the number of ties, and ties[1-4] represent a player involved in the tie
	while (games.pot > 0) {
		winner = 0;
		ties[0] = 0;
		for (int i = 0; i < 3; i++) {
			ties[i] = 4;
		}
		for (int i = 0; i < 4; i++) {
			if (winner > 3) {
				break;
			}
			if (games.players[i].gameStatus > 0) {
				if (games.players[i].handStrength > games.players[winner].handStrength) {
					winner = i;
				}
				else if (games.players[i].handStrength == games.players[winner].handStrength) {
					winner = tieBreaker(winner, i, ties);
				}
			}
		}
		if (ties[0] == 0) {
			for (int i = 0; i < 4; i++) {
				if (i != winner) {
					if (games.players[i].currentBet < games.players[winner].currentBet) {
						games.players[winner].holdings += games.players[i].currentBet;
						games.pot -= games.players[i].currentBet;
						games.players[i].currentBet = 0;
					}
					else {
						games.players[winner].holdings += games.players[i].currentBet;
						games.pot -= games.players[i].currentBet;
						games.players[i].currentBet -= games.players[winner].currentBet;
					}
				}
			}
			games.players[winner].currentBet = 0;
			games.players[winner].gameStatus = 0;
		}
		else {
			for (int n = 1; n < ties[0]; n++) {
				for (int i = 0; i < 4; i++) {
					if (i != ties[1] && i != ties[2] && i != ties[3] && i != ties[4]) {
						if (games.players[i].currentBet < games.players[ties[n]].currentBet) {
							games.players[ties[n]].holdings += games.players[i].currentBet;
							games.pot -= games.players[i].currentBet;
							games.players[ties[n]].currentBet = 0;
						}
						else {
							games.players[ties[n]].holdings += games.players[i].currentBet;
							games.pot -= games.players[i].currentBet;
							games.players[i].currentBet -= games.players[ties[n]].currentBet;
						}
					}
				}
				games.players[ties[n]].currentBet = 0;
				games.players[ties[n]].gameStatus = 0;
			}
		}
	}
	return;
}


int tieBreaker(int player1, int player2, int * ties) {
	if (games.players[player1].handStrength == 1) {
		for (int i = 0; i < 5; i++) {
			if (games.players[player1].hand[i] > games.players[player2].hand[i]) {
				return(player1);
			}
			else if (games.players[player2].hand[i] > games.players[player1].hand[i]) {
				return(player2);
			}
		}
		tieHandler(player1, player2, ties);
		return(player1);
	}
	else if (games.players[player1].handStrength == 2) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			for (int i = 2; i < 5; i++) {
				if (games.players[player1].hand[i] > games.players[player2].hand[i]) {
					return(player1);
				}
				else if (games.players[player2].hand[i] > games.players[player1].hand[i]) {
					return(player2);
				}
			}
			tieHandler(player1, player2, ties);
			return(player1);
		}
	}
	else if (games.players[player1].handStrength == 3) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			if (games.players[player1].hand[2] > games.players[player2].hand[2]) {
				return(player1);
			}
			else if (games.players[player2].hand[2] > games.players[player1].hand[2]) {
				return(player2);
			}
			else {
				if (games.players[player1].hand[5] > games.players[player2].hand[5]) {
					return(player1);
				}
				else if (games.players[player2].hand[5] > games.players[player1].hand[5]) {
					return(player2);
				}
				tieHandler(player1, player2, ties);
				return(player1);
			}
		}
	}
	else if (games.players[player1].handStrength == 4) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			for (int i = 3; i < 5; i++) {
				if (games.players[player1].hand[i] > games.players[player2].hand[i]) {
					return(player1);
				}
				else if (games.players[player2].hand[i] > games.players[player1].hand[i]) {
					return(player2);
				}
			}
			tieHandler(player1, player2, ties);
			return(player1);
		}
	}
	else if (games.players[player1].handStrength == 5) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		tieHandler(player1, player2, ties);
		return(player1);
	}
	else if (games.players[player1].handStrength == 6) {
		for (int i = 0; i < 5; i++) {
			if (games.players[player1].hand[i] > games.players[player2].hand[i]) {
				return(player1);
			}
			else if (games.players[player2].hand[i] > games.players[player1].hand[i]) {
				return(player2);
			}
		}
		tieHandler(player1, player2, ties);
		return(player1);
	}
	else if (games.players[player1].handStrength == 7) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			if (games.players[player1].hand[3] > games.players[player2].hand[3]) {
				return(player1);
			}
			else if (games.players[player2].hand[3] > games.players[player1].hand[3]) {
				return(player2);
			}
			tieHandler(player1, player2, ties);
			return(player1);
		}
	}
	else if (games.players[player1].handStrength == 8) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		else {
			if (games.players[player1].hand[4] > games.players[player2].hand[4]) {
				return(player1);
			}
			else if (games.players[player2].hand[4] > games.players[player1].hand[4]) {
				return(player2);
			}
			tieHandler(player1, player2, ties);
			return(player1);
		}
	}
	else if (games.players[player1].handStrength == 9) {
		if (games.players[player1].hand[0] > games.players[player2].hand[0]) {
			return(player1);
		}
		else if (games.players[player2].hand[0] > games.players[player1].hand[0]) {
			return(player2);
		}
		tieHandler(player1, player2, ties);
		return(player1);
	}
	else if (games.players[player1].handStrength == 10) {
		tieHandler(player1, player2, ties);
		return(player1);
	}
	return(player1);
}

void tieHandler(int player1, int player2, int * ties) {
	if (ties[0] == 0) {
		ties[0] = 2;
		ties[1] = player1;
		ties[2] = player2;
	}
	else {
		ties[ties[0]] = player2;
		ties[0]++;
	}
}
