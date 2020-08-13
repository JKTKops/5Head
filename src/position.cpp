#include <algorithm>
#include <sstream>
#include <cstring> // for std::memset and memcmp
#include <cassert>

#include "position.h"
#include "types.h"

namespace {
    const std::string PieceToChar(" PNBRQK  pnbrqk");

    constexpr Piece Pieces[] = {
        W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
        B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
    };
}

// used for rendering boards as ASCII.
namespace {
    std::string row_separator(const Board2D& pos) {
        std::ostringstream ss;
        ss << "+";

        for (File file = FILE_A; file < FILE_A + pos.board_width(); ++file) {
            ss << "---+";
        }

        return ss.str();
    }
}

// operator<<(Position) gives an ASCII rep. of a single 2D board.
// Laying out an image of the entire position is rather complex,
// so we do it timeline-by-timeline, taking one line of text from
// each board at a time. Newlines are separated by LF, not CRLF.
// This requires rendering every board on the
// timeline in order to lay them out (!!) but performance here is not
// especially a concern. This is for debugging.
//
// Each line contains the same number of columns. Each board contains
// 18 lines. An initial newline is not printed, but a final one is.
std::ostream& operator<<(std::ostream& os, const Board2D& pos) {
    char width = pos.board_width();
    std::string row_sep = row_separator(pos);
    std::string first_row_sep = row_sep;
    first_row_sep[1] = pos.side_to_move() == WHITE ? 'W' : 'B';

    os << first_row_sep << "  \n";

    for (Rank r = Rank(RANK_1 + width - 1); r >= RANK_1; --r) {
        for (File f = FILE_A; f < FILE_A + width; ++f) {
            os << "| " << PieceToChar[pos.piece_on(make_square2d(f, r))] << " ";
        }

        os << "| " << (1 + r);
        os << "\n" << row_sep << "  \n";
    }

    os << std::string("  a   b   c   d   e   f   g   h   ").substr(0, 2 + 4*width)
       << " \n";
    return os;
}

// Taken from Stockfish, with modifications for boards not 8x8.
Board2D& Board2D::set(const std::string& fenStr) {
    unsigned char token, width = 0;
    std::memset(this, 0, sizeof(Board2D));
    std::fill_n(&pieceList[0][0], sizeof(pieceList) / sizeof(Square2D), SQ_NONE);

    // 0. calculate board width
    // we assume here that the FEN is valid
    for (int idx = 0; !isspace(fenStr[idx]); ++idx) {
        token = fenStr[idx];
        if (isdigit(token)) {
            width += token - '0';
        } else if (token == '/') {
            break;
        } else {
            ++width;
        }
    }
    boardWidth = width;

    size_t idx;
    Square2D sq = SQ_A1 + (width - 1) * NORTH;
    std::istringstream ss(fenStr);

    ss >> std::noskipws;

    // 1. pieces
    while ((ss >> token) && !isspace(token)) {
        if (isdigit(token)) {
            sq += (token - '0') * EAST;
        } else if (token == '/') {
            sq += width * WEST;
            sq += SOUTH;
        } else if ((idx = PieceToChar.find(token)) != std::string::npos) {
            put_piece(Piece(idx), sq);
            ++sq;
        }
    }

    // 2. active color
    ss >> token;
    sideToMove = (token == 'w' ? WHITE : BLACK);
    ss >> token;

    return *this;
}

// debugging function; also from stockfish, with modifications.
// Currently not correct FEN as we aren't keeping all information like castling
// and EP.
const std::string Board2D::fen() const {
    int emptyCnt;
    std::ostringstream ss;

    Rank topRank = Rank(RANK_1 + boardWidth - 1);
    File rightFile = File(FILE_A + boardWidth - 1);

    for (Rank r = topRank; r >= RANK_1; --r) {
        for (File f = FILE_A; f <= rightFile; ++f) {
            for (emptyCnt = 0; f <= rightFile && empty(make_square2d(f, r)); ++f) {
                ++emptyCnt;
            }

            if (emptyCnt) ss << emptyCnt;

            if (f <= rightFile) ss << PieceToChar[piece_on(make_square2d(f, r))];
        }

        if (r > RANK_1) ss << '/';
    }

    ss << (sideToMove == WHITE ? " w " : " b ");

    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const Timeline& line) {
    const int v_sep = 5;
    // board_width() is also the number of ranks
    // and since we get 2 lines per rank, the width is half the height
    // of the ascii board.
    const unsigned middle_line = (unsigned) (*line.boards[0]).board_width();
    std::vector<std::string> lines;

    std::stringstream first_string_stream;
    first_string_stream << *line.boards[0];

    std::string str_line;
    // prepare the vector by filling in the indentation prefix and the first board
    while (std::getline(first_string_stream, str_line, '\n')) {
        if (line.printIndented) {
            int startingPly = 2 * (line.startTime - 1) + line.startColor;
            int charWidthOfBoard = (*line.boards[0]).board_width() * 4 + 3;
            int spaceIndent = (charWidthOfBoard + v_sep) * startingPly;

            str_line = std::string(spaceIndent, ' ') + str_line;

            
        }

        lines.push_back(str_line);
    }

    // fill in the rest of the boards
    for (size_t boardIdx = 1; boardIdx < line.boards.size(); ++boardIdx) {
        const Board2D& board = *line.boards[boardIdx];

        std::stringstream board_string;
        board_string << board;

        // all boards output the same number of lines when printed
        for (size_t linesIdx = 0; linesIdx < lines.size(); ++linesIdx) {
            std::getline(board_string, str_line, '\n');
            if (linesIdx == middle_line) {
                lines[linesIdx] += "---> ";
            } else {
                lines[linesIdx] += std::string(v_sep, ' ');
            }
            lines[linesIdx] += str_line;
        }
    }

    for (const std::string& line : lines) {
        os << line << std::endl;
    }

    return os;
}

