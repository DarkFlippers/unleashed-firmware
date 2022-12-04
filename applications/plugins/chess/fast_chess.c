/*
 ============================================================================
 Name        : fast-chess.c
 Author      : Frederico Jordan <fredericojordan@gmail.com>
 Version     :
 Copyright   : Copyright (c) 2016 Frederico Jordan
 Description : Simple chess game!
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "fast_chess.h"

char FILES[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
char RANKS[8] = {'1', '2', '3', '4', '5', '6', '7', '8'};

Bitboard FILES_BB[8] = {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H};
Bitboard RANKS_BB[8] = {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8};

char INITIAL_FEN[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

Board EMPTY_BOARD = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
Board INITIAL_BOARD = {
    FILE_E & RANK_1, // whiteKing;
    FILE_D& RANK_1, // whiteQueens;
    (FILE_A | FILE_H) & RANK_1, // whiteRooks;
    (FILE_B | FILE_G) & RANK_1, // whiteKnights;
    (FILE_C | FILE_F) & RANK_1, // whiteBishops;
    RANK_2, // whitePawns;

    FILE_E& RANK_8, // blackKing;
    FILE_D& RANK_8, // blackQueens;
    (FILE_A | FILE_H) & RANK_8, // blackRooks;
    (FILE_B | FILE_G) & RANK_8, // blackKnights;
    (FILE_C | FILE_F) & RANK_8, // blackBishops;
    RANK_7, // blackPawns;
};

int PIECE_VALUES[] = {
    0, // EMPTY
    100, // PAWN
    300, // KNIGHT
    300, // BISHOP
    500, // ROOK
    900, // QUEEN
    42000 // KING
};

int PAWN_BONUS[] = {0, 0,  0,  0,   0,   0,  0,  0, 0, 0, 0,  -40, -40, 0,  0, 0,
                    1, 2,  3,  -10, -10, 3,  2,  1, 2, 4, 6,  8,   8,   6,  4, 2,
                    3, 6,  9,  12,  12,  9,  6,  3, 4, 8, 12, 16,  16,  12, 8, 4,
                    5, 10, 15, 20,  20,  15, 10, 5, 0, 0, 0,  0,   0,   0,  0, 0};

int KNIGHT_BONUS[] = {-10, -30, -10, -10, -10, -10, -30, -10, -10, 0,   0,   0,   0,
                      0,   0,   -10, -10, 0,   5,   5,   5,   5,   0,   -10, -10, 0,
                      5,   10,  10,  5,   0,   -10, -10, 0,   5,   10,  10,  5,   0,
                      -10, -10, 0,   5,   5,   5,   5,   0,   -10, -10, 0,   0,   0,
                      0,   0,   0,   -10, -10, -10, -10, -10, -10, -10, -10, -10};

int BISHOP_BONUS[] = {-10, -10, -20, -10, -10, -20, -10, -10, -10, 0,   0,   0,   0,
                      0,   0,   -10, -10, 0,   5,   5,   5,   5,   0,   -10, -10, 0,
                      5,   10,  10,  5,   0,   -10, -10, 0,   5,   10,  10,  5,   0,
                      -10, -10, 0,   5,   5,   5,   5,   0,   -10, -10, 0,   0,   0,
                      0,   0,   0,   -10, -10, -10, -10, -10, -10, -10, -10, -10};

int KING_BONUS[] = {0,   20,  40,  -20, 0,   -20, 40,  20,  -20, -20, -20, -20, -20,
                    -20, -20, -20, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
                    -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
                    -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
                    -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40};

int KING_ENDGAME_BONUS[] = {0,  10, 20, 30, 30, 20, 10, 0,  10, 20, 30, 40, 40, 30, 20, 10,
                            20, 30, 40, 50, 50, 40, 30, 20, 30, 40, 50, 60, 60, 50, 40, 30,
                            30, 40, 50, 60, 60, 50, 40, 30, 20, 30, 40, 50, 50, 40, 30, 20,
                            10, 20, 30, 40, 40, 30, 20, 10, 0,  10, 20, 30, 30, 20, 10, 0};

int FLIP_VERTICAL[] = {56, 57, 58, 59, 60, 61, 62, 63, 48, 49, 50, 51, 52, 53, 54, 55,
                       40, 41, 42, 43, 44, 45, 46, 47, 32, 33, 34, 35, 36, 37, 38, 39,
                       24, 25, 26, 27, 28, 29, 30, 31, 16, 17, 18, 19, 20, 21, 22, 23,
                       8,  9,  10, 11, 12, 13, 14, 15, 0,  1,  2,  3,  4,  5,  6,  7};

void getInitialGame(Game* game) {
    game->position.board = INITIAL_BOARD;
    game->position.toMove = WHITE;
    game->position.epSquare = -1;
    game->position.castlingRights = CASTLE_KINGSIDE_WHITE | CASTLE_QUEENSIDE_WHITE |
                                    CASTLE_KINGSIDE_BLACK | CASTLE_QUEENSIDE_BLACK;
    game->position.halfmoveClock = 0;
    game->position.fullmoveNumber = 1;

    game->moveListLen = 0;
    memset(game->moveList, 0, MAX_PLYS_PER_GAME * sizeof(int));
    memset(game->positionHistory, 0, MAX_PLYS_PER_GAME * MAX_FEN_LEN * sizeof(char));
    memcpy(game->positionHistory[0], INITIAL_FEN, sizeof(INITIAL_FEN));
}

void getFenGame(Game* game, char fen[]) {
    int fenLen = loadFen(&(game->position), fen);

    game->moveListLen = 0;
    memset(game->moveList, 0, MAX_PLYS_PER_GAME * sizeof(int));
    memset(game->positionHistory, 0, MAX_PLYS_PER_GAME * MAX_FEN_LEN * sizeof(char));
    memcpy(game->positionHistory[0], fen, fenLen);
}

void insertPiece(Board* board, Bitboard position, char pieceCode) {
    switch(pieceCode) {
    case 'P':
        board->whitePawns |= position;
        break;
    case 'N':
        board->whiteKnights |= position;
        break;
    case 'B':
        board->whiteBishops |= position;
        break;
    case 'R':
        board->whiteRooks |= position;
        break;
    case 'Q':
        board->whiteQueens |= position;
        break;
    case 'K':
        board->whiteKing |= position;
        break;

    case 'p':
        board->blackPawns |= position;
        break;
    case 'n':
        board->blackKnights |= position;
        break;
    case 'b':
        board->blackBishops |= position;
        break;
    case 'r':
        board->blackRooks |= position;
        break;
    case 'q':
        board->blackQueens |= position;
        break;
    case 'k':
        board->blackKing |= position;
        break;
    }
}

int loadFen(Position* position, char fen[]) {
    // ===== BOARD =====
    position->board = EMPTY_BOARD;

    int rank = 7;
    int boardPos = rank * 8;
    char* charPos = fen;

    char pieceCode = *(charPos);

    while(pieceCode != ' ') {
        if(pieceCode == '/') {
            rank--;
            boardPos = rank * 8;
        } else if(isdigit(pieceCode)) {
            int emptySquares = atoi(charPos);
            boardPos += emptySquares;
        } else {
            insertPiece(&(position->board), index2bb(boardPos++), pieceCode);
        }

        pieceCode = *(++charPos);
    }

    // ===== TO MOVE =====
    char* nextFenField = strchr(fen, ' ') + 1;

    if(*nextFenField == 'b') {
        position->toMove = BLACK;
    } else {
        position->toMove = WHITE;
    }

    // ===== CASTLING RIGHTS =====
    nextFenField = strchr(nextFenField, ' ') + 1;

    position->castlingRights = 0;
    if(strchr(nextFenField, 'K')) position->castlingRights |= CASTLE_KINGSIDE_WHITE;
    if(strchr(nextFenField, 'Q')) position->castlingRights |= CASTLE_QUEENSIDE_WHITE;
    if(strchr(nextFenField, 'k')) position->castlingRights |= CASTLE_KINGSIDE_BLACK;
    if(strchr(nextFenField, 'q')) position->castlingRights |= CASTLE_QUEENSIDE_BLACK;

    // ===== EN PASSANT =====
    nextFenField = strchr(nextFenField, ' ') + 1;

    if(*nextFenField == '-') {
        position->epSquare = -1;
    } else {
        position->epSquare = str2index(nextFenField);
    }

    // ===== HALF MOVE CLOCK =====
    if(!strchr(nextFenField, ' ')) {
        position->halfmoveClock = 0;
        position->fullmoveNumber = 1;
        return 1 + nextFenField - fen;
    }
    nextFenField = strchr(nextFenField, ' ') + 1;

    position->halfmoveClock = atoi(nextFenField);

    // ===== FULL MOVE NUMBER =====
    if(!strchr(nextFenField, ' ')) {
        position->fullmoveNumber = 1;
        return 1 + nextFenField - fen;
    }
    nextFenField = strchr(nextFenField, ' ') + 1;

    position->fullmoveNumber = atoi(nextFenField);

    return 1 + nextFenField - fen;
}

int toFen(char* fen, Position* position) {
    int charCount = toMinFen(fen, position);
    fen[charCount - 1] = ' ';

    // ===== HALF MOVE CLOCK =====
    snprintf(&fen[charCount++], sizeof(&fen[charCount++]), "%d", position->halfmoveClock);
    if(position->halfmoveClock >= 10) {
        charCount++;
        if(position->halfmoveClock >= 100) {
            charCount++;
        }
    }
    fen[charCount++] = ' ';

    // ===== FULL MOVE NUMBER =====
    snprintf(&fen[charCount++], sizeof(&fen[charCount++]), "%d", position->fullmoveNumber);
    if(position->fullmoveNumber >= 10) {
        charCount++;
        if(position->fullmoveNumber >= 100) {
            charCount++;
        }
    }
    fen[charCount++] = '\0';

    return charCount;
}

int toMinFen(char* fen, Position* position) {
    int charCount = 0;

    // ===== BOARD =====
    int rank = 7;
    int file = 0;
    int* emptyCount = 0;

    Bitboard empties = getEmptySquares(&(position->board));
    Bitboard bb;

    while(rank >= 0) {
        bb = index2bb(8 * rank + file);

        if(bb & empties) {
            emptyCount++;
        } else {
            if(emptyCount != 0) {
                snprintf(&fen[charCount++], 2, "%ls", emptyCount);
                emptyCount = 0;
            }
            fen[charCount++] = bb2char(bb, &(position->board));
        }

        file++;
        if(file > 7) {
            if(emptyCount != 0) {
                snprintf(&fen[charCount++], 2, "%ls", emptyCount);
                emptyCount = 0;
            }
            file = 0;
            rank--;
            fen[charCount++] = '/';
        }
    }
    fen[charCount - 1] = ' ';

    // ===== TO MOVE =====
    if(position->toMove == BLACK) {
        fen[charCount++] = 'b';
    } else {
        fen[charCount++] = 'w';
    }
    fen[charCount++] = ' ';

    // ===== CASTLING RIGHTS =====
    if(position->castlingRights == 0) {
        fen[charCount++] = '-';
    } else {
        if(position->castlingRights & CASTLE_KINGSIDE_WHITE) {
            fen[charCount++] = 'K';
        }
        if(position->castlingRights & CASTLE_QUEENSIDE_WHITE) {
            fen[charCount++] = 'Q';
        }
        if(position->castlingRights & CASTLE_KINGSIDE_BLACK) {
            fen[charCount++] = 'k';
        }
        if(position->castlingRights & CASTLE_QUEENSIDE_BLACK) {
            fen[charCount++] = 'q';
        }
    }
    fen[charCount++] = ' ';

    // ===== EN PASSANT =====
    if(position->epSquare == -1) {
        fen[charCount++] = '-';
    } else {
        fen[charCount++] = getFile(position->epSquare);
        fen[charCount++] = getRank(position->epSquare);
    }
    fen[charCount++] = '\0';

    return charCount;
}

void getMovelistGame(Game* game, char moves[]) {
    getInitialGame(game);

    for(int i = 0; i < strlen(moves) - 3; i += 5) {
        makeMove(game, parseMove(&moves[i]));
        if(moves[i + 5] == ' ') i++; // FIXME Queening
    }
}

// ========= UTILITY =========

BOOL fromInitial(Game* game) {
    if(strcmp(game->positionHistory[0], INITIAL_FEN) == 0)
        return TRUE;
    else
        return FALSE;
}

Bitboard index2bb(int index) {
    Bitboard bb = 1;
    return bb << index;
}

int str2index(char* str) {
    int i, file_num = 0, rank_num = 0;
    for(i = 0; i < 8; i++) {
        if(str[0] == FILES[i]) file_num = i;
        if(str[1] == RANKS[i]) rank_num = i;
    }
    return 8 * rank_num + file_num;
}

Bitboard str2bb(char* str) {
    return index2bb(str2index(str));
}

BOOL isSet(Bitboard bb, int index) {
    if(bb & index2bb(index))
        return TRUE;
    else
        return FALSE;
}

Bitboard lsb(Bitboard bb) {
    int i;
    for(i = 0; i < NUM_SQUARES; i++) {
        Bitboard bit = index2bb(i);
        if(bb & bit) return bit;
    }
    return 0;
}

Bitboard msb(Bitboard bb) {
    int i;
    for(i = 0; i < NUM_SQUARES; i++) {
        Bitboard bit = index2bb(63 - i);
        if(bb & bit) return bit;
    }
    return 0;
}

int bb2index(Bitboard bb) {
    int i;
    for(i = 0; i < NUM_SQUARES; i++) {
        Bitboard bit = index2bb(i);
        if(bb & bit) return i;
    }
    return -1;
}

char* movelist2str(Game* game) {
    char* movestr = NULL;

    if(game->moveListLen == 0) {
        movestr = (char*)malloc(sizeof(char));
        movestr[0] = 0;
        return movestr;
    }

    movestr = (char*)malloc(5 * game->moveListLen);

    int i;
    for(i = 0; i < game->moveListLen; i++) {
        int leaving = getFrom(game->moveList[i]);
        int arriving = getTo(game->moveList[i]);
        movestr[5 * i] = getFile(leaving);
        movestr[5 * i + 1] = getRank(leaving);
        movestr[5 * i + 2] = getFile(arriving);
        movestr[5 * i + 3] = getRank(arriving);
        movestr[5 * i + 4] = ' ';
    }

    movestr[5 * game->moveListLen - 1] = 0;

    return movestr;
}

Move getLastMove(Game* game) {
    if(game->moveListLen == 0)
        return 0;
    else
        return game->moveList[game->moveListLen - 1];
}

BOOL startsWith(const char* str, const char* pre) {
    size_t lenpre = strlen(pre), lenstr = strlen(str);

    if(lenpre > lenstr) return FALSE;

    return strncmp(pre, str, lenpre) == 0 ? TRUE : FALSE;
}

int countBookOccurrences(Game* game) {
    FILE* fp = fopen("book.txt", "r");

    if(fp == NULL) return 0;

    char* moveList = movelist2str(game);
    char* line = (char*)malloc(sizeof(char) * MAX_BOOK_ENTRY_LEN);
    int charPos = 0, occurrences = 0;

    while(TRUE) {
        char ch = getc(fp);
        line[charPos++] = ch;

        if(ch == '\n' || ch == EOF) {
            line[charPos - 1] = '\0';

            if(startsWith(line, moveList) && strlen(line) > strlen(moveList) + 4) {
                occurrences++;
            }

            if(ch == EOF) break;

            charPos = 0;
        }
    }

    fclose(fp);
    free(line);
    free(moveList);

    return occurrences;
}

Move getBookMove(Game* game) {
    Move move = 0;
    int moveNum = rand() % countBookOccurrences(game);

    FILE* fp = fopen("book.txt", "r");

    if(fp == NULL) return 0;

    char* moveList = movelist2str(game);
    char* line = (char*)malloc(sizeof(char) * MAX_BOOK_ENTRY_LEN);
    int charPos = 0, occurrences = 0;

    while(TRUE) {
        char ch = getc(fp);
        line[charPos++] = ch;

        if(ch == '\n') {
            line[charPos] = '\0';

            if(startsWith(line, moveList)) {
                if(occurrences == moveNum) {
                    int ind = game->moveListLen * 5;
                    move = parseMove(&line[ind]);
                    break;
                }
                occurrences++;
            }

            charPos = 0;
        }
    }

    fclose(fp);
    free(line);
    free(moveList);

    return move;
}

char getFile(int position) {
    int file = position % 8;
    return FILES[file];
}

char getRank(int position) {
    int rank = (int)(position / 8);
    return RANKS[rank];
}

Move generateMove(int leavingSquare, int arrivingSquare) {
    int leaving = (leavingSquare << 8);
    int arriving = arrivingSquare;
    return (Move)(leaving + arriving);
}

int getFrom(Move move) {
    return (move >> 8) & 0xFF;
}

int getTo(Move move) {
    return move & 0xFF;
}

int bb2piece(Bitboard position, Board* board) {
    if(position & board->whitePawns) return WHITE | PAWN;
    if(position & board->whiteKnights) return WHITE | KNIGHT;
    if(position & board->whiteBishops) return WHITE | BISHOP;
    if(position & board->whiteRooks) return WHITE | ROOK;
    if(position & board->whiteQueens) return WHITE | QUEEN;
    if(position & board->whiteKing) return WHITE | KING;
    if(position & board->blackPawns) return BLACK | PAWN;
    if(position & board->blackKnights) return BLACK | KNIGHT;
    if(position & board->blackBishops) return BLACK | BISHOP;
    if(position & board->blackRooks) return BLACK | ROOK;
    if(position & board->blackQueens) return BLACK | QUEEN;
    if(position & board->blackKing) return BLACK | KING;

    return EMPTY;
}

char bb2char(Bitboard position, Board* board) {
    if(position & board->whitePawns) return 'P';
    if(position & board->whiteKnights) return 'N';
    if(position & board->whiteBishops) return 'B';
    if(position & board->whiteRooks) return 'R';
    if(position & board->whiteQueens) return 'Q';
    if(position & board->whiteKing) return 'K';
    if(position & board->blackPawns) return 'p';
    if(position & board->blackKnights) return 'n';
    if(position & board->blackBishops) return 'b';
    if(position & board->blackRooks) return 'r';
    if(position & board->blackQueens) return 'q';
    if(position & board->blackKing) return 'k';

    return '?';
}

char* bb2str(Bitboard position, Board* board) {
    if((position & board->whitePawns) | (position & board->blackPawns)) return "Pawn";
    if((position & board->whiteKnights) | (position & board->blackKnights)) return "Knight";
    if((position & board->whiteBishops) | (position & board->blackBishops)) return "Bishop";
    if((position & board->whiteRooks) | (position & board->blackRooks)) return "Rook";
    if((position & board->whiteQueens) | (position & board->blackQueens)) return "Queen";
    if((position & board->whiteKing) | (position & board->blackKing)) return "King";

    return "?";
}

void printBitboard(Bitboard bitboard) {
    int rank, file;

    printf("\n");
    for(rank = 0; rank < 8; rank++) {
        printf("%d", 8 - rank);
        for(file = 0; file < 8; file++) {
            if(bitboard >> (file + (7 - rank) * 8) & 1) {
                printf(" #");
            } else {
                printf(" .");
            }
        }
        printf("\n");
    }
    printf("  a b c d e f g h\n");
    fflush(stdout);
}

char getPieceChar(Bitboard position, Board* board) {
    if(position & board->whiteKing) return 'K';
    if(position & board->whiteQueens) return 'Q';
    if(position & board->whiteRooks) return 'R';
    if(position & board->whiteKnights) return 'N';
    if(position & board->whiteBishops) return 'B';
    if(position & board->whitePawns) return 'P';
    if(position & board->blackKing) return 'k';
    if(position & board->blackQueens) return 'q';
    if(position & board->blackRooks) return 'r';
    if(position & board->blackKnights) return 'n';
    if(position & board->blackBishops) return 'b';
    if(position & board->blackPawns) return 'p';
    return '.';
}

void printBoard(Board* board) {
    int rank, file;

    printf("\n");
    for(rank = 0; rank < 8; rank++) {
        printf("%d", 8 - rank);
        for(file = 0; file < 8; file++) {
            printf(" %c", getPieceChar((FILES_BB[file] & RANKS_BB[7 - rank]), board));
        }
        printf("\n");
    }
    printf("  a b c d e f g h\n");

    fflush(stdout);
}

void printGame(Game* game) {
    // printf("Game -> %p (%lu)", game, sizeof(*game));
    // printBoard(&(game->position.board));
    // printf("board -> %p (%lu)\n", &(game->position.board), sizeof(game->position.board));
    // printf("toMove = %d -> %p (%lu)\n", game->position.toMove, &game->position.toMove, sizeof(game->position.toMove));
    // printf("ep = %d -> %p (%lu)\n", game->position.epSquare, &game->position.epSquare, sizeof(game->position.epSquare));
    // printf("castle rights = %d -> %p (%lu)\n", game->position.castlingRights, &game->position.castlingRights, sizeof(game->position.castlingRights));
    // printf("half clock = %d -> %p (%lu)\n", game->position.halfmoveClock, &game->position.halfmoveClock, sizeof(game->position.halfmoveClock));
    // printf("full num = %d -> %p (%lu)\n", game->position.fullmoveNumber, &game->position.fullmoveNumber, sizeof(game->position.fullmoveNumber));

    // printf("moveListLen = %d -> %p (%lu)\n", game->moveListLen, &game->moveListLen, sizeof(game->moveListLen));
    // printf("moveList -> %p (%lu)\n", game->moveList, sizeof(game->moveList));
    // printf("positionHistory -> %p (%lu)\n", game->positionHistory, sizeof(game->positionHistory));
    // fflush(stdout);
}

Bitboard not(Bitboard bb) {
    return ~bb & ALL_SQUARES;
}

char opponent(char color) {
    switch(color) {
    case WHITE:
        return BLACK;
    case BLACK:
        return WHITE;
    }
    return -1;
}

int countBits(Bitboard bb) {
    int bitCount = 0;
    Bitboard position = 1;

    for(int i = 0; i < NUM_SQUARES; i++) {
        if(position & bb) {
            bitCount++;
        }
        position = position << 1;
    }

    return bitCount;
}

void sortNodes(Node* sortedNodes, Node* nodes, int len, char color) {
    Node nodeBuffer[len];

    int i, j;
    BOOL sorted;
    for(i = 0; i < len; i++) {
        sorted = FALSE;

        for(j = 0; j < i; j++) {
            if((color == WHITE && nodes[i].score > sortedNodes[j].score) ||
               (color == BLACK && nodes[i].score < sortedNodes[j].score)) {
                sorted = TRUE;
                memcpy(nodeBuffer, &sortedNodes[j], (i - j) * sizeof(Node));
                memcpy(&sortedNodes[j + 1], nodeBuffer, (i - j) * sizeof(Node));
                sortedNodes[j] = nodes[i];
                break;
            }
        }

        if(sorted == FALSE) {
            sortedNodes[i] = nodes[i];
        }
    }
}

void printMove(Move move) {
    printf(
        "%c%c to %c%c",
        getFile(getFrom(move)),
        getRank(getFrom(move)),
        getFile(getTo(move)),
        getRank(getTo(move)));
}

void printFullMove(Move move, Board* board) {
    Bitboard leavingBB = index2bb(getFrom(move));
    printf("%s from ", bb2str(leavingBB, board));
    printMove(move);
}

void printLegalMoves(Position* position) {
    int i;
    Move moves[MAX_BRANCHING_FACTOR];
    int moveCount = legalMoves(moves, position, position->toMove);
    for(i = 0; i < moveCount; i++) {
        printf("%2d. ", i + 1);
        printFullMove(moves[i], &(position->board));
        //        printMove(moves[i]);
        printf("\n");
    }
    fflush(stdout);
}

void printNode(Node node) {
    printMove(node.move);
    printf(": %d", node.score);
}

void getTimestamp(char* timestamp) {
    time_t timer;
    struct tm* tm_info;

    time(&timer);
    tm_info = localtime(&timer);

    strftime(timestamp, 20, "%Y-%m-%d_%H.%M.%S", tm_info);
}

void dumpContent(Game* game) {
    char* movelist = movelist2str(game);

    char filename[50];
    sprintf(filename, "chess_game_");
    getTimestamp(&filename[strlen(filename)]);
    sprintf(&filename[strlen(filename)], ".txt");

    FILE* file = fopen(filename, "w+");

    fprintf(file, "movelist = %s\nposition history:\n", movelist);

    int i;
    for(i = 0; i < game->moveListLen + 1; i++) fprintf(file, "%s\n", game->positionHistory[i]);

    free(movelist);
    fclose(file);

    printf("Dumped game content to: %s\n", filename);
    fflush(stdout);
}

void dumpPGN(Game* game, char color, BOOL hasAI) {
    char filename[50];
    sprintf(filename, "chess_game_");
    getTimestamp(&filename[strlen(filename)]);
    sprintf(&filename[strlen(filename)], ".pgn");

    FILE* file = fopen(filename, "w+");

    char date[12];
    time_t timer;
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(date, 11, "%Y.%m.%d", tm_info);

    fprintf(file, "[Event \"Casual Game\"]\n");
    fprintf(file, "[Site \"?\"]\n");
    fprintf(file, "[Date \"%s\"]\n", date);
    fprintf(file, "[Round \"-\"]\n");

    if(hasAI) {
        if(color == WHITE) {
            fprintf(file, "[White \"%s\"]\n", HUMAN_NAME);
            fprintf(file, "[Black \"%s\"]\n", ENGINE_NAME);
        } else {
            fprintf(file, "[White \"%s\"]\n", ENGINE_NAME);
            fprintf(file, "[Black \"%s\"]\n", HUMAN_NAME);
        }
    } else {
        fprintf(file, "[White \"Unknown Human Player\"]\n");
        fprintf(file, "[Black \"Unknown Human Player\"]\n");
    }

    if(hasGameEnded(&game->position)) {
        if(endNodeEvaluation(&game->position) == winScore(WHITE)) {
            fprintf(file, "[Result \"1-0\"]\n");
        } else if(endNodeEvaluation(&game->position) == winScore(BLACK)) {
            fprintf(file, "[Result \"0-1\"]\n");
        } else if(endNodeEvaluation(&game->position) == 0) {
            fprintf(file, "[Result \"1/2-1/2\"]\n");
        }
    } else {
        fprintf(file, "[Result \"*\"]\n");
    }

    if(strcmp(game->positionHistory[0], INITIAL_FEN) == 0) {
        fprintf(file, "[Variant \"Standard\"]\n");
    } else {
        fprintf(file, "[Variant \"From Position\"]\n");
        fprintf(file, "[FEN \"%s\"]\n", game->positionHistory[0]);
    }

    fprintf(file, "[PlyCount \"%d\"]\n\n", game->moveListLen);

    int i;
    char ply[8];
    for(i = 0; i < game->moveListLen; i++) {
        if(i % 2 == 0) fprintf(file, "%d. ", 1 + (i / 2));
        move2str(ply, game, i);
        fprintf(file, "%s ", ply);
    }

    fclose(file);

    printf("Dumped game pgn to: %s\n", filename);
    fflush(stdout);
}

void move2str(char* str, Game* game, int moveNumber) {
    Position posBefore, posAfter;
    loadFen(&posBefore, game->positionHistory[moveNumber]);
    loadFen(&posAfter, game->positionHistory[moveNumber + 1]);
    Move move = game->moveList[moveNumber];
    int leavingSquare = getFrom(move);
    int arrivingSquare = getTo(move);

    Bitboard leavingBB = index2bb(leavingSquare);
    Bitboard arrivingBB = index2bb(arrivingSquare);

    int length = 0;
    if((leavingBB & (posBefore.board.whiteKing | posBefore.board.blackKing)) &&
       abs(leavingSquare - arrivingSquare) == 2) { // if castling
        if(arrivingBB & FILE_G) {
            sprintf(str, "O-O");
            length += 3;
        } else if(arrivingBB & FILE_C) {
            sprintf(str, "O-O-O");
            length += 5;
        }
    } else { // if not castling
        if(leavingBB & (posBefore.board.whitePawns | posBefore.board.blackPawns)) {
            if(arrivingBB & getOccupiedSquares(&(posBefore.board))) {
                str[length++] = getFile(leavingSquare);
            }
        } else {
            str[length++] = bb2char(leavingBB, &(posBefore.board));
        }

        if(isAmbiguous(&posBefore, move)) {
            if(countBits(getTwinPieces(leavingBB, &(posBefore.board)) & fileFilter(leavingBB)) ==
               1) {
                str[length++] = getFile(leavingSquare);
            } else {
                str[length++] = getRank(leavingSquare);
            }
        }

        if(bb2piece(arrivingBB, &(posBefore.board)) != EMPTY) {
            str[length++] = 'x';
        }

        str[length++] = getFile(arrivingSquare);
        str[length++] = getRank(arrivingSquare);
    }

    if(isCheckmate(&posAfter)) {
        str[length++] = '#';
    } else if(isCheck(&(posAfter.board), posAfter.toMove)) {
        str[length++] = '+';
    }

    str[length++] = 0;
}

BOOL isAmbiguous(Position* position, Move move) {
    Move moves[MAX_BRANCHING_FACTOR];
    int moveCount = legalMoves(moves, position, position->toMove);
    Bitboard targetBB = index2bb(getTo(move));
    Bitboard attackerBB = index2bb(getFrom(move));
    int attackerPiece = bb2piece(attackerBB, &(position->board));

    int attackerCount = 0;
    for(int i = 0; i < moveCount; i++) {
        Bitboard tgtBB = index2bb(getTo(moves[i]));
        Bitboard atkBB = index2bb(getFrom(moves[i]));

        if(attackerPiece == bb2piece(atkBB, &(position->board)) && (targetBB & tgtBB)) {
            attackerCount++;
        }
    }

    return attackerCount > 1;
}

unsigned long hashPosition(Position* position) {
    char fen[MAX_FEN_LEN];
    toMinFen(fen, position);

    unsigned long hash = 5381;
    int c, i = 0;

    while((c = fen[i++])) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

void writeToHashFile(Position* position, int evaluation, int depth) {
    FILE* fp = fopen("hashfile", "a");

    if(fp == NULL) return;

    char fen[MAX_FEN_LEN];
    toMinFen(fen, position);

    fprintf(fp, "%08lx %d %d %s\n", hashPosition(position), depth, evaluation, fen);
    fclose(fp);
}

// ====== BOARD FILTERS ======

Bitboard getColoredPieces(Board* board, char color) {
    if(color == WHITE) {
        return board->whiteKing | board->whiteQueens | board->whiteRooks | board->whiteKnights |
               board->whiteBishops | board->whitePawns;
    } else {
        return board->blackKing | board->blackQueens | board->blackRooks | board->blackKnights |
               board->blackBishops | board->blackPawns;
    }
}

Bitboard getEmptySquares(Board* board) {
    return not(getOccupiedSquares(board));
}

Bitboard getOccupiedSquares(Board* board) {
    return getColoredPieces(board, WHITE) | getColoredPieces(board, BLACK);
}

Bitboard getTwinPieces(Bitboard position, Board* board) {
    if(position & board->whiteKing) return board->whiteKing;
    if(position & board->whiteQueens) return board->whiteQueens;
    if(position & board->whiteRooks) return board->whiteRooks;
    if(position & board->whiteKnights) return board->whiteKnights;
    if(position & board->whiteBishops) return board->whiteBishops;
    if(position & board->whitePawns) return board->whitePawns;

    if(position & board->blackKing) return board->blackKing;
    if(position & board->blackQueens) return board->blackQueens;
    if(position & board->blackRooks) return board->blackRooks;
    if(position & board->blackKnights) return board->blackKnights;
    if(position & board->blackBishops) return board->blackBishops;
    if(position & board->blackPawns) return board->blackPawns;

    return position;
}

Bitboard fileFilter(Bitboard positions) {
    Bitboard filter = 0;
    int i;

    for(i = 0; i < 8; i++)
        if(positions & FILES_BB[i]) filter |= FILES_BB[i];
    return filter;
}

Bitboard rankFilter(Bitboard positions) {
    Bitboard filter = 0;
    int i;

    for(i = 0; i < 8; i++)
        if(positions & RANKS_BB[i]) filter |= RANKS_BB[i];
    return filter;
}

// ======= DIRECTIONS ========

Bitboard east(Bitboard bb) {
    return (bb << 1) & 0xfefefefefefefefe; // not(FILE_A)
}

Bitboard west(Bitboard bb) {
    return (bb >> 1) & 0x7f7f7f7f7f7f7f7f; // not(FILE_H)
}

Bitboard north(Bitboard bb) {
    return (bb << 8) & 0xffffffffffffff00; // not(RANK_1)
}

Bitboard south(Bitboard bb) {
    return (bb >> 8) & 0x00ffffffffffffff; // not(RANK_8)
}

Bitboard NE(Bitboard bb) {
    return (bb << 9) & 0xfefefefefefefe00; // not(RANK_1 | FILE_A)
}

Bitboard NW(Bitboard bb) {
    return (bb << 7) & 0x7f7f7f7f7f7f7f00; // not(RANK_1 | FILE_H)
}

Bitboard SE(Bitboard bb) {
    return (bb >> 7) & 0x00fefefefefefefe; // not(RANK_8 | FILE_A)
}

Bitboard SW(Bitboard bb) {
    return (bb >> 9) & 0x007f7f7f7f7f7f7f; // not(RANK_8 | FILE_H)
}

Bitboard WNW(Bitboard moving_piece) {
    return (moving_piece << 6) & 0x3f3f3f3f3f3f3f00; // not(RANK_1 | FILE_G | FILE_H)
}

Bitboard ENE(Bitboard moving_piece) {
    return (moving_piece << 10) & 0xfcfcfcfcfcfcfc00; // not(RANK_1 | FILE_A | FILE_B)
}

Bitboard NNW(Bitboard moving_piece) {
    return (moving_piece << 15) & 0x7f7f7f7f7f7f0000; // not(RANK_1 | RANK_2 | FILE_H)
}

Bitboard NNE(Bitboard moving_piece) {
    return (moving_piece << 17) & 0xfefefefefefe0000; // not(RANK_1 | RANK_2 | FILE_A)
}

Bitboard ESE(Bitboard moving_piece) {
    return (moving_piece >> 6) & 0x00fcfcfcfcfcfcfc; // not(RANK_8| FILE_A | FILE_B)
}

Bitboard WSW(Bitboard moving_piece) {
    return (moving_piece >> 10) & 0x003f3f3f3f3f3f3f; // not(RANK_8 | FILE_G | FILE_H)
}

Bitboard SSE(Bitboard moving_piece) {
    return (moving_piece >> 15) & 0x0000fefefefefefe; // not(RANK_7 | RANK_8 | FILE_A)
}

Bitboard SSW(Bitboard moving_piece) {
    return (moving_piece >> 17) & 0x00007f7f7f7f7f7f; // not(RANK_7 | RANK_8 | FILE_H)
}

// ========== PAWN ===========

Bitboard getPawns(Board* board) {
    return board->whitePawns | board->blackPawns;
}

Bitboard pawnSimplePushes(Bitboard moving_piece, Board* board, char color) {
    switch(color) {
    case WHITE:
        return north(moving_piece) & getEmptySquares(board);
    case BLACK:
        return south(moving_piece) & getEmptySquares(board);
    }
    return 0;
}

Bitboard pawnDoublePushes(Bitboard moving_piece, Board* board, char color) {
    switch(color) {
    case WHITE:
        return north(pawnSimplePushes(moving_piece, board, color)) &
               (getEmptySquares(board) & RANK_4);
    case BLACK:
        return south(pawnSimplePushes(moving_piece, board, color)) &
               (getEmptySquares(board) & RANK_5);
    }
    return 0;
}

Bitboard pawnPushes(Bitboard moving_piece, Board* board, char color) {
    return pawnSimplePushes(moving_piece, board, color) |
           pawnDoublePushes(moving_piece, board, color);
}

Bitboard pawnEastAttacks(Bitboard moving_piece, Board* board, char color) {
    switch(color) {
    case WHITE:
        return NE(moving_piece);
    case BLACK:
        return SE(moving_piece);
    }
    return 0;
}

Bitboard pawnWestAttacks(Bitboard moving_piece, Board* board, char color) {
    switch(color) {
    case WHITE:
        return NW(moving_piece);
    case BLACK:
        return SW(moving_piece);
    }
    return 0;
}

Bitboard pawnAttacks(Bitboard moving_piece, Board* board, char color) {
    return pawnEastAttacks(moving_piece, board, color) |
           pawnWestAttacks(moving_piece, board, color);
}

Bitboard pawnSimpleCaptures(Bitboard moving_piece, Board* board, char color) {
    return pawnAttacks(moving_piece, board, color) & getColoredPieces(board, opponent(color));
}

Bitboard pawnEpCaptures(Bitboard moving_piece, Position* position, char color) {
    if(position->epSquare == -1) return 0;

    Bitboard valid_ep_square = 0;

    switch(color) {
    case WHITE:
        valid_ep_square = index2bb(position->epSquare) & RANK_6;
        break;
    case BLACK:
        valid_ep_square = index2bb(position->epSquare) & RANK_3;
        break;
    }

    return pawnAttacks(moving_piece, &(position->board), color) & valid_ep_square;
}

Bitboard pawnCaptures(Bitboard moving_piece, Position* position, char color) {
    return pawnSimpleCaptures(moving_piece, &(position->board), color) |
           pawnEpCaptures(moving_piece, position, color);
}

Bitboard pawnMoves(Bitboard moving_piece, Position* position, char color) {
    return pawnPushes(moving_piece, &(position->board), color) |
           pawnCaptures(moving_piece, position, color);
}

BOOL isDoublePush(int leaving, int arriving) {
    if((index2bb(leaving) & RANK_2) && (index2bb(arriving) & RANK_4)) return TRUE;
    if((index2bb(leaving) & RANK_7) && (index2bb(arriving) & RANK_5)) return TRUE;
    return FALSE;
}

char getEpSquare(int leaving) {
    if(index2bb(leaving) & RANK_2) return leaving + 8;
    if(index2bb(leaving) & RANK_7) return leaving - 8;
    return -1;
}

BOOL isDoubledPawn(Bitboard position, Board* board, char color) {
    if(color == WHITE) {
        if(countBits(board->whitePawns & fileFilter(position)) > 1) return TRUE;
    } else {
        if(countBits(board->blackPawns & fileFilter(position)) > 1) return TRUE;
    }

    return FALSE;
}

BOOL isIsolatedPawn(Bitboard position, Board* board, char color) {
    Bitboard sideFiles = fileFilter(east(position) | west(position));

    if(color == WHITE) {
        if(countBits(board->whitePawns & sideFiles) == 0) return TRUE;
    } else {
        if(countBits(board->blackPawns & sideFiles) == 0) return TRUE;
    }

    return FALSE;
}

BOOL isBackwardsPawn(Bitboard position, Board* board, char color) {
    Bitboard squaresFilter = east(position) | west(position);

    if(color == BLACK) {
        squaresFilter |= northRay(squaresFilter);
        if(countBits(board->blackPawns & squaresFilter) == 0) return TRUE;
    } else {
        squaresFilter |= southRay(squaresFilter);
        if(countBits(board->whitePawns & squaresFilter) == 0) return TRUE;
    }

    return FALSE;
}

BOOL isPassedPawn(Bitboard position, Board* board, char color) {
    Bitboard squaresFilter = 0;

    if(color == BLACK) {
        squaresFilter |= southRay(east(position)) | southRay(west(position)) | southRay(position);
        if(countBits(board->whitePawns & squaresFilter) == 0) return TRUE;
    } else {
        squaresFilter |= northRay(east(position)) | northRay(west(position)) | northRay(position);
        if(countBits(board->blackPawns & squaresFilter) == 0) return TRUE;
    }

    return FALSE;
}

BOOL isOpenFile(Bitboard position, Board* board) {
    if(countBits(getPawns(board) & fileFilter(position)) == 0) return TRUE;
    return FALSE;
}

BOOL isSemiOpenFile(Bitboard position, Board* board) {
    if(countBits(getPawns(board) & fileFilter(position)) == 1) return TRUE;
    return FALSE;
}

// ========== KNIGHT =========

Bitboard getKnights(Board* board) {
    return board->whiteKnights | board->blackKnights;
}

Bitboard knightAttacks(Bitboard moving_piece) {
    return NNE(moving_piece) | ENE(moving_piece) | NNW(moving_piece) | WNW(moving_piece) |
           SSE(moving_piece) | ESE(moving_piece) | SSW(moving_piece) | WSW(moving_piece);
}

Bitboard knightMoves(Bitboard moving_piece, Board* board, char color) {
    return knightAttacks(moving_piece) & not(getColoredPieces(board, color));
}

Bitboard knightFill(Bitboard moving_piece, int jumps) {
    Bitboard fill = moving_piece;
    int i;
    for(i = 0; i < jumps; i++) {
        fill |= knightAttacks(fill);
    }
    return fill;
}

int knightDistance(Bitboard leaving_square, Bitboard arriving_square) {
    Bitboard fill = leaving_square;
    int dist = 0;

    while((fill & arriving_square) == 0) {
        dist++;
        fill |= knightAttacks(fill);
    }
    return dist;
}

// ========== KING ===========

Bitboard getKing(Board* board, char color) {
    if(color == WHITE) {
        return board->whiteKing;
    } else {
        return board->blackKing;
    }
}

Bitboard kingAttacks(Bitboard moving_piece) {
    Bitboard kingAtks = moving_piece | east(moving_piece) | west(moving_piece);
    kingAtks |= north(kingAtks) | south(kingAtks);
    return kingAtks & not(moving_piece);
}

Bitboard kingMoves(Bitboard moving_piece, Board* board, char color) {
    return kingAttacks(moving_piece) & not(getColoredPieces(board, color));
}

BOOL canCastleKingside(Position* position, char color) {
    Bitboard emptyPositions = getEmptySquares(&(position->board));

    switch(color) {
    case WHITE:
        if((position->castlingRights & CASTLE_KINGSIDE_WHITE) &&
           (FILE_E & RANK_1 & position->board.whiteKing) && (FILE_F & RANK_1 & emptyPositions) &&
           (FILE_G & RANK_1 & emptyPositions) && (FILE_H & RANK_1 & position->board.whiteRooks) &&
           (!isAttacked((FILE_E & RANK_1), &(position->board), opponent(color))) &&
           (!isAttacked((FILE_F & RANK_1), &(position->board), opponent(color))) &&
           (!isAttacked((FILE_G & RANK_1), &(position->board), opponent(color))))
            return TRUE;
        else
            return FALSE;

    case BLACK:
        if((position->castlingRights & CASTLE_KINGSIDE_BLACK) &&
           (FILE_E & RANK_8 & position->board.blackKing) && (FILE_F & RANK_8 & emptyPositions) &&
           (FILE_G & RANK_8 & emptyPositions) && (FILE_H & RANK_8 & position->board.blackRooks) &&
           (!isAttacked((FILE_E & RANK_8), &(position->board), opponent(color))) &&
           (!isAttacked((FILE_F & RANK_8), &(position->board), opponent(color))) &&
           (!isAttacked((FILE_G & RANK_8), &(position->board), opponent(color))))
            return TRUE;
        else
            return FALSE;
    }
    return FALSE;
}

BOOL canCastleQueenside(Position* position, char color) {
    Bitboard emptyPositions = getEmptySquares(&(position->board));

    switch(color) {
    case WHITE:
        if((position->castlingRights & CASTLE_QUEENSIDE_WHITE) &&
           (FILE_A & RANK_1 & position->board.whiteRooks) && (FILE_B & RANK_1 & emptyPositions) &&
           (FILE_C & RANK_1 & emptyPositions) && (FILE_D & RANK_1 & emptyPositions) &&
           (FILE_E & RANK_1 & position->board.whiteKing) &&
           (!isAttacked((FILE_C & RANK_1), &(position->board), opponent(color))) &&
           (!isAttacked((FILE_D & RANK_1), &(position->board), opponent(color))) &&
           (!isAttacked((FILE_E & RANK_1), &(position->board), opponent(color))))
            return TRUE;
        else
            return FALSE;

    case BLACK:
        if((position->castlingRights & CASTLE_QUEENSIDE_BLACK) &&
           (FILE_A & RANK_8 & position->board.blackRooks) && (FILE_B & RANK_8 & emptyPositions) &&
           (FILE_C & RANK_8 & emptyPositions) && (FILE_D & RANK_8 & emptyPositions) &&
           (FILE_E & RANK_8 & position->board.blackKing) &&
           (!isAttacked((FILE_C & RANK_8), &(position->board), opponent(color))) &&
           (!isAttacked((FILE_D & RANK_8), &(position->board), opponent(color))) &&
           (!isAttacked((FILE_E & RANK_8), &(position->board), opponent(color))))
            return TRUE;
        else
            return FALSE;
    }
    return FALSE;
}

char removeCastlingRights(char original_rights, char removed_rights) {
    return (char)(original_rights & ~(removed_rights));
}

// ========== BISHOP =========

Bitboard getBishops(Board* board) {
    return board->whiteBishops | board->blackBishops;
}

Bitboard NE_ray(Bitboard bb) {
    int i;
    Bitboard ray = NE(bb);

    for(i = 0; i < 6; i++) {
        ray |= NE(ray);
    }

    return ray & ALL_SQUARES;
}

Bitboard SE_ray(Bitboard bb) {
    int i;
    Bitboard ray = SE(bb);

    for(i = 0; i < 6; i++) {
        ray |= SE(ray);
    }

    return ray & ALL_SQUARES;
}

Bitboard NW_ray(Bitboard bb) {
    int i;
    Bitboard ray = NW(bb);

    for(i = 0; i < 6; i++) {
        ray |= NW(ray);
    }

    return ray & ALL_SQUARES;
}

Bitboard SW_ray(Bitboard bb) {
    int i;
    Bitboard ray = SW(bb);

    for(i = 0; i < 6; i++) {
        ray |= SW(ray);
    }

    return ray & ALL_SQUARES;
}

Bitboard NE_attack(Bitboard single_piece, Board* board, char color) {
    Bitboard blocker = lsb(NE_ray(single_piece) & getOccupiedSquares(board));
    if(blocker) {
        return NE_ray(single_piece) ^ NE_ray(blocker);
    } else {
        return NE_ray(single_piece);
    }
}

Bitboard NW_attack(Bitboard single_piece, Board* board, char color) {
    Bitboard blocker = lsb(NW_ray(single_piece) & getOccupiedSquares(board));
    if(blocker) {
        return NW_ray(single_piece) ^ NW_ray(blocker);
    } else {
        return NW_ray(single_piece);
    }
}

Bitboard SE_attack(Bitboard single_piece, Board* board, char color) {
    Bitboard blocker = msb(SE_ray(single_piece) & getOccupiedSquares(board));
    if(blocker) {
        return SE_ray(single_piece) ^ SE_ray(blocker);
    } else {
        return SE_ray(single_piece);
    }
}

Bitboard SW_attack(Bitboard single_piece, Board* board, char color) {
    Bitboard blocker = msb(SW_ray(single_piece) & getOccupiedSquares(board));
    if(blocker) {
        return SW_ray(single_piece) ^ SW_ray(blocker);
    } else {
        return SW_ray(single_piece);
    }
}

Bitboard diagonalAttacks(Bitboard single_piece, Board* board, char color) {
    return NE_attack(single_piece, board, color) | SW_attack(single_piece, board, color);
}

Bitboard antiDiagonalAttacks(Bitboard single_piece, Board* board, char color) {
    return NW_attack(single_piece, board, color) | SE_attack(single_piece, board, color);
}

Bitboard bishopAttacks(Bitboard moving_pieces, Board* board, char color) {
    return diagonalAttacks(moving_pieces, board, color) |
           antiDiagonalAttacks(moving_pieces, board, color);
}

Bitboard bishopMoves(Bitboard moving_piece, Board* board, char color) {
    return bishopAttacks(moving_piece, board, color) & not(getColoredPieces(board, color));
}

// ========== ROOK ===========

Bitboard getRooks(Board* board) {
    return board->whiteRooks | board->blackRooks;
}

Bitboard northRay(Bitboard moving_pieces) {
    Bitboard ray_atks = north(moving_pieces);

    int i;
    for(i = 0; i < 6; i++) {
        ray_atks |= north(ray_atks);
    }

    return ray_atks & ALL_SQUARES;
}

Bitboard southRay(Bitboard moving_pieces) {
    Bitboard ray_atks = south(moving_pieces);

    int i;
    for(i = 0; i < 6; i++) {
        ray_atks |= south(ray_atks);
    }

    return ray_atks & ALL_SQUARES;
}

Bitboard eastRay(Bitboard moving_pieces) {
    Bitboard ray_atks = east(moving_pieces);

    int i;
    for(i = 0; i < 6; i++) {
        ray_atks |= east(ray_atks);
    }

    return ray_atks & ALL_SQUARES;
}

Bitboard westRay(Bitboard moving_pieces) {
    Bitboard ray_atks = west(moving_pieces);

    int i;
    for(i = 0; i < 6; i++) {
        ray_atks |= west(ray_atks);
    }

    return ray_atks & ALL_SQUARES;
}

Bitboard northAttack(Bitboard single_piece, Board* board, char color) {
    Bitboard blocker = lsb(northRay(single_piece) & getOccupiedSquares(board));

    if(blocker)
        return northRay(single_piece) ^ northRay(blocker);
    else
        return northRay(single_piece);
}

Bitboard southAttack(Bitboard single_piece, Board* board, char color) {
    Bitboard blocker = msb(southRay(single_piece) & getOccupiedSquares(board));

    if(blocker)
        return southRay(single_piece) ^ southRay(blocker);
    else
        return southRay(single_piece);
}

Bitboard fileAttacks(Bitboard single_piece, Board* board, char color) {
    return northAttack(single_piece, board, color) | southAttack(single_piece, board, color);
}

Bitboard eastAttack(Bitboard single_piece, Board* board, char color) {
    Bitboard blocker = lsb(eastRay(single_piece) & getOccupiedSquares(board));

    if(blocker)
        return eastRay(single_piece) ^ eastRay(blocker);
    else
        return eastRay(single_piece);
}

Bitboard westAttack(Bitboard single_piece, Board* board, char color) {
    Bitboard blocker = msb(westRay(single_piece) & getOccupiedSquares(board));

    if(blocker)
        return westRay(single_piece) ^ westRay(blocker);
    else
        return westRay(single_piece);
}

Bitboard rankAttacks(Bitboard single_piece, Board* board, char color) {
    return eastAttack(single_piece, board, color) | westAttack(single_piece, board, color);
}

Bitboard rookAttacks(Bitboard moving_piece, Board* board, char color) {
    return fileAttacks(moving_piece, board, color) | rankAttacks(moving_piece, board, color);
}

Bitboard rookMoves(Bitboard moving_piece, Board* board, char color) {
    return rookAttacks(moving_piece, board, color) & not(getColoredPieces(board, color));
}

// ========== QUEEN ==========

Bitboard getQueens(Board* board) {
    return board->whiteQueens | board->blackQueens;
}

Bitboard queenAttacks(Bitboard moving_piece, Board* board, char color) {
    return rookAttacks(moving_piece, board, color) | bishopAttacks(moving_piece, board, color);
}

Bitboard queenMoves(Bitboard moving_piece, Board* board, char color) {
    return rookMoves(moving_piece, board, color) | bishopMoves(moving_piece, board, color);
}

// ======== MAKE MOVE ========

void clearPositions(Board* board, Bitboard positions) {
    board->whiteKing = board->whiteKing & not(positions);
    board->whiteQueens = board->whiteQueens & not(positions);
    board->whiteRooks = board->whiteRooks & not(positions);
    board->whiteKnights = board->whiteKnights & not(positions);
    board->whiteBishops = board->whiteBishops & not(positions);
    board->whitePawns = board->whitePawns & not(positions);
    board->blackKing = board->blackKing & not(positions);
    board->blackQueens = board->blackQueens & not(positions);
    board->blackRooks = board->blackRooks & not(positions);
    board->blackKnights = board->blackKnights & not(positions);
    board->blackBishops = board->blackBishops & not(positions);
    board->blackPawns = board->blackPawns & not(positions);
}

void movePiece(Board* board, Move move) {
    Bitboard leaving = index2bb(getFrom(move));
    Bitboard arriving = index2bb(getTo(move));

    if(leaving & board->whiteKing) {
        clearPositions(board, arriving | leaving);
        board->whiteKing = arriving;
    } else if(leaving & board->whiteQueens) {
        clearPositions(board, arriving | leaving);
        board->whiteQueens = board->whiteQueens | arriving;
    } else if(leaving & board->whiteRooks) {
        clearPositions(board, arriving | leaving);
        board->whiteRooks = board->whiteRooks | arriving;
    } else if(leaving & board->whiteKnights) {
        clearPositions(board, arriving | leaving);
        board->whiteKnights = board->whiteKnights | arriving;
    } else if(leaving & board->whiteBishops) {
        clearPositions(board, arriving | leaving);
        board->whiteBishops = board->whiteBishops | arriving;
    } else if(leaving & board->whitePawns) {
        clearPositions(board, arriving | leaving);
        board->whitePawns = board->whitePawns | arriving;
    } else if(leaving & board->blackKing) {
        clearPositions(board, arriving | leaving);
        board->blackKing = arriving;
    } else if(leaving & board->blackQueens) {
        clearPositions(board, arriving | leaving);
        board->blackQueens = board->blackQueens | arriving;
    } else if(leaving & board->blackRooks) {
        clearPositions(board, arriving | leaving);
        board->blackRooks = board->blackRooks | arriving;
    } else if(leaving & board->blackKnights) {
        clearPositions(board, arriving | leaving);
        board->blackKnights = board->blackKnights | arriving;
    } else if(leaving & board->blackBishops) {
        clearPositions(board, arriving | leaving);
        board->blackBishops = board->blackBishops | arriving;
    } else if(leaving & board->blackPawns) {
        clearPositions(board, arriving | leaving);
        board->blackPawns = board->blackPawns | arriving;
    }
}

void updatePosition(Position* newPosition, Position* position, Move move) {
    memcpy(newPosition, position, sizeof(Position));
    int leavingSquare = getFrom(move);
    int arrivingSquare = getTo(move);
    Bitboard leavingBB = index2bb(leavingSquare);
    Bitboard arrivingBB = index2bb(arrivingSquare);

    // ===== MOVE PIECE =====
    movePiece(&(newPosition->board), move);

    // ===== TO MOVE =====
    newPosition->toMove = opponent(position->toMove);

    // ===== MOVE COUNTS =====
    newPosition->halfmoveClock += 1;
    if(position->toMove == BLACK) {
        newPosition->fullmoveNumber += 1;
    }

    if(arrivingBB & getOccupiedSquares(&(position->board))) {
        newPosition->halfmoveClock = 0;
    }

    // ===== PAWNS =====
    newPosition->epSquare = -1;
    if((leavingBB & position->board.whitePawns) | (leavingBB & position->board.blackPawns)) {
        newPosition->halfmoveClock = 0;

        if(arrivingSquare == position->epSquare) {
            if(index2bb(position->epSquare) & RANK_3) {
                clearPositions(&(newPosition->board), index2bb((int)(position->epSquare + 8)));
            }

            if(index2bb(position->epSquare) & RANK_6) {
                clearPositions(&(newPosition->board), index2bb((int)(position->epSquare - 8)));
            }
        }

        if(isDoublePush(leavingSquare, arrivingSquare)) {
            newPosition->epSquare = getEpSquare(leavingSquare);
        }

        if(arrivingBB & (RANK_1 | RANK_8)) {
            clearPositions(&(newPosition->board), arrivingBB);

            if(position->toMove == WHITE) {
                newPosition->board.whiteQueens = newPosition->board.whiteQueens | arrivingBB;
            } else {
                newPosition->board.blackQueens = newPosition->board.blackQueens | arrivingBB;
            }
        }
    }

    // ===== CASTLING =====
    if(leavingSquare == str2index("a1")) {
        newPosition->castlingRights =
            removeCastlingRights(newPosition->castlingRights, CASTLE_QUEENSIDE_WHITE);
    } else if(leavingSquare == str2index("h1")) {
        newPosition->castlingRights =
            removeCastlingRights(newPosition->castlingRights, CASTLE_KINGSIDE_WHITE);
    } else if(leavingSquare == str2index("a8")) {
        newPosition->castlingRights =
            removeCastlingRights(newPosition->castlingRights, CASTLE_QUEENSIDE_BLACK);
    } else if(leavingSquare == str2index("h8")) {
        newPosition->castlingRights =
            removeCastlingRights(newPosition->castlingRights, CASTLE_KINGSIDE_BLACK);
    }

    if(leavingBB & position->board.whiteKing) {
        newPosition->castlingRights = removeCastlingRights(
            newPosition->castlingRights, (CASTLE_KINGSIDE_WHITE | CASTLE_QUEENSIDE_WHITE));
        if(leavingSquare == str2index("e1")) {
            if(arrivingSquare == str2index("g1"))
                movePiece(&(newPosition->board), generateMove(str2index("h1"), str2index("f1")));

            if(arrivingSquare == str2index("c1"))
                movePiece(&(newPosition->board), generateMove(str2index("a1"), str2index("d1")));
        }
    } else if(leavingBB & position->board.blackKing) {
        newPosition->castlingRights = removeCastlingRights(
            newPosition->castlingRights, CASTLE_KINGSIDE_BLACK | CASTLE_QUEENSIDE_BLACK);
        if(leavingSquare == str2index("e8")) {
            if(arrivingSquare == str2index("g8"))
                movePiece(&(newPosition->board), generateMove(str2index("h8"), str2index("f8")));

            if(arrivingSquare == str2index("c8"))
                movePiece(&(newPosition->board), generateMove(str2index("a8"), str2index("d8")));
        }
    }
}

void makeMove(Game* game, Move move) {
    Position newPosition;
    updatePosition(&newPosition, &(game->position), move);
    memcpy(&(game->position), &newPosition, sizeof(Position));

    game->moveListLen += 1;

    // ===== MOVE LIST =====
    game->moveList[game->moveListLen - 1] = move;

    // ===== POSITION HISTORY =====
    toFen(game->positionHistory[game->moveListLen], &game->position);
}

void unmakeMove(Game* game) {
    Position newPosition;
    if(game->moveListLen >= 1) {
        loadFen(&newPosition, game->positionHistory[game->moveListLen - 1]);
        memcpy(&(game->position), &newPosition, sizeof(Position));

        game->moveList[game->moveListLen - 1] = 0;
        memset(game->positionHistory[game->moveListLen], 0, MAX_FEN_LEN * sizeof(char));

        game->moveListLen -= 1;
    } else { // return to initial game
        loadFen(&newPosition, game->positionHistory[0]);
        memcpy(&(game->position), &newPosition, sizeof(Position));

        game->moveListLen = 0;
        memset(game->moveList, 0, MAX_PLYS_PER_GAME * sizeof(int));
        memset(&game->positionHistory[1], 0, (MAX_PLYS_PER_GAME - 1) * MAX_FEN_LEN * sizeof(char));
    }
}

// ======== MOVE GEN =========

Bitboard getMoves(Bitboard movingPiece, Position* position, char color) {
    if((movingPiece & position->board.whitePawns) | (movingPiece & position->board.blackPawns))
        return pawnMoves(movingPiece, position, color);
    if((movingPiece & position->board.whiteKnights) | (movingPiece & position->board.blackKnights))
        return knightMoves(movingPiece, &(position->board), color);
    if((movingPiece & position->board.whiteBishops) | (movingPiece & position->board.blackBishops))
        return bishopMoves(movingPiece, &(position->board), color);
    if((movingPiece & position->board.whiteRooks) | (movingPiece & position->board.blackRooks))
        return rookMoves(movingPiece, &(position->board), color);
    if((movingPiece & position->board.whiteQueens) | (movingPiece & position->board.blackQueens))
        return queenMoves(movingPiece, &(position->board), color);
    if((movingPiece & position->board.whiteKing) | (movingPiece & position->board.blackKing))
        return kingMoves(movingPiece, &(position->board), color);

    return 0;
}

int pseudoLegalMoves(Move* moves, Position* position, char color) {
    int leavingSquare, arrivingSquare, moveCount = 0;
    Bitboard leavingBB = 1;
    Bitboard attackers = getColoredPieces(&(position->board), color);

    for(leavingSquare = 0; leavingSquare < NUM_SQUARES; leavingSquare++) {
        if(leavingBB & attackers) {
            Bitboard targets = getMoves(leavingBB, position, color);

            for(arrivingSquare = 0; arrivingSquare < NUM_SQUARES; arrivingSquare++) {
                if(isSet(targets, arrivingSquare)) {
                    moves[moveCount++] = generateMove(leavingSquare, arrivingSquare);
                }
            }

            if((leavingBB & position->board.whiteKing) | (leavingBB & position->board.blackKing)) {
                if(canCastleKingside(position, color)) {
                    moves[moveCount++] = generateMove(leavingSquare, leavingSquare + 2);
                }
                if(canCastleQueenside(position, color)) {
                    moves[moveCount++] = generateMove(leavingSquare, leavingSquare - 2);
                }
            }
        }
        leavingBB <<= 1;
    }

    return moveCount;
}

Bitboard getAttacks(Bitboard movingPiece, Board* board, char color) {
    if((movingPiece & board->whitePawns) | (movingPiece & board->blackPawns))
        return pawnAttacks(movingPiece, board, color);
    if((movingPiece & board->whiteKnights) | (movingPiece & board->blackKnights))
        return knightAttacks(movingPiece);
    if((movingPiece & board->whiteBishops) | (movingPiece & board->blackBishops))
        return bishopAttacks(movingPiece, board, color);
    if((movingPiece & board->whiteRooks) | (movingPiece & board->blackRooks))
        return rookAttacks(movingPiece, board, color);
    if((movingPiece & board->whiteQueens) | (movingPiece & board->blackQueens))
        return queenAttacks(movingPiece, board, color);
    if((movingPiece & board->whiteKing) | (movingPiece & board->blackKing))
        return kingAttacks(movingPiece);

    return 0;
}

int countAttacks(Bitboard target, Board* board, char color) {
    int i, attackCount = 0;

    Bitboard attackers = getColoredPieces(board, color);
    Bitboard position = 1;

    for(i = 0; i < NUM_SQUARES; i++) {
        if(position & attackers) {
            if(getAttacks(position, board, color) & target) {
                attackCount += 1;
            }
        }
        position = position << 1;
    }

    return attackCount;
}

BOOL isAttacked(Bitboard target, Board* board, char color) {
    int i = 0;

    Bitboard attackers = getColoredPieces(board, color);
    Bitboard position = 1;

    for(i = 0; i < NUM_SQUARES; i++) {
        if(position & attackers) {
            if(getAttacks(position, board, color) & target) {
                return TRUE;
            }
        }
        position = position << 1;
    }

    return FALSE;
}

BOOL isCheck(Board* board, char color) {
    return isAttacked(getKing(board, color), board, opponent(color));
}

BOOL isLegalMove(Position* position, Move move) {
    Position newPosition;
    updatePosition(&newPosition, position, move);
    if(isCheck(&(newPosition.board), position->toMove)) return FALSE;
    return TRUE;
}

int legalMoves(Move* legalMoves, Position* position, char color) {
    int i, legalCount = 0;

    Move pseudoMoves[MAX_BRANCHING_FACTOR];
    int pseudoCount = pseudoLegalMoves(pseudoMoves, position, color);

    for(i = 0; i < pseudoCount; i++) {
        if(isLegalMove(position, pseudoMoves[i])) {
            legalMoves[legalCount++] = pseudoMoves[i];
        }
    }

    return legalCount;
}

int legalMovesCount(Position* position, char color) {
    int i, legalCount = 0;

    Move pseudoMoves[MAX_BRANCHING_FACTOR];
    int pseudoCount = pseudoLegalMoves(pseudoMoves, position, color);

    for(i = 0; i < pseudoCount; i++) {
        if(isLegalMove(position, pseudoMoves[i])) {
            legalCount++;
        }
    }

    return legalCount;
}

int staticOrderLegalMoves(Move* orderedLegalMoves, Position* position, char color) {
    Move moves[MAX_BRANCHING_FACTOR];
    int legalCount = legalMoves(moves, position, color);

    Position newPosition;
    Node nodes[legalCount], orderedNodes[legalCount];

    int i;
    for(i = 0; i < legalCount; i++) {
        updatePosition(&newPosition, position, moves[i]);
        nodes[i] = (Node){.move = moves[i], .score = staticEvaluation(&newPosition)};
    }

    sortNodes(orderedNodes, nodes, legalCount, color);

    for(i = 0; i < legalCount; i++) {
        orderedLegalMoves[i] = orderedNodes[i].move;
    }

    return legalCount;
}

int legalCaptures(Move* legalCaptures, Position* position, char color) {
    int i, captureCount = 0;

    Move moves[MAX_BRANCHING_FACTOR];
    int legalCount = legalMoves(moves, position, color);

    for(i = 0; i < legalCount; i++) {
        int arrivingSquare = getTo(moves[i]);
        if(index2bb(arrivingSquare) & getColoredPieces(&(position->board), opponent(color))) {
            legalCaptures[captureCount++] = moves[i];
        }
    }

    return captureCount;
}

// ====== GAME CONTROL =======

BOOL isCheckmate(Position* position) {
    if(isCheck(&(position->board), position->toMove) &&
       legalMovesCount(position, position->toMove) == 0)
        return TRUE;
    else
        return FALSE;
}

BOOL isStalemate(Position* position) {
    if(!isCheck(&(position->board), position->toMove) &&
       legalMovesCount(position, position->toMove) == 0)
        return TRUE;
    else
        return FALSE;
}

BOOL hasInsufficientMaterial(Board* board) {
    int pieceCount = countBits(getOccupiedSquares(board));

    if(pieceCount <= 3) {
        if(pieceCount == 2 || getKnights(board) != 0 || getBishops(board) != 0) return TRUE;
    }

    return FALSE;
}

BOOL isEndgame(Board* board) {
    if(countBits(getOccupiedSquares(board)) <= ENDGAME_PIECE_COUNT) return TRUE;
    return FALSE;
}

BOOL isOver75MovesRule(Position* position) {
    if(position->halfmoveClock >= 150)
        return TRUE;
    else
        return FALSE;
}

BOOL hasGameEnded(Position* position) {
    if(isCheckmate(position) || isStalemate(position) ||
       hasInsufficientMaterial(&(position->board)) || isOver75MovesRule(position))
        return TRUE;
    else
        return FALSE;
}

void printOutcome(Position* position) {
    if(isCheckmate(position) && position->toMove == BLACK) printf("WHITE wins!\n");
    if(isCheckmate(position) && position->toMove == WHITE) printf("BLACK wins!\n");
    if(isStalemate(position)) printf("Draw by stalemate!\n");
    if(hasInsufficientMaterial(&(position->board))) printf("Draw by insufficient material!\n");
    if(isOver75MovesRule(position)) printf("Draw by 75 move rule!\n");
    fflush(stdout);
}

// ========== EVAL ===========

int winScore(char color) {
    if(color == WHITE) return 10 * PIECE_VALUES[KING];
    if(color == BLACK) return -10 * PIECE_VALUES[KING];
    return 0;
}

int materialSum(Board* board, char color) {
    if(color == WHITE) {
        return PIECE_VALUES[PAWN] * countBits(board->whitePawns) +
               PIECE_VALUES[KNIGHT] * countBits(board->whiteKnights) +
               PIECE_VALUES[BISHOP] * countBits(board->whiteBishops) +
               PIECE_VALUES[ROOK] * countBits(board->whiteRooks) +
               PIECE_VALUES[QUEEN] * countBits(board->whiteQueens);
    } else {
        return PIECE_VALUES[PAWN] * countBits(board->blackPawns) +
               PIECE_VALUES[KNIGHT] * countBits(board->blackKnights) +
               PIECE_VALUES[BISHOP] * countBits(board->blackBishops) +
               PIECE_VALUES[ROOK] * countBits(board->blackRooks) +
               PIECE_VALUES[QUEEN] * countBits(board->blackQueens);
    }
}

int materialBalance(Board* board) {
    return materialSum(board, WHITE) - materialSum(board, BLACK);
}

int positionalBonus(Board* board, char color) {
    int bonus = 0;
    Bitboard positionBB;
    Bitboard coloredPieces = getColoredPieces(board, color);

    int i;
    for(i = 0; i < NUM_SQUARES; i++) {
        positionBB = index2bb(i);

        if(positionBB & coloredPieces) {
            if((positionBB & board->whitePawns) | (positionBB & board->blackPawns)) {
                if(color == WHITE) {
                    bonus += PAWN_BONUS[i];
                } else {
                    bonus += PAWN_BONUS[FLIP_VERTICAL[i]];
                }

                if(isDoubledPawn(positionBB, board, color)) {
                    bonus -= DOUBLED_PAWN_PENALTY / 2;
                }
                if(isPassedPawn(positionBB, board, color)) {
                    bonus += PASSED_PAWN_BONUS;
                }

                if(isIsolatedPawn(positionBB, board, color)) {
                    bonus -= ISOLATED_PAWN_PENALTY;
                } else if(isBackwardsPawn(positionBB, board, color)) {
                    bonus -= BACKWARDS_PAWN_PENALTY;
                }
            } else if((positionBB & board->whiteKnights) | (positionBB & board->blackKnights)) {
                if(color == WHITE) {
                    bonus += KNIGHT_BONUS[i];
                } else {
                    bonus += KNIGHT_BONUS[FLIP_VERTICAL[i]];
                }
            } else if((positionBB & board->whiteBishops) | (positionBB & board->blackBishops)) {
                if(color == WHITE) {
                    bonus += BISHOP_BONUS[i];
                } else {
                    bonus += BISHOP_BONUS[FLIP_VERTICAL[i]];
                }
            } else if((positionBB & board->whiteRooks) | (positionBB & board->blackRooks)) {
                if(isOpenFile(positionBB, board)) {
                    bonus += ROOK_OPEN_FILE_BONUS;
                } else if(isSemiOpenFile(positionBB, board)) {
                    bonus += ROOK_SEMI_OPEN_FILE_BONUS;
                }

                if(color == WHITE) {
                    if(positionBB & RANK_7) {
                        bonus += ROOK_ON_SEVENTH_BONUS;
                    }
                } else {
                    if(positionBB & RANK_2) {
                        bonus += ROOK_ON_SEVENTH_BONUS;
                    }
                }
            } else if((positionBB & board->whiteKing) | (positionBB & board->blackKing)) {
                if(isEndgame(board)) {
                    if(color == WHITE) {
                        bonus += KING_ENDGAME_BONUS[i];
                    } else {
                        bonus += KING_ENDGAME_BONUS[FLIP_VERTICAL[i]];
                    }
                } else {
                    if(color == WHITE) {
                        bonus += KING_BONUS[i];
                    } else {
                        bonus += KING_BONUS[FLIP_VERTICAL[i]];
                    }
                }
            }
        }
    }

    return bonus;
}

int positionalBalance(Board* board) {
    return positionalBonus(board, WHITE) - positionalBonus(board, BLACK);
}

int endNodeEvaluation(Position* position) {
    if(isCheckmate(position)) {
        return winScore(opponent(position->toMove));
    }
    if(isStalemate(position) || hasInsufficientMaterial(&(position->board)) ||
       isOver75MovesRule(position)) {
        return 0;
    }
    return 0;
}

int staticEvaluation(Position* position) {
    if(hasGameEnded(position))
        return endNodeEvaluation(position);
    else
        return materialBalance(&(position->board)) + positionalBalance(&(position->board));
}

int getPieceValue(Bitboard position, Board* board) {
    if((position & board->whitePawns) | (position & board->blackPawns)) return PIECE_VALUES[PAWN];
    if((position & board->whiteKnights) | (position & board->blackKnights))
        return PIECE_VALUES[KNIGHT];
    if((position & board->whiteBishops) | (position & board->blackBishops))
        return PIECE_VALUES[BISHOP];
    if((position & board->whiteRooks) | (position & board->blackRooks)) return PIECE_VALUES[ROOK];
    if((position & board->whiteQueens) | (position & board->blackQueens))
        return PIECE_VALUES[QUEEN];
    if((position & board->whiteKing) | (position & board->blackKing)) return PIECE_VALUES[KING];
    return 0;
}

int getCaptureSequence(Move* captures, Position* position, int targetSquare) {
    Move allCaptures[MAX_BRANCHING_FACTOR], targetCaptures[MAX_ATTACKING_PIECES];
    int captureCount = legalCaptures(allCaptures, position, position->toMove);
    int i, j, targetCount = 0;

    for(i = 0; i < captureCount; i++) {
        if(getTo(allCaptures[i]) == targetSquare) {
            targetCaptures[targetCount++] = allCaptures[i];
        }
    }

    Move captureBuffer[targetCount];

    BOOL sorted;
    for(i = 0; i < targetCount; i++) {
        sorted = FALSE;
        int pieceValue = getPieceValue(index2bb(getFrom(targetCaptures[i])), &(position->board));

        for(j = 0; j < i; j++) {
            int sortedPieceValue =
                getPieceValue(index2bb(getFrom(captures[j])), &(position->board));

            if(pieceValue < sortedPieceValue) {
                sorted = TRUE;
                memcpy(captureBuffer, &captures[j], (i - j) * sizeof(Move));
                memcpy(&captures[j + 1], captureBuffer, (i - j) * sizeof(Move));
                captures[j] = targetCaptures[i];
                break;
            }
        }

        if(sorted == FALSE) {
            captures[i] = targetCaptures[i];
        }
    }

    return targetCount;
}

int staticExchangeEvaluation(Position* position, int targetSquare) {
    Move captures[MAX_ATTACKING_PIECES];
    int attackCount = getCaptureSequence(captures, position, targetSquare);
    int value = 0;

    if(attackCount > 0) {
        Position newPosition;
        updatePosition(&newPosition, position, captures[0]);
        int pieceValue = getPieceValue(index2bb(targetSquare), &(position->board));
        value = pieceValue - staticExchangeEvaluation(&newPosition, targetSquare);
    }

    return value > 0 ? value : 0;
}

int quiescenceEvaluation(Position* position) {
    int staticScore = staticEvaluation(position);

    if(hasGameEnded(position)) return staticScore;

    Move captures[MAX_BRANCHING_FACTOR];
    int captureCount = legalCaptures(captures, position, position->toMove);

    if(captureCount == 0) {
        return staticScore;
    } else {
        Position newPosition;
        int i, bestScore = staticScore;

        for(i = 0; i < captureCount; i++) {
            if(staticExchangeEvaluation(position, getTo(captures[i])) <= 0) continue;

            updatePosition(&newPosition, position, captures[i]);
            int score = quiescenceEvaluation(&newPosition);

            if((position->toMove == WHITE && score > bestScore) ||
               (position->toMove == BLACK && score < bestScore)) {
                bestScore = score;
            }
        }

        return bestScore;
    }
}

// ========= SEARCH ==========

Node staticSearch(Position* position) {
    int bestScore = position->toMove == WHITE ? INT32_MIN : INT32_MAX;
    Move bestMove = 0;

    Move moves[MAX_BRANCHING_FACTOR];
    int moveCount = legalMoves(moves, position, position->toMove);

    Position newPosition;
    int i;
    for(i = 0; i < moveCount; i++) {
        updatePosition(&newPosition, position, moves[i]);
        int score = staticEvaluation(&newPosition);

        if(score == winScore(position->toMove)) {
            return (Node){.move = moves[i], .score = score};
        }

        if((position->toMove == WHITE && score > bestScore) ||
           (position->toMove == BLACK && score < bestScore)) {
            bestScore = score;
            bestMove = moves[i];
        }
    }

    return (Node){.move = bestMove, .score = bestScore};
}

Node quiescenceSearch(Position* position) {
    int bestScore = position->toMove == WHITE ? INT32_MIN : INT32_MAX;
    Move bestMove = 0;

    Move moves[MAX_BRANCHING_FACTOR];
    int moveCount = legalMoves(moves, position, position->toMove);

    Position newPosition;
    int i;
    for(i = 0; i < moveCount; i++) {
        updatePosition(&newPosition, position, moves[i]);
        int score = quiescenceEvaluation(&newPosition);

        if(score == winScore(position->toMove)) {
            return (Node){.move = moves[i], .score = score};
        }

        if((position->toMove == WHITE && score > bestScore) ||
           (position->toMove == BLACK && score < bestScore)) {
            bestScore = score;
            bestMove = moves[i];
        }
    }

    return (Node){.move = bestMove, .score = bestScore};
}

Node alphaBeta(Position* position, char depth, int alpha, int beta) {
    if(hasGameEnded(position)) return (Node){.score = endNodeEvaluation(position)};

    if(depth == 1) return staticSearch(position);

    // Mate in 1
    Node staticNode = staticSearch(position);
    if(staticNode.score == winScore(position->toMove)) return staticNode;

    Move bestMove = 0;

    Move moves[MAX_BRANCHING_FACTOR];
    int moveCount = staticOrderLegalMoves(moves, position, position->toMove);

    Position newPosition;
    int i;
    for(i = 0; i < moveCount; i++) {
        updatePosition(&newPosition, position, moves[i]);

        int score = alphaBeta(&newPosition, depth - 1, alpha, beta).score;

        if(score == winScore(position->toMove)) {
            return (Node){.move = moves[i], .score = score};
        }

        if(position->toMove == WHITE && score > alpha) {
            alpha = score;
            bestMove = moves[i];
        } else if(position->toMove == BLACK && score < beta) {
            beta = score;
            bestMove = moves[i];
        }

        if(alpha > beta) {
            break;
        }
    }

    return (Node){.move = bestMove, .score = position->toMove == WHITE ? alpha : beta};
}

int alphaBetaNodes(Node* sortedNodes, Position* position, char depth) {
    Node nodes[MAX_BRANCHING_FACTOR];
    Move moves[MAX_BRANCHING_FACTOR];
    int moveCount = legalMoves(moves, position, position->toMove);

    Position newPosition;
    int i;
    for(i = 0; i < moveCount; i++) {
        updatePosition(&newPosition, position, moves[i]);

        nodes[i].move = moves[i];
        nodes[i].score = depth > 1 ?
                             alphaBeta(&newPosition, depth - 1, INT32_MIN, INT32_MAX).score :
                             staticEvaluation(&newPosition);
    }

    sortNodes(sortedNodes, nodes, moveCount, position->toMove);

    return moveCount;
}

Node iterativeDeepeningAlphaBeta(Position* position, char depth, int alpha, int beta, BOOL verbose) {
    if(hasGameEnded(position)) return (Node){.score = endNodeEvaluation(position)};

    if(depth == 1) return quiescenceSearch(position);
    //        return staticSearch(position);

    // Mate in 1
    Node staticNode = staticSearch(position);
    if(staticNode.score == winScore(position->toMove)) return staticNode;

    Move bestMove = 0;

    if(verbose) {
        printf("Ordering moves...\n");
        fflush(stdout);
    }

    Node nodes[MAX_BRANCHING_FACTOR];
    int moveCount = alphaBetaNodes(nodes, position, depth - 1);

    Position newPosition;
    int i;
    for(i = 0; i < moveCount; i++) {
        updatePosition(&newPosition, position, nodes[i].move);

        if(verbose) {
            printf("(Move %2d/%d) ", i + 1, moveCount);
            printFullMove(nodes[i].move, &(position->board));
            printf(" = ");
            fflush(stdout);
        }

        int score = iterativeDeepeningAlphaBeta(&newPosition, depth - 1, alpha, beta, FALSE).score;

        if(verbose) {
            printf("%.2f\n", (double)score / (double)100.00);
            fflush(stdout);
        }

        if(score == winScore(position->toMove)) {
            return (Node){.move = nodes[i].move, .score = score};
        }

        if(position->toMove == WHITE && score > alpha) {
            alpha = score;
            bestMove = nodes[i].move;
        } else if(position->toMove == BLACK && score < beta) {
            beta = score;
            bestMove = nodes[i].move;
        }

        if(alpha > beta) {
            break;
        }
    }

    return (Node){.move = bestMove, .score = position->toMove == WHITE ? alpha : beta};
}

Node pIDAB(Position* position, char depth, int* p_alpha, int* p_beta) {
    if(hasGameEnded(position)) return (Node){.score = endNodeEvaluation(position)};

    if(depth == 1) return quiescenceSearch(position);

    // Mate in 1
    Node staticNode = staticSearch(position);
    if(staticNode.score == winScore(position->toMove)) return staticNode;

    Move bestMove = 0;

    Node nodes[MAX_BRANCHING_FACTOR];
    int moveCount = alphaBetaNodes(nodes, position, depth - 1);

    Position newPosition;
    int i;
    int alpha = *p_alpha;
    int beta = *p_beta;
    for(i = 0; i < moveCount; i++) {
        updatePosition(&newPosition, position, nodes[i].move);

        int score = iterativeDeepeningAlphaBeta(&newPosition, depth - 1, alpha, beta, FALSE).score;

        if(score == winScore(position->toMove)) {
            return (Node){.move = nodes[i].move, .score = score};
        }

        if(position->toMove == WHITE && score > alpha) {
            alpha = score;
            bestMove = nodes[i].move;
        } else if(position->toMove == BLACK && score < beta) {
            beta = score;
            bestMove = nodes[i].move;
        }

        if(alpha > beta || alpha > *p_beta || *p_alpha > beta) {
            break;
        }
    }

    return (Node){.move = bestMove, .score = position->toMove == WHITE ? alpha : beta};
}

Node pIDABhashed(Position* position, char depth, int* p_alpha, int* p_beta) {
    if(hasGameEnded(position)) {
        int score = endNodeEvaluation(position);
        writeToHashFile(position, score, 0);
        return (Node){.score = score};
    }

    if(depth <= 1) {
        Node quie = quiescenceSearch(position);
        writeToHashFile(position, quie.score, depth);
        return quie;
    }

    // Mate in 1
    Node staticNode = staticSearch(position);
    if(staticNode.score == winScore(position->toMove)) {
        writeToHashFile(position, staticNode.score, 1);
        return staticNode;
    }

    Move bestMove = 0;

    Node nodes[MAX_BRANCHING_FACTOR];
    int moveCount = alphaBetaNodes(nodes, position, depth - 1);

    Position newPosition;
    int i;
    int alpha = *p_alpha;
    int beta = *p_beta;
    for(i = 0; i < moveCount; i++) {
        updatePosition(&newPosition, position, nodes[i].move);

        int score = iterativeDeepeningAlphaBeta(&newPosition, depth - 1, alpha, beta, FALSE).score;
        writeToHashFile(&newPosition, score, depth - 1);

        if(score == winScore(position->toMove)) {
            return (Node){.move = nodes[i].move, .score = score};
        }

        if(position->toMove == WHITE && score > alpha) {
            alpha = score;
            bestMove = nodes[i].move;
        } else if(position->toMove == BLACK && score < beta) {
            beta = score;
            bestMove = nodes[i].move;
        }

        if(alpha > beta || alpha > *p_beta || *p_alpha > beta) {
            break;
        }
    }

    writeToHashFile(position, position->toMove == WHITE ? alpha : beta, depth);
    return (Node){.move = bestMove, .score = position->toMove == WHITE ? alpha : beta};
}

// Parallel processing currently only implemented for Windows
#ifdef _WIN32

DWORD WINAPI evaluatePositionThreadFunction(LPVOID lpParam) {
    ThreadInfo* tInfo = (ThreadInfo*)lpParam;
    Position* pos = &tInfo->pos;

    Node node = pIDAB(pos, tInfo->depth, tInfo->alpha, tInfo->beta);

    if(pos->toMove == BLACK && node.score > *tInfo->alpha) {
        *tInfo->alpha = node.score;
    } else if(pos->toMove == WHITE && node.score < *tInfo->beta) {
        *tInfo->beta = node.score;
    }

    if(tInfo->verbose) {
        printf("-");
        fflush(stdout);
    }

    return node.score;
}

DWORD WINAPI evaluatePositionThreadFunctionHashed(LPVOID lpParam) {
    ThreadInfo* tInfo = (ThreadInfo*)lpParam;
    Position* pos = &tInfo->pos;

    Node node = pIDABhashed(pos, tInfo->depth, tInfo->alpha, tInfo->beta);

    if(pos->toMove == BLACK && node.score > *tInfo->alpha) {
        *tInfo->alpha = node.score;
    } else if(pos->toMove == WHITE && node.score < *tInfo->beta) {
        *tInfo->beta = node.score;
    }

    if(tInfo->verbose) {
        printf("-");
        fflush(stdout);
    }

    return node.score;
}

Node idabThreaded(Position* position, int depth, BOOL verbose) {
    if(hasGameEnded(position)) return (Node){.score = endNodeEvaluation(position)};

    if(depth <= 1) return quiescenceSearch(position);

    int i;
    Node nodes[MAX_BRANCHING_FACTOR];
    int moveCount = alphaBetaNodes(nodes, position, depth - 1);

    if(moveCount == 1) {
        return nodes[0];
    }

    if(verbose) {
        printf("Analyzing %d possible moves with base depth %d:\n[", moveCount, depth);
        for(i = 0; i < moveCount; i++) printf(" ");
        printf("]\r[");
        fflush(stdout);
    }

    HANDLE threadHandles[MAX_BRANCHING_FACTOR];
    ThreadInfo threadInfo[MAX_BRANCHING_FACTOR];
    int alpha = INT32_MIN;
    int beta = INT32_MAX;

    for(i = 0; i < moveCount; i++) {
        threadInfo[i].depth = depth - 1;
        updatePosition(&threadInfo[i].pos, position, nodes[i].move);
        threadInfo[i].alpha = &alpha;
        threadInfo[i].beta = &beta;
        threadInfo[i].verbose = verbose;

        threadHandles[i] =
            CreateThread(NULL, 0, evaluatePositionThreadFunction, (LPVOID)&threadInfo[i], 0, NULL);

        if(threadHandles[i] == NULL) {
            //            printf("Error launching process on move #%d!\n", i);
            printf("!");
            fflush(stdout);
        }
    }

    WaitForMultipleObjects((DWORD)moveCount, threadHandles, TRUE, INFINITE);
    if(verbose) {
        printf("]\n");
        fflush(stdout);
    }

    Move bestMove = 0;
    int bestMoveScore = position->toMove == WHITE ? INT32_MIN : INT32_MAX;
    long unsigned int retVal;
    int score;
    for(i = 0; i < moveCount; i++) {
        GetExitCodeThread(threadHandles[i], &retVal);
        score = (int)retVal;

        if((position->toMove == WHITE && score > bestMoveScore) ||
           (position->toMove == BLACK && score < bestMoveScore)) {
            bestMove = nodes[i].move;
            bestMoveScore = score;
        }

        if(CloseHandle(threadHandles[i]) == 0) {
            //            printf("Error on closing thread #%d!\n", i);
            printf("x");
            fflush(stdout);
        }
    }

    return (Node){.move = bestMove, .score = bestMoveScore};
}

Node idabThreadedBestFirst(Position* position, int depth, BOOL verbose) {
    if(hasGameEnded(position)) return (Node){.score = endNodeEvaluation(position)};

    if(depth <= 1) return quiescenceSearch(position);

    int i;
    Node nodes[MAX_BRANCHING_FACTOR];
    int moveCount = alphaBetaNodes(nodes, position, depth - 1);

    if(moveCount == 1) {
        return nodes[0];
    }

    Position firstPos;
    updatePosition(&firstPos, position, nodes[0].move);
    Node firstReply = idabThreaded(&firstPos, depth - 1, FALSE);

    if(firstReply.score == winScore(position->toMove)) {
        if(verbose) {
            printf("Playing checkmate move: ");
            printFullMove(nodes[0].move, position->board);
            printf(".\n");
        }
        return (Node){.move = nodes[0].move, .score = firstReply.score};
    }

    if(verbose) {
        printf("Move ");
        printFullMove(nodes[0].move, position->board);
        printf(" had score of %+.2f.\n", firstReply.score / 100.0);
        printf(
            "Analyzing other %d possible moves with minimum depth of %d plies:\n[",
            moveCount - 1,
            depth);
        for(i = 0; i < moveCount - 1; i++) printf(" ");
        printf("]\r[");
        fflush(stdout);
    }

    HANDLE threadHandles[MAX_BRANCHING_FACTOR];
    ThreadInfo threadInfo[MAX_BRANCHING_FACTOR];
    int alpha = INT32_MIN;
    int beta = INT32_MAX;

    if(position->toMove == WHITE) {
        alpha = firstReply.score;
    } else {
        beta = firstReply.score;
    }

    for(i = 0; i < moveCount - 1; i++) {
        threadInfo[i].depth = depth - 1;
        updatePosition(&threadInfo[i].pos, position, nodes[i + 1].move);
        threadInfo[i].alpha = &alpha;
        threadInfo[i].beta = &beta;
        threadInfo[i].verbose = verbose;

        threadHandles[i] =
            CreateThread(NULL, 0, evaluatePositionThreadFunction, (LPVOID)&threadInfo[i], 0, NULL);

        if(threadHandles[i] == NULL) {
            //            printf("Error launching process on move #%d!\n", i);
            printf("!");
            fflush(stdout);
        }
    }

    WaitForMultipleObjects((DWORD)moveCount - 1, threadHandles, TRUE, INFINITE);
    if(verbose) {
        printf("] Done!\n");
        fflush(stdout);
    }

    Move bestMove = nodes[0].move;
    int bestMoveScore = firstReply.score;
    long unsigned int retVal;
    int score;
    for(i = 0; i < moveCount - 1; i++) {
        GetExitCodeThread(threadHandles[i], &retVal);
        score = (int)retVal;

        if((position->toMove == WHITE && score > bestMoveScore) ||
           (position->toMove == BLACK && score < bestMoveScore)) {
            bestMove = nodes[i + 1].move;
            bestMoveScore = score;
        }

        if(CloseHandle(threadHandles[i]) == 0) {
            //            printf("Error on closing thread #%d!\n", i);
            printf("x");
            fflush(stdout);
        }
    }

    return (Node){.move = bestMove, .score = bestMoveScore};
}

Node idabThreadedBestFirstHashed(Position* position, int depth, BOOL verbose) {
    if(hasGameEnded(position)) {
        int score = endNodeEvaluation(position);
        writeToHashFile(position, score, 0);
        return (Node){.score = score};
    }

    if(depth <= 1) {
        Node quie = quiescenceSearch(position);
        writeToHashFile(position, quie.score, depth);
        return quie;
    }

    int i;
    Node nodes[MAX_BRANCHING_FACTOR];
    int moveCount = alphaBetaNodes(nodes, position, depth - 1);

    if(moveCount == 1) {
        return nodes[0];
    }

    Position firstPos;
    updatePosition(&firstPos, position, nodes[0].move);
    Node firstReply = idabThreaded(&firstPos, depth - 1, FALSE);

    if(firstReply.score == winScore(position->toMove)) {
        if(verbose) {
            printf("Playing checkmate move: ");
            printFullMove(nodes[0].move, position->board);
            printf(".\n");
        }
        writeToHashFile(position, firstReply.score, depth);
        return (Node){.move = nodes[0].move, .score = firstReply.score};
    }

    if(verbose) {
        printf("Move ");
        printFullMove(nodes[0].move, position->board);
        printf(" had score of %+.2f.\n", firstReply.score / 100.0);
        printf(
            "Analyzing other %d possible moves with minimum depth of %d plies:\n[",
            moveCount - 1,
            depth);
        for(i = 0; i < moveCount - 1; i++) printf(" ");
        printf("]\r[");
        fflush(stdout);
    }

    HANDLE threadHandles[MAX_BRANCHING_FACTOR];
    ThreadInfo threadInfo[MAX_BRANCHING_FACTOR];
    int alpha = INT32_MIN;
    int beta = INT32_MAX;

    if(position->toMove == WHITE) {
        alpha = firstReply.score;
    } else {
        beta = firstReply.score;
    }

    for(i = 0; i < moveCount - 1; i++) {
        threadInfo[i].depth = depth - 1;
        updatePosition(&threadInfo[i].pos, position, nodes[i + 1].move);
        threadInfo[i].alpha = &alpha;
        threadInfo[i].beta = &beta;
        threadInfo[i].verbose = verbose;

        threadHandles[i] = CreateThread(
            NULL, 0, evaluatePositionThreadFunctionHashed, (LPVOID)&threadInfo[i], 0, NULL);

        if(threadHandles[i] == NULL) {
            //            printf("Error launching process on move #%d!\n", i);
            printf("!");
            fflush(stdout);
        }
    }

    WaitForMultipleObjects((DWORD)moveCount - 1, threadHandles, TRUE, INFINITE);
    if(verbose) {
        printf("] Done!\n");
        fflush(stdout);
    }

    Move bestMove = nodes[0].move;
    int bestMoveScore = firstReply.score;
    long unsigned int retVal;
    int score;
    for(i = 0; i < moveCount - 1; i++) {
        GetExitCodeThread(threadHandles[i], &retVal);
        score = (int)retVal;

        writeToHashFile(&threadInfo[i].pos, score, depth - 1);

        if((position->toMove == WHITE && score > bestMoveScore) ||
           (position->toMove == BLACK && score < bestMoveScore)) {
            bestMove = nodes[i + 1].move;
            bestMoveScore = score;
        }

        if(CloseHandle(threadHandles[i]) == 0) {
            //            printf("Error on closing thread #%d!\n", i);
            printf("x");
            fflush(stdout);
        }
    }

    writeToHashFile(position, bestMoveScore, depth);
    return (Node){.move = bestMove, .score = bestMoveScore};
}

#endif /* _WIN32 */

