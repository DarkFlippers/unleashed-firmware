/*
 * fast-chess.h
 *
 *  Created on: 20 de set de 2016
 *      Author: fvj
 */

#ifndef FAST_CHESS_H_
#define FAST_CHESS_H_

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdint.h>

#define ENGINE_VERSION "v1.8.1"

#define ENGINE_NAME "github.com/fredericojordan/fast-chess " ENGINE_VERSION
#define HUMAN_NAME "Unknown Human Player"

#define NUM_SQUARES (64)
#define ENDGAME_PIECE_COUNT (7)

#define COLOR_MASK (1 << 3)
#define WHITE (0)
#define BLACK (1 << 3)

#define PIECE_MASK (0x7)
#define EMPTY (0)
#define PAWN (1)
#define KNIGHT (2)
#define BISHOP (3)
#define ROOK (4)
#define QUEEN (5)
#define KING (6)

#define ALL_SQUARES (0xFFFFFFFFFFFFFFFF)
#define FILE_A (0x0101010101010101)
#define FILE_B (0x0202020202020202)
#define FILE_C (0x0404040404040404)
#define FILE_D (0x0808080808080808)
#define FILE_E (0x1010101010101010)
#define FILE_F (0x2020202020202020)
#define FILE_G (0x4040404040404040)
#define FILE_H (0x8080808080808080)
#define RANK_1 (0x00000000000000FF)
#define RANK_2 (0x000000000000FF00)
#define RANK_3 (0x0000000000FF0000)
#define RANK_4 (0x00000000FF000000)
#define RANK_5 (0x000000FF00000000)
#define RANK_6 (0x0000FF0000000000)
#define RANK_7 (0x00FF000000000000)
#define RANK_8 (0xFF00000000000000)
#define DIAG_A1H8 (0x8040201008040201)
#define ANTI_DIAG_H1A8 (0x0102040810204080)
#define LIGHT_SQUARES (0x55AA55AA55AA55AA)
#define DARK_SQUARES (0xAA55AA55AA55AA55)

#define CASTLE_KINGSIDE_WHITE (1 << 0)
#define CASTLE_QUEENSIDE_WHITE (1 << 1)
#define CASTLE_KINGSIDE_BLACK (1 << 2)
#define CASTLE_QUEENSIDE_BLACK (1 << 3)

#define BOOL char

#ifndef FALSE
#define TRUE (1)
#define FALSE (0)
#endif

typedef uint_fast64_t Bitboard;
typedef int Move;

#define MAX_BOOK_ENTRY_LEN (300)
#define MAX_PLYS_PER_GAME (1024)
#define MAX_FEN_LEN (100)
// #define MAX_BRANCHING_FACTOR (218) /* R6R/3Q4/1Q4Q1/4Q3/2Q4Q/Q4Q2/pp1Q4/kBNN1KB1 w - - 0 1                  3Q4/1Q4Q1/4Q3/2Q4R/Q4Q2/3Q4/1Q4Rp/1K1BBNNk w - - 0 1 */
#define MAX_BRANCHING_FACTOR (100) // okalachev
#define MAX_ATTACKING_PIECES (12)

#define DEFAULT_AI_DEPTH (3)

typedef struct {
    Bitboard whiteKing;
    Bitboard whiteQueens;
    Bitboard whiteRooks;
    Bitboard whiteKnights;
    Bitboard whiteBishops;
    Bitboard whitePawns;

    Bitboard blackKing;
    Bitboard blackQueens;
    Bitboard blackRooks;
    Bitboard blackKnights;
    Bitboard blackBishops;
    Bitboard blackPawns;
} Board;

typedef struct {
    Board board;
    char toMove;
    char epSquare;
    char castlingRights;
    unsigned int halfmoveClock;
    unsigned int fullmoveNumber;
} Position;

typedef struct {
    Position position;

    unsigned int moveListLen;
    Move moveList[MAX_PLYS_PER_GAME];
    char positionHistory[MAX_PLYS_PER_GAME][MAX_FEN_LEN];
} Game;

