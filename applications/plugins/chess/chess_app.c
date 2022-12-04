#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>

#include <gui/icon_i.h>
#include "fast_chess.h"

static bool flag = true;
static bool should_exit = false;
// static bool ai_should_make_move = false;
static bool thinking = false;
static bool should_update_screen = true;
static uint32_t anim = 0;
static char white_move_str[8] = "", black_move_str[8] = ""; // last moves

static NotificationApp* notification;

const uint8_t _I_Chess_0[] = {
    0x01, 0x00, 0x2a, 0x01, 0x80, 0x7f, 0xc0, 0x2c, 0x0f, 0xf0, 0x7f, 0x83, 0xfc, 0x1f, 0xe0, 0xff,
    0x07, 0xf8, 0x3f, 0xc1, 0xde, 0x0f, 0xf0, 0x7f, 0x83, 0xfc, 0x1f, 0xe0, 0xff, 0x07, 0xf8, 0x3f,
    0xc2, 0x1e, 0x0f, 0xf0, 0x7f, 0x80, 0x07, 0x00, 0x0f, 0xf1, 0x7f, 0xc0, 0x41, 0xf8, 0x1e, 0x18,
    0x0f, 0xf2, 0xfe, 0x1f, 0xb8, 0x0e, 0x0a, 0x07, 0x80, 0x81, 0x83, 0xea, 0x05, 0x62, 0x01, 0x8c,
    0x30, 0x7d, 0x50, 0x48, 0x80, 0x0c, 0x66, 0x00, 0xfa, 0x84, 0x42, 0x00, 0x63, 0x40, 0x07, 0xd4,
    0x42, 0x08, 0x54, 0x24, 0xf5, 0xc0, 0x8f, 0xd9, 0x70, 0x3f, 0xa2, 0x7a, 0xb0, 0x69, 0xee, 0x57,
    0xa0, 0x3e, 0xa8, 0x0b, 0xfc, 0xc0, 0x4f, 0xc1, 0xe5, 0x3c, 0x07, 0xcd, 0x03, 0x81, 0x41, 0x06,
    0x0f, 0x0c, 0x1f, 0x32, 0x08, 0x04, 0x98, 0x46, 0x31, 0xf3, 0x10, 0x83, 0xdd, 0x58, 0x33, 0x88,
    0x07, 0x02, 0xfe, 0x82, 0x10, 0x7c, 0x88, 0x47, 0x81, 0x73, 0x07, 0xcf, 0x42, 0x07, 0xc0, 0x80,
    0x78, 0x3e, 0x2b, 0x21, 0x07, 0xbf, 0xc2, 0x1f, 0x00, 0x81, 0x83, 0xef, 0xc1, 0x1f, 0x7e, 0x0f,
    0x83, 0xff, 0x04, 0x47, 0xc7, 0x82, 0x7e, 0x42, 0x10, 0x7d, 0x96, 0xc4, 0x06, 0x71, 0x60, 0x7c,
    0x3e, 0x48, 0x1e, 0x52, 0xa5, 0xfc, 0xff, 0xd1, 0x62, 0x8f, 0x1a, 0xa8, 0x3e, 0x7f, 0xd0, 0x31,
    0x19, 0x07, 0xeb, 0xf9, 0x07, 0x80, 0x58, 0x2c, 0x01, 0xfa, 0xfc, 0x20, 0x06, 0x21, 0x80, 0x0f,
    0xd7, 0xc1, 0x00, 0x20, 0x01, 0xaa, 0xa3, 0xe1, 0x00, 0x60, 0x01, 0x95, 0x03, 0xe7, 0x80, 0x24,
    0x38, 0xa0, 0x3e, 0x7f, 0x1c, 0x73, 0x00, 0x9d, 0x8c, 0x02, 0xdc, 0x0d, 0x7c, 0x04, 0xa0, 0x2e,
    0xe1, 0x07, 0xc5, 0xc2, 0xeb, 0x00, 0x9f, 0x40, 0x12, 0x42, 0x0f, 0x8d, 0xc4, 0x69, 0x22, 0x3c,
    0x11, 0x7d, 0x5e, 0x20, 0xa0, 0x41, 0x30, 0x98, 0x3d, 0xf1, 0x1f, 0xff, 0xfa, 0x00, 0xcb, 0xd1,
    0x10, 0x2e, 0x18, 0x3e, 0xa4, 0x00, 0xfe, 0xa0, 0x03, 0xfb, 0x00, 0x6b, 0x20, 0x7d, 0xc0, 0x21,
    0xc0, 0xfe, 0xf8, 0x3f, 0xc4, 0x1f, 0x90, 0x0f, 0xe0, 0x40, 0xc1, 0xf6, 0x05, 0x30,
};
const uint8_t* const _I_Chess[] = {_I_Chess_0};

