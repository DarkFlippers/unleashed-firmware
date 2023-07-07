
#include <gui/gui.h>
#include <stdlib.h>
#include <dolphin/dolphin.h>
#include <dialogs/dialogs.h>
#include <gui/canvas_i.h>

#include <math.h>
#include "util.h"
#include "defines.h"
#include "common/card.h"
#include "common/dml.h"
#include "common/queue.h"
#include "util.h"
#include "ui.h"

#include "blackjack_icons.h"

#define DEALER_MAX 17

void start_round(GameState* game_state);

void init(GameState* game_state);

static void draw_ui(Canvas* const canvas, const GameState* game_state) {
    draw_money(canvas, game_state->player_score);

    draw_score(canvas, true, hand_count(game_state->player_cards, game_state->player_card_count));

    if(!game_state->queue_state.running && game_state->state == GameStatePlay) {
        render_menu(game_state->menu, canvas, 2, 47);
    }
}

static void render_callback(Canvas* const canvas, void* ctx) {
    furi_assert(ctx);
    const GameState* game_state = ctx;
    furi_mutex_acquire(game_state->mutex, FuriWaitForever);

    canvas_set_color(canvas, ColorBlack);
    canvas_draw_frame(canvas, 0, 0, 128, 64);

    if(game_state->state == GameStateStart) {
        canvas_draw_icon(canvas, 0, 0, &I_blackjack);
    }
    if(game_state->state == GameStateGameOver) {
        canvas_draw_icon(canvas, 0, 0, &I_endscreen);
    }

    if(game_state->state == GameStatePlay || game_state->state == GameStateDealer) {
        if(game_state->state == GameStatePlay)
            draw_player_scene(canvas, game_state);
        else
            draw_dealer_scene(canvas, game_state);
        render_queue(&(game_state->queue_state), game_state, canvas);
        draw_ui(canvas, game_state);
    } else if(game_state->state == GameStateSettings) {
        settings_page(canvas, game_state);
    }

    furi_mutex_release(game_state->mutex);
}

//region card draw
Card draw_card(GameState* game_state) {
    Card c = game_state->deck.cards[game_state->deck.index];
    game_state->deck.index++;
    return c;
}

void drawPlayerCard(void* ctx) {
    GameState* game_state = ctx;
    Card c = draw_card(game_state);
    game_state->player_cards[game_state->player_card_count] = c;
    game_state->player_card_count++;
    if(game_state->player_score < game_state->settings.round_price || game_state->doubled) {
        set_menu_state(game_state->menu, 0, false);
    }
}

void drawDealerCard(void* ctx) {
    GameState* game_state = ctx;
    Card c = draw_card(game_state);
    game_state->dealer_cards[game_state->dealer_card_count] = c;
    game_state->dealer_card_count++;
}
//endregion

//region queue callbacks
void to_lose_state(const void* ctx, Canvas* const canvas) {
    const GameState* game_state = ctx;
    if(game_state->settings.message_duration == 0) return;
    popup_frame(canvas);
    elements_multiline_text_aligned(canvas, 64, 22, AlignCenter, AlignCenter, "You lost");
}

void to_bust_state(const void* ctx, Canvas* const canvas) {
    const GameState* game_state = ctx;
    if(game_state->settings.message_duration == 0) return;
    popup_frame(canvas);
    elements_multiline_text_aligned(canvas, 64, 22, AlignCenter, AlignCenter, "Busted!");
}

void to_draw_state(const void* ctx, Canvas* const canvas) {
    const GameState* game_state = ctx;
    if(game_state->settings.message_duration == 0) return;
    popup_frame(canvas);
    elements_multiline_text_aligned(canvas, 64, 22, AlignCenter, AlignCenter, "Draw");
}

void to_dealer_turn(const void* ctx, Canvas* const canvas) {
    const GameState* game_state = ctx;
    if(game_state->settings.message_duration == 0) return;
    popup_frame(canvas);
    elements_multiline_text_aligned(canvas, 64, 22, AlignCenter, AlignCenter, "Dealers turn");
}

void to_win_state(const void* ctx, Canvas* const canvas) {
    const GameState* game_state = ctx;
    if(game_state->settings.message_duration == 0) return;
    popup_frame(canvas);
    elements_multiline_text_aligned(canvas, 64, 22, AlignCenter, AlignCenter, "You win");
}

void to_start(const void* ctx, Canvas* const canvas) {
    const GameState* game_state = ctx;
    if(game_state->settings.message_duration == 0) return;
    popup_frame(canvas);
    elements_multiline_text_aligned(canvas, 64, 22, AlignCenter, AlignCenter, "Round started");
}