Move getRandomMove(Position* position) {
    Move moves[MAX_BRANCHING_FACTOR];
    int totalMoves = legalMoves(moves, position, position->toMove);
    int chosenMove = rand() % totalMoves;
    return moves[chosenMove];
}

Move getAIMove(Game* game, int depth) {
    printf("--- AI ---\n");
    fflush(stdout);

    if(fromInitial(game) && countBookOccurrences(game) > 0) {
        printf("There are %d available book continuations.\n", countBookOccurrences(game));
        fflush(stdout);
        Move bookMove = getBookMove(game);
        printf("CHOSEN book move: ");
        printFullMove(bookMove, &(game->position.board));
        printf(".\n");
        fflush(stdout);
        return bookMove;
    }

    time_t startTime, endTime;
    startTime = time(NULL);

    //    Move move = getRandomMove(&game->position);
    //    Move move = simpleEvaluation(&game->position).move;
    //    Move move = minimax(&game->position, AI_DEPTH).move;
    //    Node node = alphaBeta(&game->position, depth, INT32_MIN, INT32_MAX, TRUE);
    //    Node node = iterativeDeepeningAlphaBeta(&game->position, depth, INT32_MIN, INT32_MAX, TRUE);
    //    Node node = idabThreaded(&game->position, depth, TRUE);
    //    Node node = idabThreadedBestFirst(&game->position, depth, TRUE);
    //    Node node = idabThreadedBestFirstHashed(&game->position, depth, TRUE);

#ifdef _WIN32
    Node node = idabThreadedBestFirst(&game->position, depth, TRUE);
#else
    Node node = iterativeDeepeningAlphaBeta(&game->position, depth, INT32_MIN, INT32_MAX, TRUE);
#endif

    endTime = time(NULL);

    printf("CHOSEN move: ");
    printFullMove(node.move, &(game->position.board));
    printf(
        " in %d seconds [%+.2f, %+.2f]\n",
        (int)(endTime - startTime),
        (double)staticEvaluation(&game->position) / (double)100.0,
        (double)node.score / (double)100.0);
    fflush(stdout);

    return node.move;
}

