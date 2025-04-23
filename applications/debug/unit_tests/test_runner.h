#pragma once

#include <furi.h>
#include <toolbox/pipe.h>

typedef struct TestRunner TestRunner;

TestRunner* test_runner_alloc(PipeSide* pipe, FuriString* args);

void test_runner_free(TestRunner* instance);

void test_runner_run(TestRunner* instance);