void before_start(void* ctx) {
    GameState* game_state = ctx;
    game_state->dealer_card_count = 0;
    game_state->player_card_count = 0;
}

void start(void* ctx) {
    GameState* game_state = ctx;
    start_round(game_state);
}

void draw(void* ctx) {
    GameState* game_state = ctx;
    game_state->player_score += game_state->bet;
    game_state->bet = 0;
    enqueue(
        &(game_state->queue_state),
        game_state,
        start,
        before_start,
        to_start,
        game_state->settings.message_duration);
}

void game_over(void* ctx) {
    GameState* game_state = ctx;
    game_state->state = GameStateGameOver;
}

void lose(void* ctx) {
    GameState* game_state = ctx;
    game_state->state = GameStatePlay;
    game_state->bet = 0;
    if(game_state->player_score >= game_state->settings.round_price) {
        enqueue(
            &(game_state->queue_state),
            game_state,
            start,
            before_start,
            to_start,
            game_state->settings.message_duration);
    } else {
        enqueue(&(game_state->queue_state), game_state, game_over, NULL, NULL, 0);
    }
}

void win(void* ctx) {
    GameState* game_state = ctx;
    game_state->state = GameStatePlay;
    game_state->player_score += game_state->bet * 2;
    game_state->bet = 0;
    enqueue(
        &(game_state->queue_state),
        game_state,
        start,
        before_start,
        to_start,
        game_state->settings.message_duration);
}

void dealerTurn(void* ctx) {
    GameState* game_state = ctx;
    game_state->state = GameStateDealer;
}

float animationTime(const GameState* game_state) {
    return (float)(furi_get_tick() - game_state->queue_state.start) /
           (float)(game_state->settings.animation_duration);
}

void dealer_card_animation(const void* ctx, Canvas* const canvas) {
    const GameState* game_state = ctx;
    float t = animationTime(game_state);

    Card animatingCard = game_state->deck.cards[game_state->deck.index];
    if(game_state->dealer_card_count > 1) {
        Vector end = card_pos_at_index(game_state->dealer_card_count);
        draw_card_animation(animatingCard, (Vector){0, 64}, (Vector){0, 32}, end, t, true, canvas);
    } else {
        draw_card_animation(
            animatingCard,
            (Vector){32, -CARD_HEIGHT},
            (Vector){64, 32},
            (Vector){2, 2},
            t,
            false,
            canvas);
    }
}

void dealer_back_card_animation(const void* ctx, Canvas* const canvas) {
    const GameState* game_state = ctx;
    float t = animationTime(game_state);

    Vector currentPos =
        quadratic_2d((Vector){32, -CARD_HEIGHT}, (Vector){64, 32}, (Vector){13, 5}, t);
    draw_card_back_at(currentPos.x, currentPos.y, canvas);
}

void player_card_animation(const void* ctx, Canvas* const canvas) {
    const GameState* game_state = ctx;
    float t = animationTime(game_state);

    Card animatingCard = game_state->deck.cards[game_state->deck.index];
    Vector end = card_pos_at_index(game_state->player_card_count);

    draw_card_animation(
        animatingCard, (Vector){32, -CARD_HEIGHT}, (Vector){0, 32}, end, t, true, canvas);
}
//endregion

void player_tick(GameState* game_state) {
    uint8_t score = hand_count(game_state->player_cards, game_state->player_card_count);
    if((game_state->doubled && score <= 21) || score == 21) {
        enqueue(
            &(game_state->queue_state),
            game_state,
            dealerTurn,
            NULL,
            to_dealer_turn,
            game_state->settings.message_duration);
    } else if(score > 21) {
        enqueue(
            &(game_state->queue_state),
            game_state,
            lose,
            NULL,
            to_bust_state,
            game_state->settings.message_duration);
    } else {
        if(game_state->selectDirection == DirectionUp ||
           game_state->selectDirection == DirectionDown) {
            move_menu(game_state->menu, game_state->selectDirection == DirectionUp ? -1 : 1);
        }

        if(game_state->selectDirection == Select) {
            activate_menu(game_state->menu, game_state);
        }
    }
}