Move parseMove(char* move) {
    int pos1 = str2index(&move[0]);
    int pos2 = str2index(&move[2]);
    return generateMove(pos1, pos2);
}

Move getPlayerMove() {
    char input[100];
    fgets(input, 100, stdin);
    return parseMove(input);
}

Move suggestMove(char fen[], int depth) {
    Game game;
    getFenGame(&game, fen);
    return getAIMove(&game, depth);
}

// ===== PLAY LOOP (TEXT) ====

void playTextWhite(int depth) {
    printf("Playing as WHITE!\n");
    fflush(stdout);

    Game game;
    getInitialGame(&game);

    while(TRUE) {
        printBoard(&(game.position.board));
        if(hasGameEnded(&game.position)) break;

        makeMove(&game, getPlayerMove());

        printBoard(&(game.position.board));
        if(hasGameEnded(&game.position)) break;

        makeMove(&game, getAIMove(&game, depth));
    }
    printOutcome(&game.position);
}

void playTextBlack(int depth) {
    printf("Playing as BLACK!\n");
    fflush(stdout);

    Game game;
    getInitialGame(&game);

    while(TRUE) {
        printBoard(&(game.position.board));
        if(hasGameEnded(&game.position)) break;

        makeMove(&game, getAIMove(&game, depth));

        printBoard(&(game.position.board));
        if(hasGameEnded(&game.position)) break;

        makeMove(&game, getPlayerMove());
    }
    printOutcome(&game.position);
}

