
#ifndef _TEST_SUITE_H_
#define _TEST_SUITE_H_
#include <limits.h>
// Project structure
#define WORKSPACE_DIR "/home/jeffrey/multicast"
#define TEST_SUITE_DIR WORKSPACE_DIR "/test_suite"
#define KSMTOOLS_DIR WORKSPACE_DIR "/kmtools"
#define SCRIPTS_DIR WORKSPACE_DIR "/scripts"
#define KRB5_ADMIN_DIR SCRIPTS_DIR "/krb5_admin"
#define RUNTIME_DIR WORKSPACE_DIR "/bin"
#define RESULTS_DIR WORKSPACE_DIR "/results"

// Executables
#define KSM1_CLIENT_EXE KSMTOOLS_DIR "/sclient"
#define KSM2_CLIENT_EXE KSMTOOLS_DIR "/sclient2"

// Script files
#define KRB_PRINC_CREATE KRB5_ADMIN_DIR "/krb_princ_create.sh"
#define KRB_PRINC_REMOVE KRB5_ADMIN_DIR "/krb_princ_remove.sh"

#define KEYTAB_CREATE KRB5_ADMIN_DIR "/keytab_create.sh"
#define KEYTAB_REMOVE KRB5_ADMIN_DIR "/keytab_remove.sh"

#define TICKET_CREATE SCRIPTS_DIR "/basic/ticket_create.sh"

#define SERVICE_CREATE SCRIPTS_DIR "/basic/service_create.sh"
#define SERVICE_REMOVE SCRIPTS_DIR "/basic/service_remove.sh"


// Setup configuration
#define TEST_PASSWORD "test"

#define KSM1_GCKS_PORT 7777
#define KSM1_BASE_CLIENT_NAME "ksm1_test_client"
#define KSM1_GCKS_SNAME "ksm1_gcks1"

#define KSM2_GCKS_PORT 8888
#define KSM2_BASE_CLIENT_NAME "ksm2_test_client"
#define KSM2_GCKS_SNAME "ksm2_gcks1"

#define REALM "LOCALHOST"

// Logging Information
#define TEST1_GENERAL_LOG RUNTIME_DIR "/general_test1.log"
#define TEST2_GENERAL_LOG RUNTIME_DIR "/general_test2.log"
#define TEST3_GENERAL_LOG RUNTIME_DIR "/general_test3.log"
#define TEST4_GENERAL_LOG RUNTIME_DIR "/general_test4.log"

// Tools
#define DEL_LAST_LINE TEST_SUITE_DIR "/del_last_line.sh"
uid_t user_uid;

#endif // _TEST_SUITE_H_