void dealer_tick(GameState* game_state) {
    uint8_t dealer_score = hand_count(game_state->dealer_cards, game_state->dealer_card_count);
    uint8_t player_score = hand_count(game_state->player_cards, game_state->player_card_count);

    if(dealer_score >= DEALER_MAX) {
        if(dealer_score > 21 || dealer_score < player_score) {
            dolphin_deed(DolphinDeedPluginGameWin);
            enqueue(
                &(game_state->queue_state),
                game_state,
                win,
                NULL,
                to_win_state,
                game_state->settings.message_duration);
        } else if(dealer_score > player_score) {
            enqueue(
                &(game_state->queue_state),
                game_state,
                lose,
                NULL,
                to_lose_state,
                game_state->settings.message_duration);
        } else if(dealer_score == player_score) {
            enqueue(
                &(game_state->queue_state),
                game_state,
                draw,
                NULL,
                to_draw_state,
                game_state->settings.message_duration);
        }
    } else {
        enqueue(
            &(game_state->queue_state),
            game_state,
            drawDealerCard,
            NULL,
            dealer_card_animation,
            game_state->settings.animation_duration);
    }
}

void settings_tick(GameState* game_state) {
    if(game_state->selectDirection == DirectionDown && game_state->selectedMenu < 4) {
        game_state->selectedMenu++;
    }
    if(game_state->selectDirection == DirectionUp && game_state->selectedMenu > 0) {
        game_state->selectedMenu--;
    }

    if(game_state->selectDirection == DirectionLeft ||
       game_state->selectDirection == DirectionRight) {
        int nextScore = 0;
        switch(game_state->selectedMenu) {
        case 0:
            nextScore = game_state->settings.starting_money;
            if(game_state->selectDirection == DirectionLeft)
                nextScore -= 10;
            else
                nextScore += 10;
            if(nextScore >= (int)game_state->settings.round_price && nextScore < 400)
                game_state->settings.starting_money = nextScore;
            break;
        case 1:
            nextScore = game_state->settings.round_price;
            if(game_state->selectDirection == DirectionLeft)
                nextScore -= 10;
            else
                nextScore += 10;
            if(nextScore >= 5 && nextScore <= (int)game_state->settings.starting_money)
                game_state->settings.round_price = nextScore;
            break;
        case 2:
            nextScore = game_state->settings.animation_duration;
            if(game_state->selectDirection == DirectionLeft)
                nextScore -= 100;
            else
                nextScore += 100;
            if(nextScore >= 0 && nextScore < 2000)
                game_state->settings.animation_duration = nextScore;
            break;
        case 3:
            nextScore = game_state->settings.message_duration;
            if(game_state->selectDirection == DirectionLeft)
                nextScore -= 100;
            else
                nextScore += 100;
            if(nextScore >= 0 && nextScore < 2000)
                game_state->settings.message_duration = nextScore;
            break;
        case 4:
            game_state->settings.sound_effects = !game_state->settings.sound_effects;
        default:
            break;
        }
    }
}

void tick(GameState* game_state) {
    game_state->last_tick = furi_get_tick();
    bool queue_ran = run_queue(&(game_state->queue_state), game_state);

    switch(game_state->state) {
    case GameStateGameOver:
    case GameStateStart:
        if(game_state->selectDirection == Select)
            init(game_state);
        else if(game_state->selectDirection == DirectionRight) {
            game_state->selectedMenu = 0;
            game_state->state = GameStateSettings;
        }
        break;
    case GameStatePlay:
        if(!game_state->started) {
            game_state->selectedMenu = 0;
            game_state->started = true;
            enqueue(
                &(game_state->queue_state),
                game_state,
                drawDealerCard,
                NULL,
                dealer_back_card_animation,
                game_state->settings.animation_duration);
            enqueue(
                &(game_state->queue_state),
                game_state,
                drawPlayerCard,
                NULL,
                player_card_animation,
                game_state->settings.animation_duration);
            enqueue(
                &(game_state->queue_state),
                game_state,
                drawDealerCard,
                NULL,
                dealer_card_animation,
                game_state->settings.animation_duration);
            enqueue(
                &(game_state->queue_state),
                game_state,
                drawPlayerCard,
                NULL,
                player_card_animation,
                game_state->settings.animation_duration);
        }
        if(!queue_ran) player_tick(game_state);
        break;
    case GameStateDealer:
        if(!queue_ran) dealer_tick(game_state);
        break;
    case GameStateSettings:
        settings_tick(game_state);
        break;
    default:
        break;
    }

    game_state->selectDirection = None;
}

void start_round(GameState* game_state) {
    game_state->menu->current_menu = 1;
    game_state->player_card_count = 0;
    game_state->dealer_card_count = 0;
    set_menu_state(game_state->menu, 0, true);
    game_state->menu->enabled = true;
    game_state->started = false;
    game_state->doubled = false;
    game_state->queue_state.running = true;
    shuffle_deck(&(game_state->deck));
    game_state->doubled = false;
    game_state->bet = game_state->settings.round_price;
    if(game_state->player_score < game_state->settings.round_price) {
        game_state->state = GameStateGameOver;
    } else {
        game_state->player_score -= game_state->settings.round_price;
    }
    game_state->state = GameStatePlay;
}