const uint8_t _I_Chess_Selection1_0[] = {
    0x00,
    0x55,
    0x80,
    0x01,
    0x80,
    0x01,
    0x80,
    0x01,
    0xAA,
};
const uint8_t* const _I_Chess_Selection1[] = {_I_Chess_Selection1_0};

const uint8_t _I_Chess_Selection2_0[] = {
    0x00,
    0xAA,
    0x01,
    0x80,
    0x01,
    0x80,
    0x01,
    0x80,
    0x55,
};
const uint8_t* const _I_Chess_Selection2[] = {_I_Chess_Selection2_0};

const uint8_t _I_Chess_bb_0[] = {
    0x00,
    0x0C,
    0x1A,
    0x3D,
    0x1E,
    0x0C,
    0x3F,
};
const uint8_t* const _I_Chess_bb[] = {_I_Chess_bb_0};

const uint8_t _I_Chess_bw_0[] = {
    0x00,
    0x0C,
    0x16,
    0x23,
    0x16,
    0x0C,
    0x3F,
};
const uint8_t* const _I_Chess_bw[] = {_I_Chess_bw_0};

const uint8_t _I_Chess_kb_0[] = {
    0x00,
    0x0C,
    0x2D,
    0x21,
    0x12,
    0x0C,
    0x3F,
};
const uint8_t* const _I_Chess_kb[] = {_I_Chess_kb_0};

const uint8_t _I_Chess_kw_0[] = {
    0x00,
    0x0C,
    0x21,
    0x21,
    0x12,
    0x0C,
    0x3F,
};
const uint8_t* const _I_Chess_kw[] = {_I_Chess_kw_0};

const uint8_t _I_Chess_nb_0[] = {
    0x00,
    0x06,
    0x0F,
    0x1F,
    0x2E,
    0x0E,
    0x3F,
};
const uint8_t* const _I_Chess_nb[] = {_I_Chess_nb_0};

const uint8_t _I_Chess_nw_0[] = {
    0x00,
    0x06,
    0x09,
    0x11,
    0x2A,
    0x0A,
    0x3F,
};
const uint8_t* const _I_Chess_nw[] = {_I_Chess_nw_0};

