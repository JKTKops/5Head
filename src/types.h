// This file is similar to the corresponding file in Stockfish 11.

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

// TODO: encoding of moves

typedef int Depth;

enum Color {
    WHITE, BLACK, COLOR_NB = 2
};

enum CastlingRights {
    NO_CASTLING,
    WHITE_OO, WHITE_OOO = WHITE_OO << 1,
    BLACK_OO = WHITE_OO << 2, BLACK_OOO = WHITE_OO << 3,

    KING_SIDE       = WHITE_OO  | BLACK_OO,
    QUEEN_SIDE      = WHITE_OOO | BLACK_OOO,
    WHITE_CASTLING  = WHITE_OO  | WHITE_OOO,
    BLACK_CASTLING  = BLACK_OO  | BLACK_OOO,
    ANY_CASTLING    = WHITE_CASTLING | BLACK_CASTLING,

    CASTLING_RIGHT_NB = 16
};

enum PieceType {
    NO_PIECE_TYPE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    ALL_PIECES = 0,
    PIECE_TYPE_NB = 8
};

enum Piece {
    NO_PIECE,
    W_PAWN = 1, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = 9, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_NB = 16
};

enum Square2D {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQ_NONE,
    SQUARE_NB = 64
};

enum File {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

enum Rank {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};

// It's useful to have a 2D direction for quickly handling standard
// chess moves. This can't be easily extended to the T and L dimensions
// but because we only know the size of the X and Y dimensions in advance,
// that's fine.
enum Direction2D : int {
    NORTH = 8, EAST = 1,
    SOUTH = -NORTH, WEST = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};

#define ENABLE_BASE_OPERATORS_ON(T)                            \
constexpr T operator+(T d1, int d2) { return T(int(d1) + d2); } \
constexpr T operator-(T d1, int d2) { return T(int(d1) - d2); } \
constexpr T operator-(T d) { return T(-int(d)); }               \
inline T& operator+=(T& d1, int d2) { return d1 = d1 + d2; }    \
inline T& operator-=(T& d1, int d2) { return d1 = d1 - d2; }

#define ENABLE_INCR_OPERATORS_ON(T)                      \
inline T& operator++(T& d) { return d = T(int(d) + 1); } \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

#define ENABLE_FULL_OPERATORS_ON(T)                         \
ENABLE_BASE_OPERATORS_ON(T)                                 \
constexpr T operator*(int i, T d) { return T(i * int(d)); } \
constexpr T operator*(T d, int i) { return T(int(d) * i); } \
constexpr T operator/(T d, int i) { return T(int(d) / i); } \
constexpr int operator/(T d1, T d2) { return int(d1) / int(d2); } \
inline T& operator*=(T& d, int i) { return d = T(int(d) * i); } \
inline T& operator/=(T& d, int i) { return d = T(int(d) / i); }

ENABLE_FULL_OPERATORS_ON(Direction2D);

ENABLE_INCR_OPERATORS_ON(PieceType);
ENABLE_INCR_OPERATORS_ON(Square2D);
ENABLE_INCR_OPERATORS_ON(File);
ENABLE_INCR_OPERATORS_ON(Rank);

#undef ENABLE_FULL_OPERATORS_ON
#undef ENABLE_INCR_OPERATORS_ON
#undef ENABLE_BASE_OPERATORS_ON

// for quickly adding 2D directions to 2D squares
constexpr Square2D operator+(Square2D s, Direction2D d) { return Square2D(int(s) + int(d)); }
constexpr Square2D operator-(Square2D s, Direction2D d) { return Square2D(int(s) - int(d)); }
inline Square2D& operator+=(Square2D& s, Direction2D d) { return s = s + d; }
inline Square2D& operator-=(Square2D& s, Direction2D d) { return s = s - d; }

constexpr Color other_color(Color c) {
    return Color(1 - c);
}

constexpr CastlingRights operator&(Color c, CastlingRights cr) {
    return CastlingRights((c == WHITE ? WHITE_CASTLING : BLACK_CASTLING) & cr);
}

constexpr Square2D make_square2d(File f, Rank r) {
    return Square2D((r << 3) + f);
}

constexpr Piece make_piece(Color c, PieceType pt) {
    return Piece((c << 3) + pt);
}

constexpr PieceType type_of(Piece pc) {
    return PieceType(pc & 7);
}

inline Color color_of(Piece pc) {
    return Color(pc >> 3);
}

constexpr bool is_ok_square2d(Square2D s) {
    return s >= SQ_A1 && s <= SQ_H8;
}

constexpr File file_of(Square2D s) {
    return File(s & 7);
}

constexpr Rank rank_of(Square2D s) {
    return Rank(s >> 3);
}

constexpr Direction2D pawn_push(Color c) {
    return c == WHITE ? NORTH : SOUTH;
}

#endif
