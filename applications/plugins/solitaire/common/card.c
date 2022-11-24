#include "card.h"
#include "dml.h"
#include "ui.h"

#define CARD_DRAW_X_START 108
#define CARD_DRAW_Y_START 38
#define CARD_DRAW_X_SPACE 10
#define CARD_DRAW_Y_SPACE 8
#define CARD_DRAW_X_OFFSET 4
#define CARD_DRAW_FIRST_ROW_LENGTH 7

uint8_t pips[4][3] = {
    {21, 10, 7}, //spades
    {7, 10, 7}, //hearts
    {0, 10, 7}, //diamonds
    {14, 10, 7}, //clubs
};
uint8_t letters[13][3] = {
    {0, 0, 5},
    {5, 0, 5},
    {10, 0, 5},
    {15, 0, 5},
    {20, 0, 5},
    {25, 0, 5},
    {30, 0, 5},
    {0, 5, 5},
    {5, 5, 5},
    {10, 5, 5},
    {15, 5, 5},
    {20, 5, 5},
    {25, 5, 5},
};

//region Player card positions
uint8_t playerCardPositions[22][4] = {
    //first row
    {108, 38},
    {98, 38},
    {88, 38},
    {78, 38},
    {68, 38},
    {58, 38},
    {48, 38},
    {38, 38},
    //second row
    {104, 26},
    {94, 26},
    {84, 26},
    {74, 26},
    {64, 26},
    {54, 26},
    {44, 26},
    //third row
    {99, 14},
    {89, 14},
    {79, 14},
    {69, 14},
    {59, 14},
    {49, 14},
};
//endregion
Icon* card_graphics = NULL;

void set_card_graphics(const Icon* graphics) {
    card_graphics = (Icon*)graphics;
}

void draw_card_at_colored(
    int8_t pos_x,
    int8_t pos_y,
    uint8_t pip,
    uint8_t character,
    bool inverted,
    Canvas* const canvas) {
    DrawMode primary = inverted ? Black : White;
    DrawMode secondary = inverted ? White : Black;
    draw_rounded_box(canvas, pos_x, pos_y, CARD_WIDTH, CARD_HEIGHT, primary);
    draw_rounded_box_frame(canvas, pos_x, pos_y, CARD_WIDTH, CARD_HEIGHT, Black);

    uint8_t* drawInfo = pips[pip];
    uint8_t px = drawInfo[0], py = drawInfo[1], s = drawInfo[2];

    uint8_t left = pos_x + 2;
    uint8_t right = (pos_x + CARD_WIDTH - s - 2);
    uint8_t top = pos_y + 2;
    uint8_t bottom = (pos_y + CARD_HEIGHT - s - 2);

    draw_icon_clip(canvas, card_graphics, right, top, px, py, s, s, secondary);
    draw_icon_clip_flipped(canvas, card_graphics, left, bottom, px, py, s, s, secondary);

    drawInfo = letters[character];
    px = drawInfo[0], py = drawInfo[1], s = drawInfo[2];
    left = pos_x + 2;
    right = (pos_x + CARD_WIDTH - s - 2);
    top = pos_y + 2;
    bottom = (pos_y + CARD_HEIGHT - s - 2);

    draw_icon_clip(canvas, card_graphics, left, top + 1, px, py, s, s, secondary);
    draw_icon_clip_flipped(canvas, card_graphics, right, bottom - 1, px, py, s, s, secondary);
}

void draw_card_at(int8_t pos_x, int8_t pos_y, uint8_t pip, uint8_t character, Canvas* const canvas) {
    draw_card_at_colored(pos_x, pos_y, pip, character, false, canvas);
}

void draw_deck(const Card* cards, uint8_t count, Canvas* const canvas) {
    for(int i = count - 1; i >= 0; i--) {
        draw_card_at(
            playerCardPositions[i][0],
            playerCardPositions[i][1],
            cards[i].pip,
            cards[i].character,
            canvas);
    }
}