typedef struct {
    Move move;
    int score;
} Node;

typedef struct {
    int depth;
    Position pos;
    int* alpha;
    int* beta;
    BOOL verbose;
} ThreadInfo;

extern char FILES[8];
extern char RANKS[8];

extern Bitboard FILES_BB[8];
extern Bitboard RANKS_BB[8];

extern char INITIAL_FEN[];
extern Board INITIAL_BOARD;
extern int PIECE_VALUES[];

#define DOUBLED_PAWN_PENALTY (10)
#define ISOLATED_PAWN_PENALTY (20)
#define BACKWARDS_PAWN_PENALTY (8)
#define PASSED_PAWN_BONUS (20)
#define ROOK_SEMI_OPEN_FILE_BONUS (10)
#define ROOK_OPEN_FILE_BONUS (15)
#define ROOK_ON_SEVENTH_BONUS (20)

extern int PAWN_BONUS[];
extern int KNIGHT_BONUS[];
extern int BISHOP_BONUS[];
extern int KING_BONUS[];
extern int KING_ENDGAME_BONUS[];
extern int FLIP_VERTICAL[];

void getInitialGame(Game* game);
void getFenGame(Game* game, char fen[]);
void insertPiece(Board* board, Bitboard position, char pieceCode);
int loadFen(Position* position, char fen[]);
int toFen(char* fen, Position* position);
int toMinFen(char* fen, Position* position);
void getMovelistGame(Game* game, char moves[]);

// ========= UTILITY =========

BOOL fromInitial(Game* game);
Bitboard index2bb(int index);
int str2index(char* str);
Bitboard str2bb(char* str);
BOOL isSet(Bitboard bb, int index);
Bitboard lsb(Bitboard bb);
Bitboard msb(Bitboard bb);
int bb2index(Bitboard bb);
char* movelist2str(Game* game);
Move getLastMove(Game* game);
BOOL startsWith(const char* str, const char* pre);
int countBookOccurrences(Game* game);
Move getBookMove(Game* game);
char getFile(int position);
char getRank(int position);
Move generateMove(int leavingSquare, int arrivingSquare);
int getFrom(Move move);
int getTo(Move move);
int char2piece(char pieceCode);
int bb2piece(Bitboard position, Board* board);
char bb2char(Bitboard position, Board* board);
char* bb2str(Bitboard position, Board* board);
void printBitboard(Bitboard bitboard);
char getPieceChar(Bitboard position, Board* board);
void printBoard(Board* board);
void printGame(Game* game);
Bitboard not(Bitboard bb);
char opponent(char color);
int countBits(Bitboard bb);
void sortNodes(Node* sortedNodes, Node* nodes, int len, char color);
void printMove(Move move);
void printFullMove(Move move, Board* board);
void printLegalMoves(Position* position);
void printNode(Node node);
void getTimestamp(char* timestamp);
void dumpContent(Game* game);
void dumpPGN(Game* game, char color, BOOL hasAI);
void move2str(char* str, Game* game, int moveNumber);
BOOL isAmbiguous(Position* posBefore, Move move);
unsigned long hashPosition(Position* position);
void writeToHashFile(Position* position, int evaluation, int depth);

// ====== BOARD FILTERS ======

Bitboard getColoredPieces(Board* board, char color);
Bitboard getEmptySquares(Board* board);
Bitboard getOccupiedSquares(Board* board);
Bitboard getTwinPieces(Bitboard position, Board* board);
Bitboard fileFilter(Bitboard positions);
Bitboard rankFilter(Bitboard positions);

// ======= DIRECTIONS ========

