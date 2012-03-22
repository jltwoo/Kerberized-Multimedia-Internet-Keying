#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include "test.h"
#include "test_util.h"

#define TEST2_CASE_B 1
int test_index = -1;

int group_count_array[8] =       { 1,  5,  10,  15,  20, 30, 40, 50};

#ifdef TEST2_CASE_B
int test_array[1] = { 10 };
#else
int test_array[1] = { 1 };
#endif

#ifdef HARD_UPPER_LIMIT
int upper_limit = INT_MAX;
int upper_limit_ksm1[7] = {40, 65, 150, 220, 270, 470, 550};
int upper_limit_ksm2[7] = {40, 60,  80, 160, 220, 320, 400};
#endif

int group_count = 5;
int assigned_group_count = 1;
int starting_k;
int max_k;
int ignore_count;

char timeLog_filepath[1024];
FILE* timeLog_file;

// Results Logging
char resultsLogRAW_filepath[1024];
FILE* resultsLogRAW_file;

char test2_dat_filepath[1024];
char test2_dat_ksm1_filepath[1024];
char test2_dat_ksm2_filepath[1024];
FILE* test2_dat_file;


/*
   test2.c - Time it takes for x number of clients to complete
             all requests, sequentially. 
    setup - 
    1 group, x number of clients, repeat k times
*/

long test2(const char* timeLog_filepath, int num_of_clients, int ksm )
{
    char shell_command[BUFSIZ];
    char cache_filepath[1024];
    char client_name[1024];
    char group_name[1024];
    int i,j,k;
    long total_time = 0.0L;
    FILE* timeLog_file = NULL;
    char* base_client_name = 
              (ksm==2)? KSM2_BASE_CLIENT_NAME:KSM1_BASE_CLIENT_NAME;

    // Create client principals and its cache for each client
    for( i = 0; i < num_of_clients; i++ ) {
        //printf("\rCreating client principal and ticket cache %d of %d",
        //     i+1, num_of_clients);
        snprintf(client_name, 1024, "%s%d", base_client_name, i);
        create_client_princ( client_name );
        create_client_cache( client_name );
    }
    //printf("\n");

    assigned_group_count = group_count; //assigned to all existing groups

    // Execute all client request sequentially
    for( i = 0; i < num_of_clients; i++ ) {
        snprintf(client_name, 1024, "%s%d", base_client_name, i);
        snprintf(cache_filepath, 1024, "%s/%s%d.cache", 
                        RUNTIME_DIR, base_client_name, i);
        
        for( j=1; j <= assigned_group_count; j++ ) {
             memset(group_name, 0, 1024);
             snprintf(group_name, 1024, "group%d", j);
             
             if( client_exec( group_name, cache_filepath,
                         timeLog_filepath, ksm) < 0 )
             { 
                 for( k = 0; k < num_of_clients; k++ ) {
                     snprintf(client_name, 1024, "%s%d", base_client_name, k);
                     delete_client_cache( client_name );
                     delete_client_princ( client_name );
                 }
                 delete_service("group",group_count); 
                 printf("Client failed\n");
                 exit(1);
             }
    
             long last_val = get_last_value( timeLog_filepath );
    
             if( last_val > 100 || last_val < 0) {
                 snprintf(shell_command, BUFSIZ,"%s %s",
                             DEL_LAST_LINE, 
                             timeLog_filepath,
                             NULL);
                 // Reject data that's out of range
                 system(shell_command);
                 if (ksm == 2) {
                     delete_client_cache( client_name );
                     create_client_cache( client_name );                    
                 }
    //               printf("last_val too high, reject%ld\n", last_val);
                 j--;
             }   
        }
    }
   
    timeLog_file = fopen(timeLog_filepath, "r");
    
    if( timeLog_file != NULL ) {
        char line[128];
        for(i = 0; i < num_of_clients; i++ ) {
            for( j = 0; j <assigned_group_count; j++ ) {

                if( fgets(line, sizeof(line), 
                       timeLog_file) != NULL ) 
                {
                    total_time += atol( line );
                }
            }
        }
        fclose(timeLog_file);
    }
    else {
        perror( timeLog_filepath );
    }

    for( i = 0; i < num_of_clients; i++ ) {
        snprintf(client_name, 1024, "%s%d", base_client_name, i);
        delete_client_cache( client_name );
        delete_client_princ( client_name );
    }
    //printf("\n"); 
    return total_time;
}

