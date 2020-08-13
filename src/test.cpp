#include <iostream>

#include "types.h"
#include "position.h"

int main() {
    Position pos;
    pos.set({ }, { "3k/4/4/KN2 w" });

    // the copies are necessary, otherwise we end up with
    // 3 distinct shared_ptrs that all own the same board. This is a problem, because
    // they don't internally know about each other, and then it will get deleted 3 times.
    // this is one of those reasons to follow stockfish's one-position model, but
    // that's something that can be changed later.
    Board2D& board = pos.timeline(0).board_on_turn(1, WHITE);
    ((Timeline&)pos.timeline(0)).append_board(*(new Board2D(board)));
    ((Timeline&)pos.timeline(0)).append_board(*(new Board2D(board)));

    Board2D& new_board = pos.new_timeline(0, 1);
    new_board.remove_piece(SQ_D4);
    new_board.put_piece(B_KING, SQ_C4);

    std::cout << pos << std::endl;
}