Bitboard east(Bitboard bb);
Bitboard west(Bitboard bb);
Bitboard north(Bitboard bb);
Bitboard south(Bitboard bb);
Bitboard NE(Bitboard bb);
Bitboard NW(Bitboard bb);
Bitboard SE(Bitboard bb);
Bitboard SW(Bitboard bb);
Bitboard WNW(Bitboard moving_piece);
Bitboard ENE(Bitboard moving_piece);
Bitboard NNW(Bitboard moving_piece);
Bitboard NNE(Bitboard moving_piece);
Bitboard ESE(Bitboard moving_piece);
Bitboard WSW(Bitboard moving_piece);
Bitboard SSE(Bitboard moving_piece);
Bitboard SSW(Bitboard moving_piece);

// ========== PAWN ===========

Bitboard getPawns(Board* board);
Bitboard pawnSimplePushes(Bitboard moving_piece, Board* board, char color);
Bitboard pawnDoublePushes(Bitboard moving_piece, Board* board, char color);
Bitboard pawnPushes(Bitboard moving_piece, Board* board, char color);
Bitboard pawnEastAttacks(Bitboard moving_piece, Board* board, char color);
Bitboard pawnWestAttacks(Bitboard moving_piece, Board* board, char color);
Bitboard pawnAttacks(Bitboard moving_piece, Board* board, char color);
Bitboard pawnSimpleCaptures(Bitboard moving_piece, Board* board, char color);
Bitboard pawnEpCaptures(Bitboard moving_piece, Position* position, char color);
Bitboard pawnCaptures(Bitboard moving_piece, Position* position, char color);
Bitboard pawnMoves(Bitboard moving_piece, Position* position, char color);
BOOL isDoublePush(int leaving, int arriving);
char getEpSquare(int leaving);
BOOL isDoubledPawn(Bitboard position, Board* board, char color);
BOOL isIsolatedPawn(Bitboard position, Board* board, char color);
BOOL isBackwardsPawn(Bitboard position, Board* board, char color);
BOOL isPassedPawn(Bitboard position, Board* board, char color);
BOOL isOpenFile(Bitboard position, Board* board);
BOOL isSemiOpenFile(Bitboard position, Board* board);

// ========== KNIGHT =========

Bitboard getKnights(Board* board);
Bitboard knightAttacks(Bitboard moving_piece);
Bitboard knightMoves(Bitboard moving_piece, Board* board, char color);

// ========== KING ===========

Bitboard getKing(Board* board, char color);
Bitboard kingAttacks(Bitboard moving_piece);
Bitboard kingMoves(Bitboard moving_piece, Board* board, char color);
BOOL canCastleKingside(Position* position, char color);
BOOL canCastleQueenside(Position* position, char color);
char removeCastlingRights(char original_rights, char removed_rights);

// ========== BISHOP =========

Bitboard getBishops(Board* board);
Bitboard NE_ray(Bitboard bb);
Bitboard SE_ray(Bitboard bb);
Bitboard NW_ray(Bitboard bb);
Bitboard SW_ray(Bitboard bb);
Bitboard NE_attack(Bitboard single_piece, Board* board, char color);
Bitboard NW_attack(Bitboard single_piece, Board* board, char color);
Bitboard SE_attack(Bitboard single_piece, Board* board, char color);
Bitboard SW_attack(Bitboard single_piece, Board* board, char color);
Bitboard diagonalAttacks(Bitboard single_piece, Board* board, char color);
Bitboard antiDiagonalAttacks(Bitboard single_piece, Board* board, char color);
Bitboard bishopAttacks(Bitboard moving_pieces, Board* board, char color);
Bitboard bishopMoves(Bitboard moving_piece, Board* board, char color);

// ========== ROOK ===========

Bitboard getRooks(Board* board);
Bitboard northRay(Bitboard moving_pieces);
Bitboard southRay(Bitboard moving_pieces);
Bitboard eastRay(Bitboard moving_pieces);
Bitboard westRay(Bitboard moving_pieces);
Bitboard northAttack(Bitboard single_piece, Board* board, char color);
Bitboard southAttack(Bitboard single_piece, Board* board, char color);
Bitboard fileAttacks(Bitboard single_piece, Board* board, char color);
Bitboard eastAttack(Bitboard single_piece, Board* board, char color);
Bitboard westAttack(Bitboard single_piece, Board* board, char color);
Bitboard rankAttacks(Bitboard single_piece, Board* board, char color);
Bitboard rookAttacks(Bitboard moving_piece, Board* board, char color);
Bitboard rookMoves(Bitboard moving_piece, Board* board, char color);

