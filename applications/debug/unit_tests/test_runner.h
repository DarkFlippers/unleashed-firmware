#pragma once

#include <furi.h>

typedef struct TestRunner TestRunner;
typedef struct Cli Cli;

TestRunner* test_runner_alloc(Cli* cli, FuriString* args);

void test_runner_free(TestRunner* isntance);

void test_runner_run(TestRunner* isntance);
