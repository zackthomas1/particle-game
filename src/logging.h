#pragma once
#include <stdio.h>
#include <stdlib.h>

#define PASSERT(cond, fmsg, ...) if(!(cond)) {fprintf(stderr, "Assertion Failed: %s.\n", fmsg); return; }