// ========== QUEEN ==========

Bitboard getQueens(Board* board);
Bitboard queenAttacks(Bitboard moving_piece, Board* board, char color);
Bitboard queenMoves(Bitboard moving_piece, Board* board, char color);

// ======== MAKE MOVE ========

void clearPositions(Board* board, Bitboard positions);
void movePiece(Board* board, Move move);
void updatePosition(Position* newPosition, Position* position, Move move);
void makeMove(Game* game, Move move);
void unmakeMove(Game* game);

// ======== MOVE GEN =========

Bitboard getMoves(Bitboard movingPiece, Position* position, char color);
int pseudoLegalMoves(Move* moves, Position* position, char color);
Bitboard getAttacks(Bitboard movingPiece, Board* board, char color);
int countAttacks(Bitboard target, Board* board, char color);
BOOL isAttacked(Bitboard target, Board* board, char color);
BOOL isCheck(Board* board, char color);
BOOL isLegalMove(Position* position, Move move);
int legalMoves(Move* legalMoves, Position* position, char color);
int legalMovesCount(Position* position, char color);
int staticOrderLegalMoves(Move* orderedLegalMoves, Position* position, char color);
int legalCaptures(Move* legalCaptures, Position* position, char color);

// ====== GAME CONTROL =======

BOOL isCheckmate(Position* position);
BOOL isStalemate(Position* position);
BOOL hasInsufficientMaterial(Board* board);
BOOL isEndgame(Board* board);
BOOL isOver75MovesRule(Position* position);
BOOL hasGameEnded(Position* position);
void printOutcome(Position* position);

// ========== EVAL ===========

int winScore(char color);
int materialSum(Board* board, char color);
int materialBalance(Board* board);
int positionalBonus(Board* board, char color);
int positionalBalance(Board* board);
int endNodeEvaluation(Position* position);
int staticEvaluation(Position* position);
int getCaptureSequence(Move* captures, Position* position, int targetSquare);
int staticExchangeEvaluation(Position* position, int targetSquare);
int quiescenceEvaluation(Position* position);

// ========= SEARCH ==========

Node staticSearch(Position* position);
Node quiescenceSearch(Position* position);
Node alphaBeta(Position* position, char depth, int alpha, int beta);
int alphaBetaNodes(Node* nodes, Position* position, char depth);
Node iterativeDeepeningAlphaBeta(Position* position, char depth, int alpha, int beta, BOOL verbose);
Node pIDAB(Position* position, char depth, int* p_alpha, int* p_beta);
Node pIDABhashed(Position* position, char depth, int* p_alpha, int* p_beta);
Move getRandomMove(Position* position);
Move getAIMove(Game* game, int depth);
Move parseMove(char* move);
Move getPlayerMove();
Move suggestMove(char fen[], int depth);

// Parallel processing currently only implemented for Windows
#ifdef _WIN32
DWORD WINAPI evaluatePositionThreadFunction(LPVOID lpParam);
DWORD WINAPI evaluatePositionThreadFunctionHashed(LPVOID lpParam);
Node idabThreaded(Position* position, int depth, BOOL verbose);
Node idabThreadedBestFirst(Position* position, int depth, BOOL verbose);
Node idabThreadedBestFirstHashed(Position* position, int depth, BOOL verbose);
#endif

// ===== PLAY LOOP (TEXT) ====

void playTextWhite(int depth);
void playTextBlack(int depth);
void playTextAs(char color, int depth);
void playTextRandomColor(int depth);

// ===========================

#endif /* FAST_CHESS_H_ */