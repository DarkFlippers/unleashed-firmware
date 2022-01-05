#include "state_sequencer.h"
#include "stdio.h"

TickSequencer::TickSequencer() {
}

TickSequencer::~TickSequencer() {
}

void TickSequencer::tick() {
    if(tick_count == list_it->first) {
        tick_count = 0;

        list_it++;
        if(list_it == list.end()) {
            list_it = list.begin();
        }
    }

    list_it->second();
    tick_count++;
}

void TickSequencer::reset() {
    list_it = list.begin();
    tick_count = 0;
}

void TickSequencer::clear() {
    list.clear();
    reset();
}

void TickSequencer::do_every_tick(uint32_t tick_count, std::function<void(void)> fn) {
    list.push_back(std::make_pair(tick_count, fn));
    reset();
}

void TickSequencer::do_after_tick(uint32_t tick_count, std::function<void(void)> fn) {
    if(tick_count > 1) {
        list.push_back(
            std::make_pair(tick_count - 1, std::bind(&TickSequencer::do_nothing, this)));
    }
    list.push_back(std::make_pair(1, fn));

    reset();
}

void TickSequencer::do_nothing() {
}
