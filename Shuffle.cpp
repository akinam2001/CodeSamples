/* This function in C++ uses my implemented List ADT to perfectly shuffle a deck of cards.
It takes two halves of the deck and combines it so the elements of the 2 halves are alternating.
It edits the original list to hold the perfectly shuffled list. */

#include <string>
#include <iostream>
#include <iomanip>
#include "List.h"

void shuffle(List& D){
    //lists for the 2 halves
    List firstHalf = List();
    List secondHalf = List();
    //loop through the deck and insert half into first half and then second half
    D.moveFront();
    int n = D.size();
    int x;
    for(x = 0; x < (n/2); x++){
        firstHalf.insertAfter(D.peekNext());
        D.moveNext();
        firstHalf.moveNext();
    }
    for(int y = 0; y < (n-x); y++){
        secondHalf.insertAfter(D.peekNext());
        D.moveNext();
        secondHalf.moveNext();
    }
    //clear D to change order
    D.clear();
    //iterate through halves and add from each list 
    firstHalf.moveFront();
    secondHalf.moveFront();
    for(int y = 0; y < n; y++){
        if(y % 2 == 0){
            D.insertAfter(secondHalf.peekNext());
            D.moveNext();
            secondHalf.moveNext();
            continue;
        }
        D.insertAfter(firstHalf.peekNext());
        D.moveNext();
        firstHalf.moveNext();
    }
}