/** compile with -std=gnu99 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <argp.h>
#include "armpmu_lib.h"

/* Simple loop body to keep things interested. Make sure it gets inlined. */
static inline int
loop(int* __restrict__ a, int* __restrict__ b, int n)
{
        unsigned sum = 0;
        for (int i = 0; i < n; ++i)
                if(a[i] > b[i])
                        sum += a[i] + 5;
        return sum;
}


// arguments

static char doc[] = "Take pfc id from the command line and count the number of events triggered during a loop.";
static char args_doc[] = "";
static struct argp_option options[] = {
        {"event_number", 'e', "EVENT_NUM", 0, "Event number of the event to tally."},
        {"loop_count", 'l', "LOOP_COUNT", 0, "Runs LOOP_COUNT iterations of the tallied loop."},
        { 0 }
};
struct arguments{
        int event_number;
        int loop_count;
};

static error_t parse_opt (int key, char *arg, struct argp_state *state){
        // get input argument from argp_parse, which we know is a pointer to our arguments struct
        struct arguments *arguments = state->input;
        switch(key){
                case 'e':
                        arguments->event_number = strtol(arg, NULL, 0); // infer string end and base
                        break;
                case 'l':
                        arguments->loop_count = strtol(arg, NULL, 0);
                        break;
                default:
                        return ARGP_ERR_UNKNOWN;
        }

        return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc};

int main(int argc, char **argv) {
        // parse arguments
        struct arguments arguments;
        //default values
        arguments.event_number = 0x0008;
        arguments.loop_count = 1000;
        argp_parse(&argp, argc, argv, 0, 0, &arguments);
        printf("%s: arguments.event_number = 0x%04X\n", argv[0], arguments.event_number);
        printf("%s: arguments.loop_count = %d\n", argv[0], arguments.loop_count);
        int event_number = arguments.event_number;
        


        uint32_t time_start = 0;
        uint32_t time_end   = 0;
	uint32_t cnt_start = 0;
	uint32_t cnt_end = 0;

        int *a  = NULL;
        int *b  = NULL;
        int sum = 0;


	

        a = malloc(arguments.loop_count*sizeof(*a));
        b = malloc(arguments.loop_count*sizeof(*b));

        for (int i = 0; i < arguments.loop_count; ++i) {
                a[i] = i+128;
                b[i] = i+64;
        }

        printf("%s: beginning loop\n", argv[0]);
        time_start = rdtsc32();
        sum = loop(a, b, arguments.loop_count);
        time_end   = rdtsc32();
        printf("%s: done. sum = %d; time delta = %u\n", argv[0], sum, time_end - time_start);

        printf("%s: beginning loop\n", argv[0]);
	enable_pmu(event_number);
        cnt_start = read_pmu();
        sum = loop(a, b, arguments.loop_count);
        cnt_end   = read_pmu();
	disable_pmu(event_number);
        printf("%s: done. sum = %d; event 0x%03x delta = %u\n", argv[0], sum, event_number, cnt_end - cnt_start);

        free(a); free(b);
        return 0;
}
