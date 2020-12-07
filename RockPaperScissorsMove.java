/* This function in Java was created to make a move against an opponent for Rock Paper Scissors. I had it so that if the 
first two moves would be selected randomly. After that, I would check if the opponent is repeating itself, and if it is, 
I would return the opponent's move's weakness. If it isn't repeating itself, I would return its last move. */
public String move(List <String> myMoves, List <String> opponentMoves, int myScore, int opponentScore)
    {
        String[] moves = {"r", "p", "s"};
        if(myMoves.size() < 2){
            return moves[(int)(Math.random()*3)];
        }
        else if(opponentMoves.get(opponentMoves.size()-1).equals(opponentMoves.get(opponentMoves.size()-2))){
            if(opponentMoves.get(opponentMoves.size()-1).equals("r")){
                return "p";
            }
            else if(opponentMoves.get(opponentMoves.size()-1).equals("p")){
                return "s";
            }
            else{
                return "r";
            }
        }
        else{
            return opponentMoves.get(opponentMoves.size()-1);
        }
    }