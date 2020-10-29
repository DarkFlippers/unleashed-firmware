#include "flipper_v2.h"
#include "minunit.h"
#include <stdint.h>

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Rgb;

static uint32_t rgb_final_state;

static void rgb_clear(void* ctx, void* state) {
    Rgb* rgb = state;
    rgb->red = 0;
    rgb->green = 0;
    rgb->blue = 0;
}

static void rgb_commit(void* ctx, void* state) {
    Rgb* rgb = state;
    rgb_final_state = ((uint32_t)rgb->red) | (((uint32_t)rgb->green) << 8) |
                      (((uint32_t)rgb->blue) << 16);
}

static void set_red_composer(void* ctx, void* state) {
    Rgb* rgb = state;
    uint8_t* red = ctx;

    rgb->red = *red;
}

void test_furi_value_composer() {
    Rgb rgb = {0, 0, 0};
    ValueComposer composer;
    Rgb layer1_rgb = {0, 0, 0};
    ValueMutex layer1_mutex;
    uint8_t layer2_red = 0;

    rgb_final_state = 0xdeadbeef;

    mu_check(init_composer(&composer, &rgb));

    mu_check(init_mutex(&layer1_mutex, &layer1_rgb, sizeof(layer1_rgb)));

    perform_compose(&composer, rgb_clear, rgb_commit, NULL);
    mu_assert_int_eq(0xdeadbeef, rgb_final_state);

    ValueComposerHandle* layer1_handle =
        add_compose_layer(&composer, COPY_COMPOSE, &layer1_mutex, UiLayerNotify);
    mu_assert_pointers_not_eq(layer1_handle, NULL);

    // RGB state should be updated with the layer1 state
    perform_compose(&composer, rgb_clear, rgb_commit, NULL);
    mu_assert_int_eq(0x000000, rgb_final_state);

    layer2_red = 0xcc;
    ValueComposerHandle* layer2_handle =
        add_compose_layer(&composer, set_red_composer, &layer2_red, UiLayerAboveNotify);
    mu_assert_pointers_not_eq(layer2_handle, NULL);

    // RGB state should be updated with the layer1 and layer2 state, in order
    perform_compose(&composer, rgb_clear, rgb_commit, NULL);
    mu_assert_int_eq(0x0000cc, rgb_final_state);

    // Change layer1 state
    Rgb* state = acquire_mutex(&layer1_mutex, 0);
    mu_assert_pointers_not_eq(state, NULL);
    state->red = 0x12;
    state->green = 0x34;
    state->blue = 0x56;
    release_mutex(&layer1_mutex, state);

    // Nothing should happen, we need to trigger composition request first
    perform_compose(&composer, rgb_clear, rgb_commit, NULL);
    mu_assert_int_eq(0x0000cc, rgb_final_state);

    request_compose(layer1_handle);

    perform_compose(&composer, rgb_clear, rgb_commit, NULL);
    mu_assert_int_eq(0x5634cc, rgb_final_state);

    // Change layer2 state
    layer2_red = 0xff;

    // Nothing should happen, we need to trigger composition request first
    perform_compose(&composer, rgb_clear, rgb_commit, NULL);
    mu_assert_int_eq(0x5634cc, rgb_final_state);

    request_compose(layer2_handle);

    perform_compose(&composer, rgb_clear, rgb_commit, NULL);
    mu_assert_int_eq(0x5634ff, rgb_final_state);

    // Remove layer1
    mu_check(remove_compose_layer(layer1_handle));

    perform_compose(&composer, rgb_clear, rgb_commit, NULL);
    mu_assert_int_eq(0x0000ff, rgb_final_state);

    // Remove layer2
    mu_check(remove_compose_layer(layer2_handle));

    perform_compose(&composer, rgb_clear, rgb_commit, NULL);
    mu_assert_int_eq(0x000000, rgb_final_state);

    mu_check(delete_composer(&composer));
}

static const uint32_t notify_value_0 = 0x12345678;
static const uint32_t notify_value_1 = 0x11223344;

static uint32_t pubsub_value = 0;

void test_value_manager_handler(const void* arg, void* ctx) {
    pubsub_value = *(uint32_t*)arg;
}

void test_furi_value_manager() {
    uint32_t value = 0;
    ValueManager managed;

    mu_check(init_managed(&managed, &value, sizeof(value)));

    pubsub_value = 0;

    PubSubItem* test_pubsub_item;
    test_pubsub_item = subscribe_pubsub(&managed.pubsub, test_value_manager_handler, 0);
    mu_assert_pointers_not_eq(test_pubsub_item, NULL);

    mu_check(write_managed(&managed, (void*)&notify_value_0, sizeof(notify_value_0), 100));

    mu_assert_int_eq(pubsub_value, notify_value_0);

    uint32_t* ptr = acquire_mutex(&managed.value, 100);
    mu_assert_pointers_not_eq(ptr, NULL);

    *ptr = notify_value_1;

    mu_check(commit_managed(&managed, ptr));

    mu_assert_int_eq(pubsub_value, notify_value_1);

    mu_check(delete_managed(&managed));
}