Vector card_pos_at_index(uint8_t index) {
    return (Vector){playerCardPositions[index][0], playerCardPositions[index][1]};
}

void draw_card_back_at(int8_t pos_x, int8_t pos_y, Canvas* const canvas) {
    draw_rounded_box(canvas, pos_x, pos_y, CARD_WIDTH, CARD_HEIGHT, White);
    draw_rounded_box_frame(canvas, pos_x, pos_y, CARD_WIDTH, CARD_HEIGHT, Black);

    draw_icon_clip(canvas, card_graphics, pos_x + 1, pos_y + 1, 35, 0, 15, 21, Black);
}

void generate_deck(Deck* deck_ptr, uint8_t deck_count) {
    uint16_t counter = 0;
    if(deck_ptr->cards != NULL) {
        free(deck_ptr->cards);
    }

    deck_ptr->deck_count = deck_count;
    deck_ptr->card_count = deck_count * 52;
    deck_ptr->cards = malloc(sizeof(Card) * deck_ptr->card_count);

    for(uint8_t deck = 0; deck < deck_count; deck++) {
        for(uint8_t pip = 0; pip < 4; pip++) {
            for(uint8_t label = 0; label < 13; label++) {
                deck_ptr->cards[counter] = (Card){pip, label, false, false};
                counter++;
            }
        }
    }
}

void shuffle_deck(Deck* deck_ptr) {
    srand(DWT->CYCCNT);
    deck_ptr->index = 0;
    int max = deck_ptr->deck_count * 52;
    for(int i = 0; i < max; i++) {
        int r = i + (rand() % (max - i));
        Card tmp = deck_ptr->cards[i];
        deck_ptr->cards[i] = deck_ptr->cards[r];
        deck_ptr->cards[r] = tmp;
    }
}

uint8_t hand_count(const Card* cards, uint8_t count) {
    uint8_t aceCount = 0;
    uint8_t score = 0;

    for(uint8_t i = 0; i < count; i++) {
        if(cards[i].character == 12)
            aceCount++;
        else {
            if(cards[i].character > 8)
                score += 10;
            else
                score += cards[i].character + 2;
        }
    }

    for(uint8_t i = 0; i < aceCount; i++) {
        if((score + 11) <= 21)
            score += 11;
        else
            score++;
    }

    return score;
}

void draw_card_animation(
    Card animatingCard,
    Vector from,
    Vector control,
    Vector to,
    float t,
    bool extra_margin,
    Canvas* const canvas) {
    float time = t;
    if(extra_margin) {
        time += 0.2;
    }

    Vector currentPos = quadratic_2d(from, control, to, time);
    if(t > 1) {
        draw_card_at(
            currentPos.x, currentPos.y, animatingCard.pip, animatingCard.character, canvas);
    } else {
        if(t < 0.5)
            draw_card_back_at(currentPos.x, currentPos.y, canvas);
        else
            draw_card_at(
                currentPos.x, currentPos.y, animatingCard.pip, animatingCard.character, canvas);
    }
}

void init_hand(Hand* hand_ptr, uint8_t count) {
    hand_ptr->cards = malloc(sizeof(Card) * count);
    hand_ptr->index = 0;
    hand_ptr->max = count;
}

void free_hand(Hand* hand_ptr) {
    FURI_LOG_D("CARD", "Freeing hand");
    free(hand_ptr->cards);
}

void add_to_hand(Hand* hand_ptr, Card card) {
    FURI_LOG_D("CARD", "Adding to hand");
    if(hand_ptr->index < hand_ptr->max) {
        hand_ptr->cards[hand_ptr->index] = card;
        hand_ptr->index++;
    }
}

