#include <stdio.h>
#include <furi.h>

#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification.h>
#include <notification/notification_messages.h>
#include "bomberduck_icons.h"
#include <dolphin/dolphin.h>

int max(int a, int b) {
    return (a > b) ? a : b;
}

int min(int a, int b) {
    return (a < b) ? a : b;
}

#define WorldSizeX 12
#define WorldSizeY 6
#define BombRange 1

typedef struct {
    FuriMutex* mutex;
} BomberState;

typedef struct {
    int row;
    int col;
} Cell;

typedef struct {
    Cell cells[WorldSizeY * WorldSizeX];
    int front;
    int rear;
} Queue;

void enqueue(Queue* q, Cell c) {
    q->cells[q->rear] = c;
    q->rear++;
}

Cell dequeue(Queue* q) {
    Cell c = q->cells[q->front];
    q->front++;

    return c;
}

bool is_empty(Queue* q) {
    return q->front == q->rear;
}

typedef struct {
    int x;
    int y;
    int planted;
} Bomb;

typedef struct {
    int x;
    int y;
    bool side;
} Player;

typedef struct {
    int x;
    int y;
    int last;
    bool side;
    int level;
} Enemy;

typedef struct {
    int matrix[WorldSizeY][WorldSizeX];
    Player* player;
    bool running;
    int level;

    Enemy enemies[10];
    int enemies_count;

    Bomb bombs[100];
    int bombs_count;

    int endx;
    int endy;
} World;

Player player = {0, 0, 1};
World world = {{{0}}, &player, 1, 0, {}, 0, {}, 0, 0, 0};
bool vibration = false;

void init() {
    player.x = 1;
    player.y = 1;

    world.endx = 4 + rand() % 8;
    world.endy = rand() % 6;
    for(int i = 0; i < WorldSizeY; i++) {
        for(int j = 0; j < WorldSizeX; j++) {
            world.matrix[i][j] = rand() % 3;
        }
    }
    world.running = 1;
    world.bombs_count = 0;
    vibration = false;
    for(int j = max(0, player.y - BombRange); j < min(WorldSizeY, player.y + BombRange + 1); j++) {
        world.matrix[j][player.x] = 0;
    }

    for(int j = max(0, player.x - BombRange); j < min(WorldSizeX, player.x + BombRange + 1); j++) {
        world.matrix[player.y][j] = 0;
    }

    world.enemies_count = 0;
    for(int j = 0; j < rand() % 4 + world.level / 5; j++) {
        Enemy enemy;
        enemy.x = 4 + rand() % 7;
        enemy.y = rand() % 6;
        enemy.last = 0;
        enemy.side = 1;
        enemy.level = 0;

        world.enemies[j] = enemy;
        world.enemies_count++;

        for(int m = max(0, world.enemies[j].y - BombRange);
            m < min(WorldSizeY, world.enemies[j].y + BombRange + 1);
            m++) {
            world.matrix[m][world.enemies[j].x] = 0;
        }

        for(int m = max(0, world.enemies[j].x - BombRange);
            m < min(WorldSizeX, world.enemies[j].x + BombRange + 1);
            m++) {
            world.matrix[world.enemies[j].y][m] = 0;
        }
    }
    world.matrix[world.endy][world.endx] = 1;
}

const NotificationSequence end = {
    &message_vibro_on,

    &message_note_ds4,
    &message_delay_10,
    &message_sound_off,
    &message_delay_10,

    &message_note_ds4,
    &message_delay_10,
    &message_sound_off,
    &message_delay_10,

    &message_note_ds4,
    &message_delay_10,
    &message_sound_off,
    &message_delay_10,

    &message_vibro_off,
    NULL,
};

static const NotificationSequence bomb2 = {
    &message_vibro_on,
    &message_delay_25,
    &message_vibro_off,
    NULL,
};

static const NotificationSequence bomb_explore = {
    &message_vibro_on,
    &message_delay_50,
    &message_vibro_off,
    NULL,
};

