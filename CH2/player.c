
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdlib.h>
#include "player.h"

int getHandStrength5Cards(char * cards) {
	double average = 0;
	int sum = 0;
	int place = 0;
	for (int i = 8; i < 60; i++) {
		if (cards[place] == i) {
			place++;
		}
		else {
			cards[5] = i;
			sum += getHandStrength6Cards(cards);
		}
	}
	average = (double) sum / 46;
	if (average - (int) average > .5) {
		average++;
	}
	return( (int) average);
}

int getHandStrength6Cards(char * cards) {
	double average = 0;
	int sum = 0;
	int place = 0;
	char tempCards[7];
	for (int i = 8; i < 60; i++) {
		if (cards[place] == i || cards[6] == i) {
			place++;
		}
		else {
			for (int n = 0; n < 6; n++) {
				tempCards[n] = cards[n];
			}
			tempCards[6] = i;
			sortCardsLtoG(tempCards);
			sum += getHandStrength7Cards(tempCards, 0);
		}
	}
	average = (double) sum / 46;
	if (average - (int) average > .5) {
		average++;
	}
	return( (int) average);
}

int getHandStrength7Cards(char * cards, int makeHand) {
	int hand;
	if (makeHand) {
		hand = bestHand(cards, 1);
	}
	else {
		hand = bestHand(cards, 0);
	}
	if (hand < currentHand) {
		currentHand = hand;
	}
	if (hand > 7) {
		return(10);
	}
	else if (hand > 6) {
		return(9);
	}
	else if (hand > 5) {
		return(7);
	}
	else if (hand > 4) {
		return(6);
	}
	else if (hand > 3) {
		return(5);
	}
	else if (hand > 2) {
		return(3);
	}
	else if (hand > 1) {
		return(1);
	}
	else {
		return(0);
	}
}

int bestHand(char * cards, int makeHand) {
	if (checkRoyalFlush(cards, makeHand)) {
		return(10);
	}
	else if (checkStraightFlush(cards, makeHand)) {
		return(9);
	}
	else if(checkFourOfAKind(cards, makeHand)) {
		return(8);
	}
	else if(checkFullHouse(cards, makeHand)) {
		return(7);
	}
	else if(checkFlush(cards, makeHand)) {
		return(6);
	}
	else if(checkStraight(cards, makeHand)) {
		return(5);
	}
	else if(checkThreeOfAKind(cards, makeHand)) {
		return(4);
	}
	else if(checkTwoPair(cards, makeHand)) {
		return(3);
	}
	else if(checkPair(cards, makeHand)) {
		return(2);
	}
	else {
		return(1);
	}
}

int checkRoyalFlush(char * cards, int makeHand) {
	if (!checkStraightFlush(cards, 0)) {
		return(0);
	}
	if ((cards[6] / 4) != 14) {
		return(0);
	}
	int numTens = 0;
	int firstTen = 0;
	for (int i = 0; i < 7; i++) {
		if ((cards[i] / 4) == 10) {
			if (numTens == 0) {
				firstTen = i;
			}
			numTens++;
		}
	}
	if (numTens == 0) {
		return(0);
	}
	int current;
	int suit;
	int num;
	for (int i = 0; i < numTens; i++) {
		current = 10;
		suit = cards[firstTen+i] % 4;
		num = 1;
		for (int n = firstTen + i; n < 7; n++) {
			if (((cards[n] / 4) == (current + 1)) && ((cards[n] % 4) == suit)) {
				num++;
				current++;
			}
		}
		if (num > 4) {
			if (makeHand) {
				num = 0;
				current = 10;
				for (int n = firstTen + i; n < 7; n++) {
					if (((cards[n] / 4) == (current + 1)) && ((cards[n] % 4) == suit)) {
						current++;
						fiveCardHand[num] = cards[n];
						num++;
					}
				}
				sortCardsGtoL(fiveCardHand);
			}
			return(1);
		}
	}
	if (makeHand) {

	}
	return(0);
}