void init(GameState* game_state) {
    set_menu_state(game_state->menu, 0, true);
    game_state->menu->enabled = true;
    game_state->menu->current_menu = 1;
    game_state->settings = load_settings();
    game_state->last_tick = 0;
    game_state->processing = true;
    game_state->selectedMenu = 0;
    game_state->player_score = game_state->settings.starting_money;
    generate_deck(&(game_state->deck), 6);
    start_round(game_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);
    AppEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

static void update_timer_callback(FuriMessageQueue* event_queue) {
    furi_assert(event_queue);
    AppEvent event = {.type = EventTypeTick};
    furi_message_queue_put(event_queue, &event, 0);
}

void doubleAction(void* state) {
    GameState* game_state = state;
    if(!game_state->doubled && game_state->player_score >= game_state->settings.round_price) {
        game_state->player_score -= game_state->settings.round_price;
        game_state->bet += game_state->settings.round_price;
        game_state->doubled = true;
        enqueue(
            &(game_state->queue_state),
            game_state,
            drawPlayerCard,
            NULL,
            player_card_animation,
            game_state->settings.animation_duration);
        game_state->player_cards[game_state->player_card_count] =
            game_state->deck.cards[game_state->deck.index];
        uint8_t score = hand_count(game_state->player_cards, game_state->player_card_count + 1);
        if(score > 21) {
            enqueue(
                &(game_state->queue_state),
                game_state,
                lose,
                NULL,
                to_bust_state,
                game_state->settings.message_duration);
        } else {
            enqueue(
                &(game_state->queue_state),
                game_state,
                dealerTurn,
                NULL,
                to_dealer_turn,
                game_state->settings.message_duration);
        }
        set_menu_state(game_state->menu, 0, false);
    }
}

void hitAction(void* state) {
    GameState* game_state = state;
    enqueue(
        &(game_state->queue_state),
        game_state,
        drawPlayerCard,
        NULL,
        player_card_animation,
        game_state->settings.animation_duration);
}
void stayAction(void* state) {
    GameState* game_state = state;
    enqueue(
        &(game_state->queue_state),
        game_state,
        dealerTurn,
        NULL,
        to_dealer_turn,
        game_state->settings.message_duration);
}

int32_t blackjack_app(void* p) {
    UNUSED(p);

    int32_t return_code = 0;

    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(AppEvent));

    GameState* game_state = malloc(sizeof(GameState));
    game_state->menu = malloc(sizeof(Menu));
    game_state->menu->menu_width = 40;
    init(game_state);
    add_menu(game_state->menu, "Double", doubleAction);
    add_menu(game_state->menu, "Hit", hitAction);
    add_menu(game_state->menu, "Stay", stayAction);
    set_card_graphics(&I_card_graphics);

    game_state->state = GameStateStart;

    game_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    if(!game_state->mutex) {
        FURI_LOG_E(APP_NAME, "cannot create mutex\r\n");
        return_code = 255;
        goto free_and_exit;
    }

    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, game_state);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    FuriTimer* timer = furi_timer_alloc(update_timer_callback, FuriTimerTypePeriodic, event_queue);
    furi_timer_start(timer, furi_kernel_get_tick_frequency() / 25);

    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    AppEvent event;

    // Call dolphin deed on game start
    dolphin_deed(DolphinDeedPluginGameStart);

    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);
        furi_mutex_acquire(game_state->mutex, FuriWaitForever);
        if(event_status == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        game_state->selectDirection = DirectionUp;
                        break;
                    case InputKeyDown:
                        game_state->selectDirection = DirectionDown;
                        break;
                    case InputKeyRight:
                        game_state->selectDirection = DirectionRight;
                        break;
                    case InputKeyLeft:
                        game_state->selectDirection = DirectionLeft;
                        break;
                    case InputKeyBack:
                        if(game_state->state == GameStateSettings) {
                            game_state->state = GameStateStart;
                            save_settings(game_state->settings);
                        } else
                            processing = false;
                        break;
                    case InputKeyOk:
                        game_state->selectDirection = Select;
                        break;
                    default:
                        break;
                    }
                }
            } else if(event.type == EventTypeTick) {
                tick(game_state);
                processing = game_state->processing;
            }
        }
        view_port_update(view_port);
        furi_mutex_release(game_state->mutex);
    }

    furi_timer_free(timer);
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_mutex_free(game_state->mutex);

free_and_exit:
    free(game_state->deck.cards);
    free_menu(game_state->menu);
    queue_clear(&(game_state->queue_state));
    free(game_state);
    furi_message_queue_free(event_queue);

    return return_code;
}