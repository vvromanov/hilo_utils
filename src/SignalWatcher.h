#pragma once

#include "ev.h"

typedef void on_signal_t(int signum);
extern on_signal_t* on_sighup;
extern on_signal_t* on_sigusr1;

void InitSignalWatcher();