int checkStraightFlush(char * cards, int makeHand) {
	if (!(checkFlush(cards, 0) && checkStraight(cards, 0))) {
		return(0);
	}
	int num;
	int suit;
	int current;
	for (int i = 0; i < 3; i++) {
		current = cards[i] / 4;
		suit = cards[i] % 4;
		num = 1;
		for (int n = i + 1; n < 7; n++) {
			if (((cards[n] / 4) == (current + 1)) && ((cards[n] % 4) == suit)) {
				num++;
				current++;
			}
			if (num > 4) {
				if (makeHand) {
					num = 0;
					current = cards[i] / 4;
					for (int n = i + 1; n < 7; n++) {
						if (((cards[n] / 4) == (current + 1)) && ((cards[n] % 4) == suit)) {
							num++;
							current++;
							fiveCardHand[num] = cards[n];
							num++;
						}
					}
					sortCardsGtoL(fiveCardHand);
				}
				return(1);
			}
		}
	}
	return(0);
}

int checkFourOfAKind(char * cards, int makeHand) {
	int num;
	for (int i = 0; i < 6; i++) {
		num = 1;
		for (int n = i + 1; n < 7; n++) {
			if ((cards[i] / 4) == (cards[n] / 4)) {
				num++;
			}
		}
		if (num > 3) {
			if (makeHand) {
				for (int n = 0; n < 4; n++) {
					fiveCardHand[n] = ((cards[i] / 4) * 4) + n;
				}
				for (int n = 6; n > 0; n--) {
					if ((cards[n] / 4) != (cards[i] / 4)) {
						fiveCardHand[4] = cards[n];
						return(1);
					}
				}
			}
			return(1);
		}
	}
	return(0);
}

int checkFullHouse(char * cards, int makeHand) {
	if (checkTwoPair(cards, 0) && checkThreeOfAKind(cards, 0)) {
		if(makeHand) {
			int count;
			for (int i = 6; i > 1; i--) {
				count = 0;
				for (int n = i - 1; n >= 0; n--) {
					if ((cards[n] / 4) == (cards[i] / 4)) {
						count++;
					}
				}
				if (count > 2) {
					count = 0;
					for (int n = 0; n < 7; n++) {
						if ((cards[n] / 4) == (cards[i] / 4)) {
							fiveCardHand[count] = cards[n];
							count++;
						}
					}
					break;
				}
			}
			for (int i = 6; i > 0; i--) {
				if (((cards[i] / 4) == (cards[i-1] / 4)) && ((cards[i] / 4) != (fiveCardHand[0] / 4))) {
					fiveCardHand[3] = cards[i];
					fiveCardHand[4] = cards[i-1];
					return(1);
				}
			}
		}
		return(1);
	}
	return(0);
}

int checkFlush(char * cards, int makeHand) {
	int num;
	for (int i = 0; i < 4; i++) {
		num = 0;
		for (int n = 0; n < 7; n++) {
			if ((cards[n] % 4) == i) {
				num++;
			}
		}
		if (num > 4) {
			if (makeHand) {
				num = 0;
				for (int n = 6; n >= 0; n--) {
					if ((cards[n] % 4) == i) {
						fiveCardHand[num] = cards[n];
						num++;
						if (num > 4) {
							return(1);
						}
 					}
				}
			}
			return(1);
		}
	}
	return(0);
}

int checkStraight(char * cards, int makeHand) {
	int num = 1;
	for (int i = 0; i < 6; i++) {
		if (((cards[i] / 4) + 1) == (cards[i+1] / 4)) {
			num++;
		}
		else if ((cards[i] / 4) == (cards[i+1] / 4)) {}
		else {
			if (num > 4) {
				if (makeHand) {
					num = 1;
					fiveCardHand[0] = cards[i-1];
					for (int n = i - 2; n >= 0; n--) {
						if ((cards[n] / 4) == ((fiveCardHand[num-1] / 4) - 1)) {
							fiveCardHand[num] = cards[n];
							num++;
							if (num > 4) {
								return(1);
							}
						}
					}
				}
				return(1);
			}
			else if (i > 1) {
				return(0);
			}
			else {
				num = 1;
			}
		}
	}
	if (num > 4) {
		if (makeHand) {
			num = 1;
			fiveCardHand[0] = cards[6];
			for (int n = 6; n >= 0; n--) {
				if ((cards[n] / 4) == ((fiveCardHand[num-1] / 4) - 1)) {
					fiveCardHand[num] = cards[n];
					num++;
					if (num > 4) {
						return(1);
					}
				}
			}
		}
		return(1);
	}
	return(0);
}

