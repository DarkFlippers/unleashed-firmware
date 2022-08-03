#include <stdio.h>
#include <string.h>
#include <furi.h>
#include "../minunit.h"

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
    FuriPubSub* test_pubsub = NULL;
    FuriPubSubSubscription* test_pubsub_subscription = NULL;

    // init pubsub case
    test_pubsub = furi_pubsub_alloc();
    mu_assert_pointers_not_eq(test_pubsub, NULL);

    // subscribe pubsub case
    test_pubsub_subscription =
        furi_pubsub_subscribe(test_pubsub, test_pubsub_handler, (void*)&context_value);
    mu_assert_pointers_not_eq(test_pubsub_subscription, NULL);

    /// notify pubsub case
    furi_pubsub_publish(test_pubsub, (void*)&notify_value_0);
    mu_assert_int_eq(pubsub_value, notify_value_0);
    mu_assert_int_eq(pubsub_context_value, context_value);

    // unsubscribe pubsub case
    furi_pubsub_unsubscribe(test_pubsub, test_pubsub_subscription);

    /// notify unsubscribed pubsub case
    furi_pubsub_publish(test_pubsub, (void*)&notify_value_1);
    mu_assert_int_not_eq(pubsub_value, notify_value_1);

    // delete pubsub case
    furi_pubsub_free(test_pubsub);
}
