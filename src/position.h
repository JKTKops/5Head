#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include <vector>
#include <deque>
#include <memory> // shared ptrs

#include "types.h"

// 5D Chess does not have draw-by-repetition rules to keep track of.

// This class is somewhat like Stockfish 11's 'Position' class
class Board2D {
public:
    Board2D() = default;
    Board2D(const Board2D&) = default;
    Board2D& operator=(const Board2D&) = delete;

    // Board input/output via FEN
    Board2D& set(const std::string& fenStr);
    const std::string fen() const;

    char board_width() const;

    // Position representation
    Piece piece_on(Square2D s) const;
    bool empty(Square2D s) const;
    //template<PieceType pt> int count(Color c) const;
    //template<PieceType pt> int count() const;
    // TODO: maybe we want this one to be a vector so we can bundle
    // it up with move information from other dimensions more easily.
    // probably not.
    template<PieceType pt> const Square2D* squares(Color c) const;

    Color side_to_move() const;

    // Placing and removing pieces onto a 2D board
    void put_piece(Piece pc, Square2D s);
    void remove_piece(Square2D s);

    friend class Position;
private:
    void passTurn();
    // Data members

    // 5D chess supports smaller board sizes. It's easier to work
    // with full size boards and "cut them off" than to try and
    // have a Square2D type that is size-aware.
    // This must be <= 8.
    char boardWidth;

    Piece board[SQUARE_NB];

    int pieceCount[PIECE_NB];
    Square2D pieceList[PIECE_NB][16];
    // index of the piece on this square in the piece list
    int index[SQUARE_NB];

    Color sideToMove;

    // Keeping it simple for now. Full version will require
    // information about castling rights. Optimizations will
    // probably want to include some kind of bitboards, at
    // least to speed up the traditional chess aspects.
};

extern std::ostream& operator<<(std::ostream& os, const Board2D& pos);

typedef int Time;
typedef int L;

class Timeline {
public:
    Timeline(Time startTime, Color startColor);
    Timeline(const Timeline&) = default;
    Timeline& operator=(const Timeline&) = delete;

    Time start_time() const;
    Color start_color() const;
    bool is_active() const;
    void activate();

    // quick access
    const Board2D& first_board() const;
    const Board2D& last_board() const;
    // fine access
    bool has_board_on_turn(Time time, Color c) const;
    Board2D& board_on_turn(Time time, Color c) const;

    void append_board(Board2D& newBoard);

    Timeline& set_print_indented(bool pi);

    friend std::ostream& operator<<(std::ostream& os, const Timeline& line);
private:
    /// returns -1 if this timeline starts after the given ply.
    int plyToBoardIdx(Time time, Color c) const;

    Time startTime;
    Color startColor;

    bool active = false;

    // Stockfish's search model acts over one position, doing and un-doing moves
    // and storing the results in a hash table. If we were to follow that model,
    // then Board2Ds would never be duplicated and we should drop the shared_ptr.
    std::vector<std::shared_ptr<Board2D>> boards;

    // used for output
    bool printIndented = true;
};


class Position {
public:
    Position() = default;
    Position(const Position&) = delete;
    Position& operator=(const Position&) = delete;

    /// negative FENs should be passed top-down, that is, with the maximum
    /// absolute-value timeline first.
    /// The central timeline should be in positiveLines[0].
    void set(std::vector<std::string> negativeFENs, std::vector<std::string> positiveFENs);

    L negative_timeline_count() const;
    L positive_timeline_count() const;
    // Doesn't check if the timeline exists, which can lead to crashes!
    const Timeline& timeline(L timeline) const;

    Color side_to_move() const;
    Time  time_of_present() const;

    /// Coordinates are of the board which should be copied. Time should
    /// be the in-game T coordinate, this function will identify the correct ply
    /// based on side_to_move.
    /// Both of these functions correctly adjust the position's active timeline
    /// bookkeeping.
    /// The new board will have the appropriate side-to-move for its coordinates,
    /// but will still need to be modified to complete the move.
    Board2D& new_timeline(L branchLine, Time branchTime);
private:
    // Not currently supporting 2 central timelines.
    std::vector<Timeline> negativeLines;
    // the central timeline is positiveLines[0]
    std::vector<Timeline> positiveLines;

    // doesn't include 0
    short activePositiveLines;
    short activeNegativeLines;

    Time timeOfPresent;
    Color sideToMove;
};

extern std::ostream& operator<<(std::ostream& os, const Position& pos);

inline char Board2D::board_width() const {
    return boardWidth;
}

inline Color Board2D::side_to_move() const {
    return sideToMove;
}

inline Piece Board2D::piece_on(Square2D s) const {
    return board[s];
}

inline bool Board2D::empty(Square2D s) const {
    return piece_on(s) == NO_PIECE;
}

template<PieceType Pt> inline const Square2D* Board2D::squares(Color c) const {
    return pieceList[make_piece(c, Pt)];
}

inline void Board2D::put_piece(Piece pc, Square2D s) {
    board[s] = pc;
    index[s] = pieceCount[pc]++;
    pieceList[pc][index[s]] = s;
    pieceCount[make_piece(color_of(pc), ALL_PIECES)]++;
}

inline void Board2D::remove_piece(Square2D s) {
    // this isn't strictly the inverse of put_piece.
    // You'll get back a board representing the same
    // position, but the index[] and pieceList[] may
    // be different.
    Piece pc = board[s];
    board[s] = NO_PIECE; // this is not strictly needed. move_piece clears it,
                         // and a capture would overwrite it.
    Square2D lastSquare = pieceList[pc][--pieceCount[pc]];
    index[lastSquare] = index[s];
    pieceList[pc][index[lastSquare]] = lastSquare;
    pieceList[pc][pieceCount[pc]] = SQ_NONE;
    pieceCount[make_piece(color_of(pc), ALL_PIECES)]--;
}

inline void Board2D::passTurn() {
    sideToMove = other_color(sideToMove);
}

inline Time Timeline::start_time() const {
    return startTime;
}

inline Color Timeline::start_color() const {
    return startColor;
}

inline bool Timeline::is_active() const {
    return active;
}

inline void Timeline::activate() {
    active = true;
}

inline const Board2D& Timeline::first_board() const {
    return *boards.front();
}

inline const Board2D& Timeline::last_board() const {
    return *boards.back();
}

inline bool Timeline::has_board_on_turn(Time time, Color c) const {
    int idx = plyToBoardIdx(time, c);
    return idx < 0;
}

inline Board2D& Timeline::board_on_turn(Time time, Color c) const {
    return *boards[plyToBoardIdx(time, c)];
}

inline void Timeline::append_board(Board2D& newBoard) {
    boards.push_back(std::shared_ptr<Board2D>(&newBoard));
}

inline Timeline& Timeline::set_print_indented(bool pi) {
    printIndented = pi;
    return *this;
}

inline L Position::negative_timeline_count() const {
    return negativeLines.size();
}

inline L Position::positive_timeline_count() const {
    // positiveLines[0] is the central line and shouldn't be counted
    return positiveLines.size() - 1;
}
inline const Timeline& Position::timeline(L timeline) const {
    if (timeline >= 0) {
        return positiveLines[timeline];
    }
    return negativeLines[-timeline - 1];
}

inline Color Position::side_to_move() const {
    return sideToMove;
}

inline Time Position::time_of_present() const {
    return timeOfPresent;
}

#endif