const uint8_t _I_Chess_old_0[] = {
    0x01, 0x00, 0x35, 0x01, 0x80, 0x7f, 0xc0, 0x2c, 0x0f, 0xf0, 0x7f, 0x83, 0xfc, 0x1f, 0xe0, 0xff,
    0x07, 0xf8, 0x3f, 0xc0, 0x03, 0x80, 0x0f, 0x70, 0x3f, 0xe0, 0x10, 0x11, 0x77, 0x40, 0x7f, 0x97,
    0xf0, 0xfd, 0xc0, 0x70, 0x50, 0x3c, 0x04, 0x0c, 0x1f, 0x50, 0x2b, 0x10, 0x0c, 0x61, 0x80, 0xfa,
    0x82, 0x44, 0x00, 0x63, 0x30, 0x07, 0xd4, 0x22, 0x10, 0x03, 0x1a, 0x00, 0x3e, 0xa2, 0x10, 0x42,
    0xa1, 0x90, 0x2d, 0x01, 0x97, 0x03, 0xfa, 0x03, 0xe7, 0x01, 0x83, 0x5f, 0xf8, 0x3f, 0xa8, 0x00,
    0xfc, 0xc0, 0x4f, 0xc1, 0xe5, 0x3c, 0x07, 0xcd, 0x03, 0x81, 0x41, 0x06, 0x0f, 0x0c, 0x1f, 0x32,
    0x08, 0x04, 0x98, 0x46, 0x31, 0xf8, 0x08, 0xfa, 0x15, 0x83, 0x38, 0x80, 0x70, 0x2f, 0xf0, 0x20,
    0x7d, 0x08, 0x47, 0x81, 0x73, 0x07, 0xcf, 0x42, 0x07, 0xc0, 0x80, 0x78, 0x3e, 0x30, 0x40, 0x7c,
    0x7c, 0x21, 0xf0, 0x08, 0x18, 0x3e, 0xfc, 0x11, 0xf7, 0xe0, 0xf8, 0x3f, 0xe0, 0xfa, 0x9f, 0x90,
    0x84, 0x1f, 0x65, 0xb1, 0x01, 0x9c, 0x59, 0x9d, 0x21, 0x34, 0x95, 0x2f, 0xe7, 0xfe, 0xee, 0x14,
    0x78, 0xd5, 0x41, 0xf3, 0xfe, 0x81, 0x88, 0xc8, 0x3f, 0x5f, 0xc8, 0x3c, 0x02, 0xc1, 0x60, 0x0f,
    0xd7, 0xe1, 0x00, 0x31, 0x0c, 0x00, 0x7e, 0xbe, 0x08, 0x01, 0x00, 0x08, 0x7e, 0x90, 0x04, 0x00,
    0x10, 0xfd, 0x70, 0x00, 0x65, 0x00, 0x8a, 0x0f, 0xeb, 0x8e, 0x60, 0x13, 0xa9, 0x07, 0xa3, 0x5f,
    0x01, 0x28, 0x0c, 0x08, 0x1f, 0x37, 0x0b, 0xac, 0x02, 0x7d, 0x00, 0x49, 0x08, 0x3e, 0x37, 0x11,
    0xa4, 0x88, 0xf0, 0x45, 0xf5, 0x78, 0x82, 0x81, 0x44, 0xc2, 0x40, 0xf8, 0xc4, 0x7f, 0xff, 0xe8,
    0x03, 0x07, 0xc4, 0x40, 0x18, 0x40, 0xfb, 0x90, 0x03, 0xfa, 0x80, 0x0f, 0x50, 0x84, 0x60, 0x0d,
    0x62, 0x20, 0xd8, 0x70, 0x3f, 0xbe, 0x0f, 0xf1, 0x07, 0xe4, 0x03, 0xf8, 0x10, 0x40, 0x7d, 0x01,
    0x0c, 0x1f, 0x77, 0xf2, 0x91, 0x83, 0xeb, 0x7e, 0x1f, 0xdf, 0xa5, 0x7c, 0x1d, 0x90, 0x0d, 0x54,
    0xa8, 0x1f, 0xb5, 0x68, 0xa8, 0x7f, 0xa1, 0x40, 0xfd, 0xaa, 0xc1, 0x41, 0xfb, 0xa1, 0x81, 0x03,
    0xf5, 0xa0, 0x80, 0xff, 0x07, 0xce, 0x01, 0x9c, 0x80,
};
const uint8_t* const _I_Chess_old[] = {_I_Chess_old_0};

const uint8_t _I_Chess_pb_0[] = {
    0x00,
    0x00,
    0x0C,
    0x1E,
    0x1E,
    0x0C,
    0x1E,
};
const uint8_t* const _I_Chess_pb[] = {_I_Chess_pb_0};

const uint8_t _I_Chess_pw_0[] = {
    0x00,
    0x00,
    0x0C,
    0x12,
    0x12,
    0x0C,
    0x1E,
};
const uint8_t* const _I_Chess_pw[] = {_I_Chess_pw_0};

