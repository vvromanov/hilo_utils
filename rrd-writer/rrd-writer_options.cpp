#include "rrd-writer_options.h"
#include <argp.h>
#include <common_utils.h>
#include <counters.h>
#include <LogOptions.h>

bool opt_verbose = false;

custom_prefix_t custom_prefix[MAX_CUSTOM_COUNT];
int custom_prefix_count = 0;

/* Parse a single option. */
static error_t option_parse(int key, char *arg, struct argp_state *state) {
    PARSE_LOG_OPTIONS
//    int64_t tmp;
    switch (key) {
        case 'v':
            opt_verbose = true;
            break;
        case OPT_PREFIX_HISTORY:
            if (custom_prefix_count == MAX_CUSTOM_COUNT) {
                fprintf(stderr, "Too many prefixes\n");
                return ARGP_ERR_UNKNOWN;
            }
            custom_prefix[custom_prefix_count].type = prefix_history;
            STRNCPY(custom_prefix[custom_prefix_count].name, arg);
            ++custom_prefix_count;
            break;
        case OPT_PREFIX_VALUE:
            if (custom_prefix_count == MAX_CUSTOM_COUNT) {
                fprintf(stderr, "Too many prefixes\n");
                return ARGP_ERR_UNKNOWN;
            }
            custom_prefix[custom_prefix_count].type = prefix_value;
            STRNCPY(custom_prefix[custom_prefix_count].name, arg);
            ++custom_prefix_count;
            break;
        case OPT_PREFIX_INCREMENTED:
            if (custom_prefix_count == MAX_CUSTOM_COUNT) {
                fprintf(stderr, "Too many prefixes\n");
                return ARGP_ERR_UNKNOWN;
            }
            custom_prefix[custom_prefix_count].type = prefix_incremented;
            STRNCPY(custom_prefix[custom_prefix_count].name, arg);
            ++custom_prefix_count;
            break;
        case OPT_VALUE_SUFFIX:
            counters_suffix = arg;
            break;
        case ARGP_KEY_ARG:
            switch (state->arg_num) {
                case 0:
                    break;
                default:
                    argp_usage(state);
            }
            break;
        case ARGP_KEY_END:
//            if (state->arg_num < 3)
//                argp_usage(state);
            break;
        case ARGP_KEY_INIT:
        case ARGP_KEY_FINI:
        case ARGP_KEY_NO_ARGS:
        case ARGP_KEY_SUCCESS:
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* A description of the arguments we accept. */
static char args_doc[] = "";

/* Program documentation. */
static char doc[] =
        "Telexir RRD Writer Utility";

void rrd_writer_options_parse(int argc, char **argv) {
    /* The options we understand. */
    static struct argp_option options[] = {
            LOG_OPTIONS
            {"verbose", 'v', 0, OPTION_ARG_OPTIONAL, "Produce verbose output"},
            {"suffix", OPT_VALUE_SUFFIX, "SUFFIX", 0, "Add suffix to counters storage name"},
            {"add-incremented", OPT_PREFIX_INCREMENTED, "PREFIX", 0, "Add incremented counters prefix"},
            {"add-value", OPT_PREFIX_VALUE, "PREFIX", 0, "Add value counters prefix"},
            {"add-history", OPT_PREFIX_HISTORY, "PREFIX", 0, "Add history counters prefix"},
            {0, 0, 0, 0, 0, 0}
    };
    int error_arg_number = -1;
    static struct argp argp = {options, option_parse, args_doc, doc};
    argp_parse(&argp, argc, argv, 0, &error_arg_number, NULL);
}