static const NotificationSequence vibr1 = {
    &message_vibro_on,
    &message_delay_10,
    &message_vibro_off,
    &message_delay_10,
    &message_vibro_on,
    &message_delay_10,
    &message_vibro_off,
    &message_delay_10,

    NULL,
};

void intToStr(int num, char* str) {
    int i = 0, sign = 0;

    if(num < 0) {
        num = -num;
        sign = 1;
    }

    do {
        str[i++] = num % 10 + '0';
        num /= 10;
    } while(num > 0);

    if(sign) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse the string
    int j, len = i;
    char temp;
    for(j = 0; j < len / 2; j++) {
        temp = str[j];
        str[j] = str[len - j - 1];
        str[len - j - 1] = temp;
    }
}

bool BFS() {
    // Initialize visited array and queue
    int visited[WorldSizeY][WorldSizeX] = {0};
    Queue q = {.front = 0, .rear = 0};
    // Mark the starting cell as visited and enqueue it
    visited[world.player->y][world.player->x] = 1;
    Cell startCell = {.row = world.player->y, .col = world.player->x};
    enqueue(&q, startCell);
    // Traverse the field
    while(!is_empty(&q)) {
        // Dequeue a cell from the queue
        Cell currentCell = dequeue(&q);
        // Check if the current cell is the destination cell
        if(currentCell.row == world.endy && currentCell.col == world.endx) {
            return true;
        }
        // Check the neighboring cells
        for(int rowOffset = -1; rowOffset <= 1; rowOffset++) {
            for(int colOffset = -1; colOffset <= 1; colOffset++) {
                // Skip diagonals and the current cell
                if(rowOffset == 0 && colOffset == 0) {
                    continue;
                }
                if(rowOffset != 0 && colOffset != 0) {
                    continue;
                }
                // Calculate the row and column of the neighboring cell
                int neighborRow = currentCell.row + rowOffset;
                int neighborCol = currentCell.col + colOffset;
                // Skip out-of-bounds cells and already visited cells
                if(neighborRow < 0 || neighborRow >= WorldSizeY || neighborCol < 0 ||
                   neighborCol >= WorldSizeX) {
                    continue;
                }
                if(visited[neighborRow][neighborCol]) {
                    continue;
                }
                // Mark the neighboring cell as visited and enqueue it
                if(world.matrix[neighborRow][neighborCol] != 2) {
                    visited[neighborRow][neighborCol] = 1;
                    Cell neighborCell = {.row = neighborRow, .col = neighborCol};
                    enqueue(&q, neighborCell);
                }
            }
        }
    }
    return false;
}