int checkThreeOfAKind(char * cards, int makeHand) {
	int num;
	for (int i = 0; i < 6; i++) {
		num = 1;
		for (int n = i + 1; n < 7; n++) {
			if ((cards[i] / 4) == (cards[n] / 4)) {
				num++;
			}
		}
		if (num > 2) {
			if (makeHand) {
				num = 0;
				for (int n = 0; n < 7; n++) {
					if ((cards[n] / 4) == (cards[i] / 4)) {
						fiveCardHand[num] = cards[n];
						num++;
					}
				}
				for (int n = 6; n > 0; n++) {
					if ((cards[n] / 4) != (cards[i] / 4)) {
						fiveCardHand[num] = cards[n];
						num++;
						if (num > 4) {
							return(1);
						}
					}
				}
			}
			return(1);
		}
	}
	return(0);
}

int checkTwoPair(char * cards, int makeHand) {
	int pairs = 0;
	int pair1 = 0;
	for (int i = 6; i > 0; i--) {
		for (int n = i - 1; n >= 0; n--) {
			if ((cards[i] / 4) == (cards[n] / 4)) {
				pairs++;
				if (pairs > 1) {
					if (makeHand) {
						pairs = 0;
						for (int m = 6; m >= 0; m--) {
							if ((cards[m] / 4) == pair1) {
								fiveCardHand[pairs] = cards[m];
								pairs++;
							}
							if (pairs > 1) {
								break;
							}
						}
						for (int m = 4; m >= 0; m--) {
							if ((cards[m] / 4) == (cards[i] / 4)) {
								fiveCardHand[pairs] = cards[m];
								pairs++;
							}
							if (pairs > 3) {
								break;
							}
						}
						for (int m = 6; m >= 0; m--) {
							if (((cards[m] / 4) != (cards[i] / 4)) && ((cards[m] / 4) != pair1)) {
								fiveCardHand[4] = cards[m];
								return(1);
							}
						}
					}
					return(1);
				}
				pair1 = cards[i] / 4;
				while ((cards[i] / 4) == (cards[n] / 4) && n >= 0) {
					i--;
					n--;
				}
				break;
			}
		}
	}
	return(0);
}

int checkPair(char * cards, int makeHand) {
	for (int i = 6; i > 0; i--) {
		for (int n = i - 1; n >= 0; n--) {
			if (cards[i] / 4 == cards[n] / 4) {
				if (makeHand) {
					int place = 2;
					fiveCardHand[0] = cards[i];
					fiveCardHand[0] = cards[n];
					for (int m = 6; m >= 0; m++) {
						if ((cards[m] / 4) != (cards[i] / 4)) {
							fiveCardHand[place] = cards[m];
						}
						if (place > 4) {
							return(1);
						}
					}
				}
				return(1);
			}
		}
	}
	return(0);
}

void sortCardsLtoG(char * cards) {
	int temp;
	int place;
	for (int i = 0; i < 6; i++) {
		temp = cards[i];
		place = i;
		for (int n = i; n < 7; n++) {
			if (cards[n] < cards[i]) {
				cards[i] = cards[n];
				place = n;
			}
		}
		cards[place] = temp;
	}
}

void sortCardsGtoL(char * cards) {
	int temp;
	int place;
	for (int i = 0; i < 6; i++) {
		temp = cards[i];
		place = i;
		for (int n = i; n < 7; n++) {
			if (cards[n] > cards[i]) {
				cards[i] = cards[n];
				place = n;
			}
		}
		cards[place] = temp;
	}
}
