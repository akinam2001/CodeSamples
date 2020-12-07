/* This function in C is part of my implemented Big Integer ADT.
It multiplies two big integers together in the base given by BASE, which is set to 10^9.
It places the product into BigInteger P. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "BigInteger.h"
#include "List.h"

#define POWER 9
#define BASE pow(10, POWER)

// multiply()
// Places the product of A and B in the existing BigInteger P, overwriting
// its current state: P = A*B
void multiply(BigInteger P, BigInteger A, BigInteger B){
    //checks if the given Big Integer A is null
    if(A == NULL){
        fprintf(stderr, "Big Integer Error: calling multiply() on NULL Big Integer A reference\n");
        exit(EXIT_FAILURE);
    }
    //checks if the given Big Integer B is null
    if(B == NULL){
        fprintf(stderr, "Big Integer Error: calling multiply() on NULL Big Integer B reference\n");
        exit(EXIT_FAILURE);
    }
    
    //checks if P is null and if so, creates a new big integer
    if(P == NULL){
        P = newBigInteger();
    }
    //clears P so it can have the product
    makeZero(P);

    //do nothing if either BigInteger is a 0
    if(equals(A, P) || equals(B, P)){
        return;
    }

    //deals with sign of the product
    if(A->sign == B->sign){
        P->sign = 1;
    }
    else {
        P->sign = -1;
    }

    //creates copies of the big integers so the cursor element of the originals is unaffected
    BigInteger copyA = copy(A);
    BigInteger copyB = copy(B);

    moveBack(copyB->list);
    moveBack(copyA->list);
    //used to hold the carry over and what we are appending into the list
    long carry = 0;
    long appending = 0;
    //iterates through big integer B
    int x;
    for(x = 0; x < length(copyB->list); x++){
        //places the cursor of P into the right decimal placed based in how far we are in B
        if(length(P->list) > 0){
            moveBack(P->list);
            int y;
            for(y = 0; y < x; y++){
                movePrev(P->list);
            }
        }
        //iterates through big integer A
        int z;
        for(z = 0; z < length(copyA->list); z++){
            //multiples the corresponding decimal places of A and B and adds the carry
            long product = (get(copyA->list) * get(copyB->list)) + carry;
            //the carry is the most significant digits that are past the BASE
            carry = product/((long)(BASE));
            //appending is what we add to P and are the least significant digits
            appending = product % ((long)(BASE));
            //checks if cursor of P is undefined
            if(index(P->list) == -1){
                prepend(P->list, appending);
                moveFront(P->list);
            //checks what is already in P in the digits place we are on before adding new product into P
            }else{
                long sum = get(P->list) + appending;
                long newValue = sum % ((long)(BASE));
                carry += sum/((long)(BASE));
                set(P->list, newValue);
            }
            //move on to next digit
            movePrev(P->list);
            movePrev(copyA->list);
        }
        //once we are done going through A, checks if there is still any carry left
        if(carry != 0){
            prepend(P->list, carry);
        }
        carry = 0;
        //reset A's cursor and iterate B's cursor
        moveBack(copyA->list);
        movePrev(copyB->list);
    }
    //once we are done going through B, checks if there is still any carry left
    if(carry != 0){
        prepend(P->list, carry);
    }
    //frees memory space made by copies of A and B
    freeBigInteger(&copyA);
    freeBigInteger(&copyB);
}