const uint8_t _I_Chess_qb_0[] = {
    0x00,
    0x2D,
    0x2D,
    0x2D,
    0x1E,
    0x1E,
    0x3F,
};
const uint8_t* const _I_Chess_qb[] = {_I_Chess_qb_0};

const uint8_t _I_Chess_qw_0[] = {
    0x00,
    0x2D,
    0x2D,
    0x2D,
    0x1E,
    0x1E,
    0x3F,
};
const uint8_t* const _I_Chess_qw[] = {_I_Chess_qw_0};

const uint8_t _I_Chess_rb_0[] = {
    0x00,
    0x2D,
    0x2D,
    0x1E,
    0x1E,
    0x1E,
    0x3F,
};
const uint8_t* const _I_Chess_rb[] = {_I_Chess_rb_0};

const uint8_t _I_Chess_rw_0[] = {
    0x00,
    0x2D,
    0x2D,
    0x12,
    0x12,
    0x12,
    0x3F,
};
const uint8_t* const _I_Chess_rw[] = {_I_Chess_rw_0};

const Icon I_Chess_Selection2 =
    {.width = 8, .height = 8, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_Selection2};
const Icon I_Chess_old =
    {.width = 128, .height = 64, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_old};
const Icon I_Chess_Selection1 =
    {.width = 8, .height = 8, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_Selection1};
const Icon I_Chess =
    {.width = 128, .height = 64, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess};
const Icon I_Chess_kb =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_kb};
const Icon I_Chess_rw =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_rw};
const Icon I_Chess_rb =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_rb};
const Icon I_Chess_kw =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_kw};
const Icon I_Chess_qb =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_qb};
const Icon I_Chess_qw =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_qw};
const Icon I_Chess_pw =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_pw};
const Icon I_Chess_pb =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_pb};
const Icon I_Chess_nb =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_nb};
const Icon I_Chess_bw =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_bw};
const Icon I_Chess_bb =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_bb};
const Icon I_Chess_nw =
    {.width = 6, .height = 6, .frame_count = 1, .frame_rate = 0, .frames = _I_Chess_nw};

typedef struct {
    uint8_t col, row;
} _Position;

typedef struct {
    enum {
        None = 0,
        Pawn,
        King,
        Queen,
        Bishop,
        Knight,
        Rook,
    } type;
    enum { White, Black } side;
} Piece;

static const _Position PosNone = {.col = 255, .row = 255};
// static Piece board[8][8]; // col, row
static _Position sel, move_from = PosNone, move_to = PosNone;

Game* game;

// uint8_t sel_col = 0, sel_row = 0;

// static enum {
//     SelectingFrom,
//     SelectingTo
// } state = SelectingFrom;

// static void reset_board() {
//     memset(board, 0, sizeof(board));

//     board[0][0].type = Rook;
//     board[1][0].type = Knight;
//     board[2][0].type = Bishop;
//     board[3][0].type = Queen;
//     board[4][0].type = King;
//     board[5][0].type = Bishop;
//     board[6][0].type = Knight;
//     board[7][0].type = Rook;

//     board[0][1].type = Pawn;
//     board[1][1].type = Pawn;
//     board[2][1].type = Pawn;
//     board[3][1].type = Pawn;
//     board[4][1].type = Pawn;
//     board[5][1].type = Pawn;
//     board[6][1].type = Pawn;
//     board[7][1].type = Pawn;

//     board[0][7].type = Rook;    board[0][7].side = Black;
//     board[1][7].type = Knight;  board[1][7].side = Black;
//     board[2][7].type = Bishop;  board[2][7].side = Black;
//     board[3][7].type = Queen;   board[3][7].side = Black;
//     board[4][7].type = King;    board[4][7].side = Black;
//     board[5][7].type = Bishop;  board[5][7].side = Black;
//     board[6][7].type = Knight;  board[6][7].side = Black;
//     board[7][7].type = Rook;    board[7][7].side = Black;