void playTextAs(char color, int depth) {
    if(color == WHITE) playTextWhite(depth);
    if(color == BLACK) playTextBlack(depth);
}

void playTextRandomColor(int depth) {
    char colors[] = {WHITE, BLACK};
    char color = colors[rand() % 2];
    playTextAs(color, depth);
}

// ===========================

/*
int main(int argc, char *argv[]) {
    srand(time(NULL));

    int opt;
    int FEN_MODE = 0, MOVES_MODE = 1, mode = FEN_MODE;
    int depth = DEFAULT_AI_DEPTH;

    while ((opt = getopt(argc, argv, "fmd:v")) != -1) {
        switch (opt) {
        case 'f': mode = FEN_MODE; break;
        case 'm': mode = MOVES_MODE; break;
        case 'd': depth = atoi(optarg); break;
        case 'v': printf(ENGINE_VERSION); exit(0);
        default:
            fprintf(stderr, "Usage: %s [-d depth] [-f fen] [-m move_list]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    Game game;
    if (argc > optind) {
        if (mode == FEN_MODE) {
            getFenGame(&game, argv[optind]);
        } else if (mode == MOVES_MODE) {
            getMovelistGame(&game, argv[optind]);
        }
    } else {
        getInitialGame(&game);
    }

    Move move;
    if ( mode == MOVES_MODE && countBookOccurrences(&game) > 0 ) {
        move = getBookMove(&game);
    } else {
        Node node = iterativeDeepeningAlphaBeta(&(game.position), (char) depth, INT32_MIN, INT32_MAX, FALSE);
        move = node.move;
    }

    printf("%c%c%c%c", getFile(getFrom(move)), getRank(getFrom(move)), getFile(getTo(move)), getRank(getTo(move)));

    return EXIT_SUCCESS;
}
// */
