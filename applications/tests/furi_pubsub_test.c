#include <stdio.h>
#include <string.h>
#include <furi.h>
#include "minunit.h"

const uint32_t context_value = 0xdeadbeef;
const uint32_t notify_value_0 = 0x12345678;
const uint32_t notify_value_1 = 0x11223344;

uint32_t pubsub_value = 0;
uint32_t pubsub_context_value = 0;

void test_pubsub_handler(const void* arg, void* ctx) {
    pubsub_value = *(uint32_t*)arg;
    pubsub_context_value = *(uint32_t*)ctx;
}

void test_furi_pubsub() {
    bool result;
    PubSub test_pubsub;
    PubSubItem* test_pubsub_item;

    // init pubsub case
    result = init_pubsub(&test_pubsub);
    mu_assert(result, "init pubsub failed");

    // subscribe pubsub case
    test_pubsub_item = subscribe_pubsub(&test_pubsub, test_pubsub_handler, (void*)&context_value);
    mu_assert_pointers_not_eq(test_pubsub_item, NULL);

    /// notify pubsub case
    result = notify_pubsub(&test_pubsub, (void*)&notify_value_0);
    mu_assert(result, "notify pubsub failed");
    mu_assert_int_eq(pubsub_value, notify_value_0);
    mu_assert_int_eq(pubsub_context_value, context_value);

    // unsubscribe pubsub case
    result = unsubscribe_pubsub(test_pubsub_item);
    mu_assert(result, "unsubscribe pubsub failed");

    result = unsubscribe_pubsub(test_pubsub_item);
    mu_assert(!result, "unsubscribe pubsub not failed");

    /// notify unsubscribed pubsub case
    result = notify_pubsub(&test_pubsub, (void*)&notify_value_1);
    mu_assert(result, "notify pubsub failed");
    mu_assert_int_not_eq(pubsub_value, notify_value_1);

    // delete pubsub case
    result = delete_pubsub(&test_pubsub);
    mu_assert(result, "unsubscribe pubsub failed");

    // TODO test case that the pubsub_delete will remove pubsub from heap
}