//     board[0][6].type = Pawn;    board[0][6].side = Black;
//     board[1][6].type = Pawn;    board[1][6].side = Black;
//     board[2][6].type = Pawn;    board[2][6].side = Black;
//     board[3][6].type = Pawn;    board[3][6].side = Black;
//     board[4][6].type = Pawn;    board[4][6].side = Black;
//     board[5][6].type = Pawn;    board[5][6].side = Black;
//     board[6][6].type = Pawn;    board[6][6].side = Black;
//     board[7][6].type = Pawn;    board[7][6].side = Black;
// }

// static const Icon* get_icon(const Piece* piece) {
//     if (piece->side == White) {
//         switch (piece->type) {
//             case Pawn: return &I_Chess_pw;
//             case King: return &I_Chess_kw;
//             case Queen: return &I_Chess_qw;
//             case Bishop: return &I_Chess_bw;
//             case Knight: return &I_Chess_nw;
//             case Rook: return &I_Chess_rw;
//             default: return NULL;
//         }
//     } else {
//         switch (piece->type) {
//             case Pawn: return &I_Chess_pb;
//             case King: return &I_Chess_kb;
//             case Queen: return &I_Chess_qb;
//             case Bishop: return &I_Chess_bb;
//             case Knight: return &I_Chess_nb;
//             case Rook: return &I_Chess_rb;
//             default: return NULL;
//         }
//     }
// }

static void notify_click() {
    // static const NotificationSequence sequence = {
    //     &message_click,
    //     &message_delay_1,
    //     &message_sound_off,
    //     NULL,
    // };

    // notification_message_block(notification, &sequence);
    notification_message(notification, &sequence_single_vibro);
}
static const Icon* _get_icon(uint8_t file, uint8_t rank) {
    char piece = getPieceChar((FILES_BB[file] & RANKS_BB[7 - rank]), &(game->position.board));
    switch(piece) {
    case 'P':
        return &I_Chess_pw;
    case 'K':
        return &I_Chess_kw;
    case 'Q':
        return &I_Chess_qw;
    case 'B':
        return &I_Chess_bw;
    case 'N':
        return &I_Chess_nw;
    case 'R':
        return &I_Chess_rw;
    case 'p':
        return &I_Chess_pb;
    case 'k':
        return &I_Chess_kb;
    case 'q':
        return &I_Chess_qb;
    case 'b':
        return &I_Chess_bb;
    case 'n':
        return &I_Chess_nb;
    case 'r':
        return &I_Chess_rb;
    default:
        return NULL;
    }
}

static int get_position(uint8_t file, uint8_t rank) {
    return 8 * rank + file;
}

static int get_rank(int position) {
    return (int)(position / 8);
}

static int get_file(int position) {
    return position % 8;
}

static void make_move(uint8_t file1, uint8_t rank1, uint8_t file2, uint8_t rank2) {
    int from = get_position(file1, rank1);
    int to = get_position(file2, rank2);
    Move move = generateMove(from, to);
    if(!isLegalMove(&game->position, move)) {
        return;
    }
    makeMove(game, move);
    move2str(white_move_str, game, game->moveListLen - 1);
    notify_click();
    black_move_str[0] = 0;
    anim = furi_get_tick();
    thinking = true;
}

static int32_t make_ai_move(void* context) {
    UNUSED(context);
    // thinking = true;
    int depth = 1;
    Move move;
    Node node =
        iterativeDeepeningAlphaBeta(&(game->position), (char)depth, INT32_MIN, INT32_MAX, FALSE);
    move = node.move;
    makeMove(game, move);
    move2str(black_move_str, game, game->moveListLen - 1);
    notify_click();
    thinking = false;
    anim = furi_get_tick();
    return 0;
}

static FuriThread* worker_thread = NULL;

