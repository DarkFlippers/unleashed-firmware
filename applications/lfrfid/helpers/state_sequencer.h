#pragma once
#include "stdint.h"
#include <list>
#include <functional>

class TickSequencer {
public:
    TickSequencer();
    ~TickSequencer();

    void tick();
    void reset();
    void clear();

    void do_every_tick(uint32_t tick_count, std::function<void(void)> fn);
    void do_after_tick(uint32_t tick_count, std::function<void(void)> fn);

private:
    std::list<std::pair<uint32_t, std::function<void(void)> > > list;
    std::list<std::pair<uint32_t, std::function<void(void)> > >::iterator list_it;

    uint32_t tick_count;

    void do_nothing();
};