void run_test2( running_stats_t* rs, int num_of_clients, int ksm ) {
    // Indexing
    int i,j,n;
    long y = 0.0L;  // Single time reading for one iteration of test
    long prev_y = -1;
    int in_loop = 0;

    //upper_limit = (ksm == 1 )?
    //                 upper_limit_ksm1[test_index]:
    //                 upper_limit_ksm2[test_index];
    //printf("upper_limit: %d\n",upper_limit);
    for( n = j = 1; j <= max_k; j++, n++ ) {
        // This does one iteration of test2
        in_loop =0;
        do {  
            // For each iteration, we clear the log
            timeLog_file = fopen( timeLog_filepath, "w" );
            fclose(timeLog_file);
            y = test2( timeLog_filepath, num_of_clients, ksm );
            if( prev_y == -1) {
                prev_y = y;
            }
            if(in_loop==0) {
                in_loop = 1;
            }
            else {
//                printf("y > prev_y\n");
            }   
        }
        while( y > prev_y * 1.1 );
        
        prev_y = y;
        double prev_mean = rs->mean;

        double prev_variance, prev_std_dev, prev_coeff_var;
        prev_variance = rs->variance;
        prev_std_dev = rs->std_dev;
        prev_coeff_var = rs->coeff_variance;
        
        rs->mean = (prev_mean*(n-1) + y) / (n) ;

        if( j >  2 ) {
            rs->variance = ( 
                    (n - 2)*rs->variance + (y-rs->mean) * (y-prev_mean) )
                             / ( n -1 );

            rs->std_dev = sqrt( rs->variance );
            rs->coeff_variance = rs->std_dev / rs->mean;

            if( rs->coeff_variance >= prev_coeff_var && j > starting_k) {
                rs->variance = prev_variance;
                rs->std_dev = prev_std_dev;
                rs->mean = prev_mean;
                rs->coeff_variance = prev_coeff_var;
                n--;
            }
        }
        
        print_results(resultsLogRAW_filepath,
            n, y,  rs->mean, rs->variance, rs->std_dev, rs->coeff_variance);

        if( j >= starting_k  && rs->coeff_variance < rs->coeff_target ) {
            rs->k = n;
            break;
        }
        // Calculate the total time taken for all clients to complete
    }
}

void setup_logs() {
    FILE* fp = NULL;
    snprintf(resultsLogRAW_filepath, 1024, "%s/test2.dat.log",RESULTS_DIR);
    snprintf(timeLog_filepath, 1024, "%s/test2.dat.tmp",RESULTS_DIR);

    // Clear output files
    fp = fopen( resultsLogRAW_filepath, "w" );
    if(fp) fclose(fp);
    
    fp = fopen( generalLog_filepath, "w" );
    if(fp) fclose(fp);

    snprintf(test2_dat_filepath, 1024, "%s/test2.dat",RESULTS_DIR);
    snprintf(test2_dat_ksm1_filepath, 1024, 
                                        "%s/test2.dat.ksm1",RESULTS_DIR);
    snprintf(test2_dat_ksm2_filepath, 1024, 
                                        "%s/test2.dat.ksm2",RESULTS_DIR);

    fp = fopen( test2_dat_filepath, "w");
    if(fp) fclose(fp);

    fp = fopen( test2_dat_ksm1_filepath, "w");
    if(fp) fclose(fp);

    fp = fopen( test2_dat_ksm2_filepath, "w");
    if(fp) fclose(fp);
}

int main(int argc, char* argv[]) 
{
    // Simulation configuration
    int num_of_clients = 1;
    
    // Indexing
    int i,j,n,l;

    // Temporary Logging
    generalLog_filepath = TEST2_GENERAL_LOG;

    starting_k = 5;
    max_k = 20;
    ignore_count = 3;
    
    if( getenv("SUDO_UID") ) {
        user_uid = atoi(getenv("SUDO_UID"));
    }
    else {
        printf("You need sudo to run the test\n");
        exit(0);
    }
#ifdef HARD_UPPER_LIMIT
    if( argc > 1 ) {
        upper_limit = atoi(argv[1]);
    }
#endif

    if( argc > 2 ) {
        num_of_clients = atoi(argv[2]);
    }

    seteuid(user_uid);  setfsuid(user_uid);

    setup_logs();
    // Statistical variables
    running_stats_t rs;
    //set_default_running_stats( &rs );

    for( test_index = 0; test_index < sizeof(test_array)/sizeof(int);
         test_index++) {
        num_of_clients = ( argc > 2 )? num_of_clients:test_array[test_index];

         for( l=1; l <= sizeof(group_count_array)/sizeof(int); l++) {
             group_count = group_count_array[l-1];
             create_service("group",group_count);
             set_default_running_stats( &rs );
                  
             printf("===================Pre-test Runs=====================\n");
             for( j = 0; j < ignore_count; j++ ) { // ignore first few iterations
                 timeLog_file = fopen( timeLog_filepath, "w" );
                 fclose(timeLog_file);
                 test2( timeLog_filepath, num_of_clients, 1);
             }
             
             printf("===================Starting KSM1=====================\n");
             resultsLogRAW_file = fopen( resultsLogRAW_filepath, "a" );
             fprintf(resultsLogRAW_file,
                   "===================Starting KSM1=====================\n");
             if(resultsLogRAW_file) fclose(resultsLogRAW_file);
             run_test2(&rs, num_of_clients, 1);
             log_data( test2_dat_ksm1_filepath, group_count, &rs );


             set_default_running_stats( &rs );
             printf("===================Pre-test Runs=====================\n");
             for( j = 0; j < ignore_count; j++ ) { // ignore first few iterations
                 timeLog_file = fopen( timeLog_filepath, "w" );
                 fclose(timeLog_file);
                 test2( timeLog_filepath, num_of_clients, 2);
             }
             
             printf("===================Starting KSM2=====================\n");
             resultsLogRAW_file = fopen( resultsLogRAW_filepath, "a" );
             fprintf(resultsLogRAW_file,
                     "===================Starting KSM2=====================\n");
             if(resultsLogRAW_file) fclose(resultsLogRAW_file);
         
             run_test2(&rs, num_of_clients, 2);
             log_data( test2_dat_ksm2_filepath, group_count, &rs );

             delete_service("group",group_count);
         }

    
        if( argc > 2 ) break;
    }        


    merge_data(test2_dat_filepath, test2_dat_ksm1_filepath,
                                   test2_dat_ksm2_filepath);
    remove(test2_dat_ksm1_filepath);
    remove(test2_dat_ksm2_filepath);
    remove(timeLog_filepath);

    return 0;
}