static int32_t ai_thread(void* context) {
    while(true) {
        if(should_exit) break;
        if(thinking) make_ai_move(context);
        furi_delay_ms(100);
    }
    return 0;
}

static void run_ai_thread() {
    if(worker_thread == NULL) {
        worker_thread = furi_thread_alloc();
    }

    furi_thread_set_name(worker_thread, "ChessEngine");
    furi_thread_set_stack_size(worker_thread, 7000);
    // furi_thread_set_context(thread, bad_usb);
    furi_thread_set_callback(worker_thread, ai_thread);
    furi_thread_start(worker_thread);

    // furi_thread_join(worker_thread);
    // furi_thread_free(worker_thread);
}

static void chess_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    should_update_screen = false;
    canvas_clear(canvas);

    // canvas_set_color(canvas, flag ? ColorBlack : ColorWhite);

    canvas_draw_icon(canvas, 0, 0, &I_Chess);

    if(!thinking) {
        canvas_set_color(canvas, (sel.col + sel.row) % 2 != 0 ? ColorBlack : ColorWhite);
        canvas_draw_icon(
            canvas,
            sel.col * 8,
            (7 - sel.row) * 8,
            flag ? &I_Chess_Selection1 : &I_Chess_Selection2);

        if(move_from.col != 255) {
            canvas_set_color(
                canvas, (move_from.col + move_from.row) % 2 != 0 ? ColorBlack : ColorWhite);
            canvas_draw_icon(
                canvas,
                move_from.col * 8,
                (7 - move_from.row) * 8,
                flag ? &I_Chess_Selection1 : &I_Chess_Selection2);
        }
    }

    // print moves
    if(game->moveListLen > 0) {
        canvas_set_color(canvas, ColorBlack);
        canvas_set_font(canvas, FontSecondary);

        // int num = game->moveListLen;

        // char white_str[8], black_str[8] = "...";

        // if (num == 0) {
        // } else if (num % 2 == 0) {
        //     // white move
        //     move2str(white_str, game, game->moveListLen - 2);
        //     move2str(black_str, game, game->moveListLen - 1);
        // } else {
        //     move2str(white_str, game, game->moveListLen - 1);
        // }

        char str[28];
        snprintf(
            str, 28, "%d. %s %s", (game->moveListLen + 1) / 2, white_move_str, black_move_str);
        canvas_draw_str(canvas, 75, 12, str);
    }

    Move last_move = getLastMove(game);

    for(uint8_t row = 0; row < 8; row++) {
        for(uint8_t col = 0; col < 8; col++) {
            bool white_field = (row + col) % 2 != 0;

            // if (!white_field) {
            //     canvas_draw_box(canvas, col * 8, row * 8, 8, 8);
            // }
            const Icon* icon = _get_icon(col, row);
            if(icon != NULL) {
                int x = col * 8;
                int y = row * 8;

                int dt = furi_get_tick() - anim;
                if(anim && dt >= 300) {
                    anim = 0;
                }

                if(anim && last_move && get_file(getTo(last_move)) == col &&
                   get_rank(getTo(last_move)) == (7 - row)) {
                    // moving piece
                    uint8_t from_x = get_file(getFrom(last_move)) * 8;
                    uint8_t from_y = (7 - get_rank(getFrom(last_move))) * 8;
                    x = from_x + (x - from_x) * dt / 300;
                    y = from_y + (y - from_y) * dt / 300;
                }

                canvas_set_color(canvas, white_field ? ColorWhite : ColorBlack);
                canvas_draw_icon(canvas, x + 1, y + 1, icon);
            }

            // if (board[col][7 - row].type != None) {
            //     canvas_set_color(canvas, white_field ? ColorWhite : ColorBlack);
            //     canvas_draw_icon(canvas, col * 8 + 1, row * 8 + 1, get_icon(&board[col][7 - row]));
            // }
        }
    }

    // for (uint8_t i = 0; i < 4; i++) {
    //     canvas_draw_dot(canvas, sel_col * 8, sel_row * 8);
    //     canvas_draw_dot(canvas, sel_col * 8 + 2, sel_row * 8);
    //     canvas_draw_dot(canvas, sel_col * 8, sel_row * 8);
    //     canvas_draw_dot(canvas, sel_col * 8, sel_row * 8);
    // }

    // canvas_draw_disc(canvas, GUI_DISPLAY_WIDTH / 2 - 40, GUI_DISPLAY_HEIGHT / 2, 15);
    // canvas_set_color(canvas, flag ? ColorBlack : ColorWhite);
    // canvas_draw_disc(canvas, GUI_DISPLAY_WIDTH / 2, GUI_DISPLAY_HEIGHT / 2, 15);
}