Timeline::Timeline(Time setStartTime, Color setStartColor) {
    startTime = setStartTime;
    startColor = setStartColor;
}

int Timeline::plyToBoardIdx(Time time, Color c) const {
    int dt = time - startTime;
    int dc = c - startColor;
    return 2 * dt + dc;
}

std::ostream& operator<<(std::ostream& os, const Position& pos) {
    for (L timeline = -pos.negative_timeline_count();
         timeline <= pos.positive_timeline_count();
         ++timeline) {
        os << pos.timeline(timeline);
        os << std::endl;
    }

    return os;
}

// Maybe we want to just take one list of FENs and balance it ourselves,
// to avoid weird BS with imbalanced active timelines (which is not possible
// in a legal position of a real game, and therefore shouldn't be legal in
// puzzles either...?)
void Position::set
(
    std::vector<std::string> negativeFENs,
    std::vector<std::string> positiveFENs
) {
    negativeLines.clear();
    positiveLines.clear();
    activePositiveLines = 0;
    activeNegativeLines = 0;
    timeOfPresent = 0;

    for (int i = negativeFENs.size() - 1; i >= 0; --i) {
        Board2D* board = new Board2D();
        board->set(negativeFENs[i]);

        Timeline tl(1, board->side_to_move());
        tl.append_board(*board);
        tl.activate();

        negativeLines.push_back(tl);
    }
    for (std::string& fen : positiveFENs) {
        Board2D* board = new Board2D();
        board->set(fen);

        Timeline tl(1, board->side_to_move());
        tl.append_board(*board);
        tl.activate();

        positiveLines.push_back(tl);
    }

    activePositiveLines = positiveFENs.size() - 1;
    activeNegativeLines = negativeFENs.size();
    timeOfPresent = 1;
    sideToMove = positiveLines[0].first_board().side_to_move();
}

Board2D& Position::new_timeline(L branchLine, Time branchTime) {
    const Timeline& targetLine = timeline(branchLine);
    const Board2D& targetBoard = targetLine.board_on_turn(branchTime, sideToMove);

    Board2D* newBoard = new Board2D(targetBoard);
    newBoard->passTurn();
    Timeline newTimeline(branchTime + (int)sideToMove, newBoard->side_to_move());
    newTimeline.append_board(*newBoard);

    L pos_cnt = positive_timeline_count();
    L neg_cnt = negative_timeline_count();

    if (sideToMove == WHITE) {
        if (pos_cnt == neg_cnt || pos_cnt == neg_cnt - 1) {
            // all timelines are active and the new timeline should be active
            newTimeline.activate();
            ++activePositiveLines;
            // if the new timeline goes before the present, move the present
            timeOfPresent = std::min(timeOfPresent, newTimeline.start_time());
        } else if (pos_cnt < neg_cnt - 1) {
            // black has inactive timelines. Specifically, we know -(pos_cnt + 2) is inactive
            // and should be activated.
            newTimeline.activate();
            ++activePositiveLines;

            // negativeLines is 0-indexed
            assert(!negativeLines[pos_cnt + 1].is_active());
            negativeLines[pos_cnt + 1].activate();
            ++activeNegativeLines;

            timeOfPresent = std::min(timeOfPresent,
                                     std::min(newTimeline.start_time(),
                                              negativeLines[pos_cnt+1].start_time()
                                             )
                                    );
        } // else white has more timelines and this one should stay inactive.

        positiveLines.push_back(newTimeline);
    } else { // sideToMove == BLACK
        if (neg_cnt == pos_cnt || neg_cnt == pos_cnt - 1) {
            newTimeline.activate();
            ++activeNegativeLines;
            timeOfPresent = std::min(timeOfPresent, newTimeline.start_time());
        } else if (neg_cnt < pos_cnt - 1) {
            // white has inactive timelines, specifically, -(neg_cnt + 2) is inactive
            newTimeline.activate();
            ++activeNegativeLines;

            assert(!positiveLines[neg_cnt + 2].is_active());
            positiveLines[neg_cnt + 2].activate();
            timeOfPresent = std::min(timeOfPresent,
                                     std::min(newTimeline.start_time(),
                                              positiveLines[neg_cnt+2].start_time()
                                             )
                                    );
        } // else black has more timelines than white already.

        negativeLines.push_back(newTimeline);
    }

    return *newBoard;
}
