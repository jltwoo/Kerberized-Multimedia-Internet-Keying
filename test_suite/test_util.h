#ifndef _TEST_UTIL_H_
#define _TEST_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "test.h"

extern int starting_k;
extern int max_k;
extern int ignore_count;
extern char* generalLog_filepath;

typedef struct running_stats {
    int k;
    double coeff_target;
    double mean;
    double variance;
    double std_dev;
    double coeff_variance;
} running_stats_t;

void create_service( char* service_name, int count );

void delete_service( char* service_name, int count);

void create_client_cache( char* client_name);

void create_client_princ( char* client_name);

void delete_client_cache( char* client_name);

void delete_client_princ( char* client_name);

long get_last_value(const char* filepath);

int client_exec( const char* group_name, const char* cache_filepath,
                     const char* timeLog_filepath, int ksm);

void set_default_running_stats( running_stats_t* stats );

void print_results(const char* logfile, int k, long y, double mean,
             double variance, double std_dev, double coeff_variance);

void log_data(const char* filepath, int x_val, running_stats_t* rs );
void log_data_f(const char* filepath, float x_val, running_stats_t* rs );
#endif // _TEST_UTIL_H_