void draw_card_space(int16_t pos_x, int16_t pos_y, bool highlighted, Canvas* const canvas) {
    if(highlighted) {
        draw_rounded_box_frame(canvas, pos_x, pos_y, CARD_WIDTH, CARD_HEIGHT, Black);
        draw_rounded_box_frame(
            canvas, pos_x + 2, pos_y + 2, CARD_WIDTH - 4, CARD_HEIGHT - 4, White);
    } else {
        draw_rounded_box(canvas, pos_x, pos_y, CARD_WIDTH, CARD_HEIGHT, Black);
        draw_rounded_box_frame(
            canvas, pos_x + 2, pos_y + 2, CARD_WIDTH - 4, CARD_HEIGHT - 4, White);
    }
}

int first_non_flipped_card(Hand hand) {
    for(int i = 0; i < hand.index; i++) {
        if(!hand.cards[i].flipped) {
            return i;
        }
    }
    return hand.index;
}

void draw_hand_column(
    Hand hand,
    int16_t pos_x,
    int16_t pos_y,
    int8_t highlight,
    Canvas* const canvas) {
    if(hand.index == 0) {
        draw_card_space(pos_x, pos_y, highlight > 0, canvas);
        if(highlight == 0)
            draw_rounded_box(canvas, pos_x, pos_y, CARD_WIDTH, CARD_HEIGHT, Inverse);
        return;
    }

    int loopEnd = hand.index;
    int hStart = max(loopEnd - 4, 0);
    int pos = 0;
    int first = first_non_flipped_card(hand);
    bool wastop = false;
    if(first >= 0 && first <= hStart && highlight != first) {
        if(first > 0) {
            draw_card_back_at(pos_x, pos_y + pos, canvas);
            pos += 4;
            hStart++;
            wastop = true;
        }
        draw_card_at_colored(
            pos_x, pos_y + pos, hand.cards[first].pip, hand.cards[first].character, false, canvas);
        pos += 8;
        hStart++;
    }
    if(hStart > highlight && highlight >= 0) {
        if(!wastop && first > 0) {
            draw_card_back_at(pos_x, pos_y + pos, canvas);
            pos += 4;
            hStart++;
        }
        draw_card_at_colored(
            pos_x,
            pos_y + pos,
            hand.cards[highlight].pip,
            hand.cards[highlight].character,
            true,
            canvas);
        pos += 8;
        hStart++;
    }
    for(int i = hStart; i < loopEnd; i++, pos += 4) {
        if(hand.cards[i].flipped) {
            draw_card_back_at(pos_x, pos_y + pos, canvas);
            if(i == highlight)
                draw_rounded_box(
                    canvas, pos_x + 1, pos_y + pos + 1, CARD_WIDTH - 2, CARD_HEIGHT - 2, Inverse);
        } else {
            draw_card_at_colored(
                pos_x,
                pos_y + pos,
                hand.cards[i].pip,
                hand.cards[i].character,
                (i == highlight),
                canvas);
            if(i == highlight || i == first) pos += 4;
        }
    }
}

Card remove_from_deck(uint16_t index, Deck* deck) {
    FURI_LOG_D("CARD", "Removing from deck");
    Card result = {0, 0, true, false};
    if(deck->card_count > 0) {
        deck->card_count--;
        for(int i = 0, curr_index = 0; i <= deck->card_count; i++) {
            if(i != index) {
                deck->cards[curr_index] = deck->cards[i];
                curr_index++;
            } else {
                result = deck->cards[i];
            }
        }
        if(deck->index >= 0) {
            deck->index--;
        }
    }
    return result;
}

void extract_hand_region(Hand* hand, Hand* to, uint8_t start_index) {
    FURI_LOG_D("CARD", "Extracting hand region");
    if(start_index >= hand->index) return;

    for(uint8_t i = start_index; i < hand->index; i++) {
        add_to_hand(to, hand->cards[i]);
    }
    hand->index = start_index;
}

void add_hand_region(Hand* to, Hand* from) {
    FURI_LOG_D("CARD", "Adding hand region");
    if((to->index + from->index) <= to->max) {
        for(int i = 0; i < from->index; i++) {
            add_to_hand(to, from->cards[i]);
        }
    }
}