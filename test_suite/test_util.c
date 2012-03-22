#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "test.h"
#include "test_util.h"

char* generalLog_filepath;

/*
   test1.c - Time it takes for x number of clients to complete
             all requests, sequentially. 
    setup - 
    1 group, x number of clients, repeat k times
*/
void create_service( char* service_name, int count){ 
    char shell_command[BUFSIZ];
    snprintf(shell_command, BUFSIZ,"%s %s %d >> %s",
                             SERVICE_CREATE, 
                             service_name,
                             count,
                             generalLog_filepath,
                             NULL);
    if( setuid(0) < 0 ) printf("Failed to set uid\n");
    system(shell_command);
    seteuid(user_uid);
    setfsuid(user_uid);
}

void delete_service( char* service_name, int count) {
    char shell_command[BUFSIZ];
    snprintf(shell_command, BUFSIZ,"%s %s %d >> %s",
                             SERVICE_REMOVE, 
                             service_name,
                             count,
                             generalLog_filepath,
                             NULL);
    if( setuid(0) < 0 ) printf("Failed to set uid\n");
    system(shell_command);
    seteuid(user_uid);
    setfsuid(user_uid);
}

void create_client_cache( char* client_name) {
    char shell_command[BUFSIZ];
    snprintf(shell_command, BUFSIZ,"%s %s %s >> %s",
                             TICKET_CREATE, 
                             client_name,
                             TEST_PASSWORD, 
                             generalLog_filepath,
                             NULL);
    //execl(KRB_PRINC_CREATE, "sh", "-c", client_name, TEST_PASSWORD);
    system(shell_command);
}

void create_client_princ( char* client_name) {
    char shell_command[BUFSIZ];
    snprintf(shell_command, BUFSIZ,"sudo %s %s %s >> %s",
                             KRB_PRINC_CREATE, 
                             client_name,
                             TEST_PASSWORD, 
                            generalLog_filepath,
                             NULL);
    //execl(KRB_PRINC_CREATE, "sh", "-c", client_name, TEST_PASSWORD);
    if( setuid(0) < 0 ) printf("Failed to set uid\n");
    system(shell_command);
    seteuid(user_uid);
    setfsuid(user_uid);
    /*execl( KRB_PRINC_CREATE, KRB_PRINC_CREATE,
                             client_name,
                             TEST_PASSWORD, 
                             NULL);*/
}

void delete_client_cache( char* client_name) {
    char shell_command[BUFSIZ];
    char cache_filepath[1024];
    snprintf(cache_filepath, 1024, "%s/%s.cache", 
                    RUNTIME_DIR, client_name);
    snprintf(shell_command, BUFSIZ,"kdestroy -c %s >> %s",
                             cache_filepath,
                            generalLog_filepath,
                             NULL);
    system(shell_command);
}

void delete_client_princ( char* client_name) {
    char shell_command[BUFSIZ];
    snprintf(shell_command, BUFSIZ,"%s %s >> %s",
                             KRB_PRINC_REMOVE, 
                             client_name,
                             generalLog_filepath,
                             NULL);
    setuid(0);
    system(shell_command);
    seteuid(user_uid);
    setfsuid(user_uid);
}

long get_last_value(const char* filepath) {
    char shell_command[BUFSIZ];
    char line[128];
    char tmpFile[BUFSIZ];
    
    snprintf(shell_command, BUFSIZ,"tail -n1 %s > %s.tmp",
                             filepath,filepath);
    system(shell_command);
    snprintf(tmpFile, BUFSIZ, "%s.tmp", filepath);

    FILE* fp = fopen(tmpFile, "r" );

    if( fp != NULL ) {
        if( fgets(line, sizeof(line),fp) != NULL ) 
        {
            if( fp ) fclose(fp);
            remove(tmpFile);
            //printf("Reading from logfile%s\n", line);
            return atol(line);
        }
    }  
    if( fp ) fclose(fp);
    remove(tmpFile);
    return LONG_MAX;
}

int client_exec( const char* group_name, const char* cache_filepath,
                     const char* timeLog_filepath, int ksm) 
{
    char shell_command[BUFSIZ];
    if( ksm != 2 ) {
        snprintf(shell_command, BUFSIZ,"%s %s %d %s %s %s %s >> %s ",
                            KSM1_CLIENT_EXE,
                            REALM,
                            KSM1_GCKS_PORT, 
                            KSM1_GCKS_SNAME,
                            group_name,
                            cache_filepath,
                            timeLog_filepath,
                            generalLog_filepath,
                            NULL);
    }
    else {
        snprintf(shell_command, BUFSIZ,"%s %s %s %s %s >> %s ",
                            KSM2_CLIENT_EXE,
                            REALM,
                            group_name,
                            cache_filepath,
                            timeLog_filepath,
                            generalLog_filepath,
                            NULL);
    }

    setfsuid(0);
    return system(shell_command);
}

void set_default_running_stats( running_stats_t *stats ) {
    stats->k = starting_k;
    stats->coeff_target = 0.05;
    stats->mean = 0.0;
    stats->variance = 0.0;
    stats->std_dev = 0.0;
    stats->coeff_variance = 0.0;
}

void print_results(const char* filepath, int n, long y, double mean,
             double variance, double std_dev, double coeff_variance)
{
    FILE* fp = fopen(filepath, "a");
    printf(
    "n[%d] y[%ld] mean[%4.3f] var[%4.3f] std[%4.3f] coeff[%4.3f]\n",
        n, y, mean, variance, std_dev, coeff_variance);
    fprintf( fp,
    "n[%d] y[%ld] mean[%4.3f] var[%4.3f] std[%4.3f] coeff[%4.3f]\n",
        n, y, mean, variance, std_dev, coeff_variance);
    if( fp ) fclose(fp);
}

void log_data(const char* filepath, int x_val, running_stats_t* rs ) {
    FILE* fp = fopen(filepath, "a");
    
    fprintf(fp, "%d %4.3f %4.3f %4.3f %4.3f %4.3f %d \n", 
         x_val, rs->mean, rs->variance, rs->std_dev,
         2*rs->std_dev, rs->coeff_variance, rs->k); 

    if( fp ) fclose( fp );
}
void log_data_f(const char* filepath, float x_val, running_stats_t* rs ) {
    FILE* fp = fopen(filepath, "a");
    
    fprintf(fp, "%4.2f %4.3f %4.3f %4.3f %4.3f %4.3f %d \n", 
         x_val, rs->mean, rs->variance, rs->std_dev,
         2*rs->std_dev, rs->coeff_variance, rs->k); 

    if( fp ) fclose( fp );
}

void merge_data( const char* dest, const char* ksm1_filepath,
                     const char* ksm2_filepath ) 
{
    char shell_command[BUFSIZ];
        snprintf(shell_command, BUFSIZ,"paste %s %s > %s",
                            ksm1_filepath,
                            ksm2_filepath,
                            dest, 
                            NULL);
    system(shell_command);
}
