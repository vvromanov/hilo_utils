#include <zconf.h>
#include <execinfo.h>
#include "SignalWatcher.h"
#include "LogBase.h"

on_signal_t *on_sighup = nullptr;
on_signal_t *on_sigusr1 = nullptr;

static ev_signal sigint;
static ev_signal sighup;
static ev_signal sigusr1;
static ev_signal sigterm;
static ev_signal sigquit;
static ev_signal sigsegv;

static const char *signame(int signum) {
#define S(s) case s: return #s;
    switch (signum) {
        S(SIGHUP)
        S(SIGINT)
        S(SIGQUIT)
        S(SIGILL)
        S(SIGTRAP)
        S(SIGABRT)
        S(SIGBUS)
        S(SIGFPE)
        S(SIGKILL)
        S(SIGUSR1)
        S(SIGSEGV)
        S(SIGUSR2)
        S(SIGPIPE)
        S(SIGALRM)
        S(SIGTERM)
#ifndef CYGWIN
        S(SIGSTKFLT)
#endif
        S(SIGCHLD)
        S(SIGCONT)
        S(SIGSTOP)
        S(SIGTSTP)
        S(SIGTTIN)
        S(SIGTTOU)
        S(SIGURG)
        S(SIGXCPU)
        S(SIGXFSZ)
        S(SIGVTALRM)
        S(SIGPROF)
        S(SIGWINCH)
        S(SIGIO)
        S(SIGPWR)
        S(SIGSYS)
        default:
            return "UNKNOWN";
    }
#undef S
}

static void sig_cb(EV_P_ ev_signal *w, int revents) {
    log_write(LOG_LEVEL_NOTICE, "Take %s [%d] signal", signame(w->signum), w->signum);
    if (w->signum == SIGSEGV) {
        size_t size;
        void *array[100];
        size = backtrace(array, 100);
        backtrace_symbols_fd(array, size, STDERR_FILENO);
    }
    switch (w->signum) {
        case SIGHUP:
            if (on_sighup) {
                on_sighup(w->signum);
            }
            break;
        case SIGUSR1:
            if (on_sigusr1) {
                on_sigusr1(w->signum);
            }
            break;
        default:
            ev_unloop(EV_DEFAULT, EVUNLOOP_ALL);
    }
}

void InitSignalWatcher() {
    ev_signal_init(&sigint, sig_cb, SIGINT);
    ev_signal_init(&sighup, sig_cb, SIGHUP);
    ev_signal_init(&sigterm, sig_cb, SIGTERM);
    ev_signal_init(&sigquit, sig_cb, SIGQUIT);
    ev_signal_init(&sigsegv, sig_cb, SIGSEGV);
    ev_signal_init(&sigusr1, sig_cb, SIGUSR1);
    ev_signal_start(EV_DEFAULT_ &sigint);
    ev_signal_start(EV_DEFAULT_ &sighup);
    ev_signal_start(EV_DEFAULT_ &sigterm);
    ev_signal_start(EV_DEFAULT_ &sigquit);
    ev_signal_start(EV_DEFAULT_ &sigsegv);
    ev_signal_start(EV_DEFAULT_ &sigusr1);
}