static void chess_input_callback(InputEvent* event, void* ctx) {
    UNUSED(ctx);
    if(event->type == InputTypeShort) {
        if(event->key == InputKeyLeft) {
            sel.col = (sel.col == 0) ? 0 : sel.col - 1;
        } else if(event->key == InputKeyRight) {
            sel.col++;
        } else if(event->key == InputKeyDown) {
            sel.row = (sel.row == 0) ? 0 : sel.row - 1;
        } else if(event->key == InputKeyUp) {
            sel.row++;
        } else if(event->key == InputKeyOk) {
            if(move_from.col == 255) {
                move_from = sel;
            } else if(move_to.col == 255) {
                move_to = sel;
                make_move(move_from.col, move_from.row, move_to.col, move_to.row);
                // thinking = true;
                // ai_should_make_move = true;
                // make_ai_move_threaded();
                // Piece piece = board[move_from.col][move_from.row];
                // board[move_from.col][move_from.row].type = None;
                // board[move_to.col][move_to.row] = piece;
                move_from = PosNone;
                move_to = PosNone;
            }
        } else if(event->key == InputKeyBack) {
            should_exit = true;
        }
        sel.col = CLAMP(sel.col, 7, 0);
        sel.row = CLAMP(sel.row, 7, 0);
    }
}

static void setup_engine() {
    // int depth = 1; // DEFAULT_AI_DEPTH;

    getInitialGame(game);

    // Move move;
    // Node node = iterativeDeepeningAlphaBeta(&(game.position), (char) depth, INT32_MIN, INT32_MAX, FALSE);
    // move = node.move;

    // node = iterativeDeepeningAlphaBeta(&(game.position), (char) 2, INT32_MIN, INT32_MAX, FALSE);

    // node = iterativeDeepeningAlphaBeta(&(game.position), (char) 3, INT32_MIN, INT32_MAX, FALSE);

    // printf("%d\n", move);
}

//  void test_engine() {
//     FuriThread* thread; //

//     thread = furi_thread_alloc();
//     furi_thread_set_name(thread, "ChessEngine");
//     furi_thread_set_stack_size(thread, 20000);
//     // furi_thread_set_context(thread, bad_usb);
//     furi_thread_set_callback(thread, setup_engine);

//     furi_thread_start(thread);

//     furi_thread_join(thread);

//     furi_thread_free(thread);
// }

int32_t chess_app(void* p) {
    UNUSED(p);
    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, chess_draw_callback, NULL);
    view_port_input_callback_set(view_port, chess_input_callback, NULL);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    notification = furi_record_open(RECORD_NOTIFICATION);

    should_exit = false;

    game = malloc(sizeof(Game));

    setup_engine();
    run_ai_thread();

    // test_engine();

    while(!should_exit) {
        furi_delay_ms(100);
        if(!thinking) {
            flag = !flag;
            should_update_screen = true;
        }
        if(anim) {
            should_update_screen = true;
        }
        if(should_update_screen) {
            view_port_update(view_port);
        }
        // flag = true;
        // delay(40);
        // flag = false;
        // view_port_update(view_port);
        // delay(80);
    }

    furi_thread_join(worker_thread);
    furi_thread_free(worker_thread);
    worker_thread = NULL;

    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);

    furi_record_close(RECORD_GUI);

    free(game);

    return 0;
}