static void draw_callback(Canvas* canvas, void* ctx) {
    furi_assert(ctx);
    const BomberState* bomber_state = ctx;

    furi_mutex_acquire(bomber_state->mutex, FuriWaitForever);
    if(!BFS()) {
        init();
    }
    canvas_clear(canvas);

    canvas_draw_icon(canvas, world.endx * 10 + 4, world.endy * 10 + 2, &I_end);

    if(world.running) {
        for(size_t i = 0; i < WorldSizeY; i++) {
            for(size_t j = 0; j < WorldSizeX; j++) {
                switch(world.matrix[i][j]) {
                case 0:
                    break;
                case 1:
                    canvas_draw_icon(canvas, j * 10 + 4, i * 10 + 2, &I_box);
                    break;
                case 2:
                    canvas_draw_icon(canvas, j * 10 + 4, i * 10 + 2, &I_unbreakbox);
                    break;
                case 3:
                    canvas_draw_icon(canvas, j * 10 + 4, i * 10 + 2, &I_bomb0);
                    break;
                case 4:
                    canvas_draw_icon(canvas, j * 10 + 4, i * 10 + 2, &I_bomb1);
                    break;
                case 5:
                    canvas_draw_icon(canvas, j * 10 + 4, i * 10 + 2, &I_bomb2);
                    break;
                case 6:
                    canvas_draw_icon(canvas, j * 10 + 4, i * 10 + 2, &I_explore);
                    world.matrix[i][j] = 0;
                    break;
                }
            }
        }

        if(world.player->side) {
            canvas_draw_icon(
                canvas, world.player->x * 10 + 4, world.player->y * 10 + 2, &I_playerright);
        } else {
            canvas_draw_icon(
                canvas, world.player->x * 10 + 4, world.player->y * 10 + 2, &I_playerleft);
        }

        for(int i = 0; i < world.enemies_count; i++) {
            if(world.enemies[i].level > 0) {
                canvas_draw_icon(
                    canvas, world.enemies[i].x * 10 + 4, world.enemies[i].y * 10 + 2, &I_enemy1);
            } else {
                if(world.enemies[i].side) {
                    canvas_draw_icon(
                        canvas,
                        world.enemies[i].x * 10 + 4,
                        world.enemies[i].y * 10 + 2,
                        &I_enemyright);
                } else {
                    canvas_draw_icon(
                        canvas,
                        world.enemies[i].x * 10 + 4,
                        world.enemies[i].y * 10 + 2,
                        &I_enemyleft);
                }
            }
        }
    } else {
        canvas_set_font(canvas, FontPrimary);
        if(world.player->x == world.endx && world.player->y == world.endy) {
            if(world.level == 20) {
                canvas_draw_str(canvas, 30, 35, "You win!");
            } else {
                canvas_draw_str(canvas, 30, 35, "Next level!");
                char str[20];
                intToStr(world.level, str);
                canvas_draw_str(canvas, 90, 35, str);
            }

        } else {
            canvas_draw_str(canvas, 30, 35, "You died :(");
        }
    }

    furi_mutex_release(bomber_state->mutex);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    // Проверяем, что контекст не нулевой
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;

    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t bomberduck_app(void* p) {
    UNUSED(p);

    // Текущее событие типа InputEvent
    InputEvent event;
    // Очередь событий на 8 элементов размера InputEvent
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    BomberState* bomber_state = malloc(sizeof(BomberState));

    bomber_state->mutex = furi_mutex_alloc(FuriMutexTypeNormal); // Alloc Mutex
    if(!bomber_state->mutex) {
        FURI_LOG_E("BomberDuck", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(bomber_state);
        return 255;
    }

    dolphin_deed(DolphinDeedPluginGameStart);
    // Создаем новый view port
    ViewPort* view_port = view_port_alloc();
    // Создаем callback отрисовки, без контекста
    view_port_draw_callback_set(view_port, draw_callback, bomber_state);
    // Создаем callback нажатий на клавиши, в качестве контекста передаем
    // нашу очередь сообщений, чтоб запихивать в неё эти события
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Создаем GUI приложения
    Gui* gui = furi_record_open(RECORD_GUI);
    // Подключаем view port к GUI в полноэкранном режиме
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message_block(notification, &sequence_display_backlight_enforce_on);

    init();

    // Бесконечный цикл обработки очереди событий
    while(1) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            furi_mutex_acquire(bomber_state->mutex, FuriWaitForever);
            // Если нажата кнопка "назад", то выходим из цикла, а следовательно и из приложения

            if(event.type == InputTypePress) {
                if(event.key == InputKeyOk) {
                    if(world.running) {
                        if(world.matrix[world.player->y][world.player->x] == 0 &&
                           world.bombs_count < 2) {
                            notification_message(notification, &bomb2);
                            world.matrix[world.player->y][world.player->x] = 3;
                            Bomb bomb = {world.player->x, world.player->y, furi_get_tick()};
                            world.bombs[world.bombs_count] = bomb;
                            world.bombs_count++;
                        }
                    } else {
                        init();
                    }
                }
                if(world.running) {
                    if(event.key == InputKeyUp) {
                        if(world.player->y > 0 &&
                           world.matrix[world.player->y - 1][world.player->x] == 0)
                            world.player->y--;
                    }
                    if(event.key == InputKeyDown) {
                        if(world.player->y < WorldSizeY - 1 &&
                           world.matrix[world.player->y + 1][world.player->x] == 0)
                            world.player->y++;
                    }
                    if(event.key == InputKeyLeft) {
                        world.player->side = 0;
                        if(world.player->x > 0 &&
                           world.matrix[world.player->y][world.player->x - 1] == 0)
                            world.player->x--;
                    }
                    if(event.key == InputKeyRight) {
                        world.player->side = 1;
                        if(world.player->x < WorldSizeX - 1 &&
                           world.matrix[world.player->y][world.player->x + 1] == 0)
                            world.player->x++;
                    }
                }
            } else if(event.type == InputTypeLong) {
                if(event.key == InputKeyBack) {
                    break;
                }
            }
        }
        if(world.running) {
            if(world.player->x == world.endx && world.player->y == world.endy) {
                notification_message(notification, &end);
                world.running = 0;
                world.level += 1;
                if(world.level % 5 == 0) {
                    dolphin_deed(DolphinDeedPluginGameWin);
                }
            }
            for(int i = 0; i < world.bombs_count; i++) {
                if(furi_get_tick() - world.bombs[i].planted >
                   (unsigned long)max((3000 - world.level * 150), 1000)) {
                    vibration = false;
                    world.matrix[world.bombs[i].y][world.bombs[i].x] = 6;
                    notification_message(notification, &bomb_explore);

                    for(int j = max(0, world.bombs[i].y - BombRange);
                        j < min(WorldSizeY, world.bombs[i].y + BombRange + 1);
                        j++) {
                        if(world.matrix[j][world.bombs[i].x] != 2) {
                            world.matrix[j][world.bombs[i].x] = 6;
                            if(j == world.player->y && world.bombs[i].x == world.player->x) {
                                notification_message(notification, &end);
                                world.running = 0;
                            }
                            for(int e = 0; e < world.enemies_count; e++) {
                                if(j == world.enemies[e].y &&
                                   world.bombs[i].x == world.enemies[e].x) {
                                    if(world.enemies[e].level > 0) {
                                        world.enemies[e].level--;
                                    } else {
                                        for(int l = e; l < world.enemies_count - 1; l++) {
                                            world.enemies[l] = world.enemies[l + 1];
                                        }
                                        world.enemies_count--;
                                    }
                                }
                            }
                        }
                    }

                    for(int j = max(0, world.bombs[i].x - BombRange);
                        j < min(WorldSizeX, world.bombs[i].x + BombRange + 1);
                        j++) {
                        if(world.matrix[world.bombs[i].y][j] != 2) {
                            world.matrix[world.bombs[i].y][j] = 6;
                            if(world.bombs[i].y == world.player->y && j == world.player->x) {
                                notification_message(notification, &end);
                                world.running = 0;
                            }
                            for(int e = 0; e < world.enemies_count; e++) {
                                if(world.bombs[i].y == world.enemies[e].y &&
                                   j == world.enemies[e].x) {
                                    if(world.enemies[e].level > 0) {
                                        world.enemies[e].level--;
                                    } else {
                                        for(int l = e; l < world.enemies_count - 1; l++) {
                                            world.enemies[l] = world.enemies[l + 1];
                                        }
                                        world.enemies_count--;
                                    }
                                }
                            }
                        }
                    }

                    for(int j = i; j < world.bombs_count - 1; j++) {
                        world.bombs[j] = world.bombs[j + 1];
                    }
                    world.bombs_count--;
                } else if(
                    furi_get_tick() - world.bombs[i].planted >
                        (unsigned long)max((3000 - world.level * 150) * 2 / 3, 666) &&
                    world.matrix[world.bombs[i].y][world.bombs[i].x] != 5) {
                    world.matrix[world.bombs[i].y][world.bombs[i].x] = 5;
                    vibration = true;

                } else if(
                    furi_get_tick() - world.bombs[i].planted >
                        (unsigned long)max((3000 - world.level * 150) / 3, 333) &&
                    world.matrix[world.bombs[i].y][world.bombs[i].x] != 4) {
                    world.matrix[world.bombs[i].y][world.bombs[i].x] = 4;
                }
            }
            for(int e = 0; e < world.enemies_count; e++) {
                if(world.player->y == world.enemies[e].y &&
                   world.player->x == world.enemies[e].x) {
                    notification_message(notification, &end);
                    world.running = 0;
                }
            }

            for(int e = 0; e < world.enemies_count; e++) {
                if(world.enemies[e].level > 0) {
                    if(furi_get_tick() - world.enemies[e].last >
                       (unsigned long)max((2000 - world.level * 100), 1000)) {
                        world.enemies[e].last = furi_get_tick();
                        int move = rand() % 4;
                        switch(move) {
                        case 0:
                            if(world.enemies[e].y > 0 &&
                               world.matrix[world.enemies[e].y - 1][world.enemies[e].x] != 2)
                                world.enemies[e].y--;
                            break;
                        case 1:
                            if(world.enemies[e].y < WorldSizeY - 1 &&
                               world.matrix[world.enemies[e].y + 1][world.enemies[e].x] != 2)
                                world.enemies[e].y++;
                            break;
                        case 2:
                            world.enemies[e].side = 0;
                            if(world.enemies[e].x > 0 &&
                               world.matrix[world.enemies[e].y][world.enemies[e].x - 1] != 2)
                                world.enemies[e].x--;
                            break;
                        case 3:
                            world.enemies[e].side = 1;
                            if(world.enemies[e].x < WorldSizeX - 1 &&
                               world.matrix[world.enemies[e].y][world.enemies[e].x + 1] != 2)
                                world.enemies[e].x++;
                        default:
                            break;
                        }
                    }
                } else {
                    if(furi_get_tick() - world.enemies[e].last >
                       (unsigned long)max((1000 - world.level * 50), 500)) {
                        world.enemies[e].last = furi_get_tick();
                        int move = rand() % 4;
                        switch(move) {
                        case 0:
                            if(world.enemies[e].y > 0 &&
                               world.matrix[world.enemies[e].y - 1][world.enemies[e].x] == 0)
                                world.enemies[e].y--;
                            break;
                        case 1:
                            if(world.enemies[e].y < WorldSizeY - 1 &&
                               world.matrix[world.enemies[e].y + 1][world.enemies[e].x] == 0)
                                world.enemies[e].y++;
                            break;
                        case 2:
                            world.enemies[e].side = 0;
                            if(world.enemies[e].x > 0 &&
                               world.matrix[world.enemies[e].y][world.enemies[e].x - 1] == 0)
                                world.enemies[e].x--;
                            break;
                        case 3:
                            world.enemies[e].side = 1;
                            if(world.enemies[e].x < WorldSizeX - 1 &&
                               world.matrix[world.enemies[e].y][world.enemies[e].x + 1] == 0)
                                world.enemies[e].x++;
                        default:
                            break;
                        }
                    }
                }
            }
            for(int e = 0; e < world.enemies_count; e++) {
                for(int h = e + 1; h < world.enemies_count; h++) {
                    if(world.enemies[e].y == world.enemies[h].y &&
                       world.enemies[e].x == world.enemies[h].x) {
                        world.enemies[h].level++;
                        for(int l = e; l < world.enemies_count - 1; l++) {
                            world.enemies[l] = world.enemies[l + 1];
                        }
                        world.enemies_count--;
                    }
                }
            }
            if(vibration) {
                notification_message(notification, &vibr1);
            }
        }

        view_port_update(view_port);
        furi_mutex_release(bomber_state->mutex);
    }

    // Return to normal backlight settings
    notification_message_block(notification, &sequence_display_backlight_enforce_auto);
    furi_record_close(RECORD_NOTIFICATION);
    // Специальная очистка памяти, занимаемой очередью
    furi_message_queue_free(event_queue);

    // Чистим созданные объекты, связанные с интерфейсом
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);

    furi_mutex_free(bomber_state->mutex);
    furi_record_close(RECORD_GUI);
    free(bomber_state);

    return 0;
}
