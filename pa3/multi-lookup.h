
/******************************************************************************
 * Copyright (C) 2018
 * Christopher Morroni
 * University of Colorado, Boulder
 *
 * Redistribution, modification, or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of operating
 * systems. Christopher Morroni and the University of Colorado are not liable
 * for any misuse of this material.
 *
 *****************************************************************************/
/**
 * @file multi-lookup.h
 * @brief Resolve a set of hostnames to IP addresses
 * 
 * Definitions and declarations for functions to resolve hostnames to
 * IP addresses
 * 
 * @author Christopher Morroni
 * @date 2018-03-11
 */

#ifndef __MULTI_LOOKUP_H__
#define __MULTI_LOOKUP_H__

#define MIN_NUM_PARAMS (6)
#define PARAM_NUM_REQUESTERS (1)
#define PARAM_NUM_RESOLVERS (2)
#define PARAM_NUM_REQUESTER_LOG (3)
#define PARAM_NUM_RESOLVER_LOG (4)
#define PARAM_NUM_DATA_FILE (5)

#define NAME_SERVICED_LOG ("serviced.txt")

#define USAGE_DECLARATION ("\nNAME\n    multi-lookup resolve a set of hostnames to IP addresses\n\nSYNOPSIS\n    multi-lookup <# requesters> <# resolvers> <requester log> <resolver log> <data file> [<data file> ...]\n\nDESCRIPTION\n    The file names specified by <data file> are passed to the pool of requester threads\n    which place information into a shared data area. Resolver threads read the shared\n    data area and find the corresponding IP address.\n\n    <# requesters> number of requester threads to place into the thread pool.\n    <# resolvers> number of resolver threads to place into the thread pool.\n    <requester log> name of the file into which all the requester status information is written.\n    <resolver log> name of the file into which all the resolver status information is written.\n    <data file> file(s) that are to be processed. Each file contains a list of host names, one per line,\n                that are to be resolved.\n")

typedef struct
{
  FILE * ptr_file;
  char ** name;
  int name_len;
  pthread_mutex_t * ptr_mutex;
} file_t;

typedef struct
{
  file_t * requester_log;
  file_t * resolver_log;
  file_t ** input_files;
  int num_input_files;
  int num_requester;
  int num_resolver;
} lookup_params_t;

typedef struct
{
  lookup_params_t * ptr_lookup_params;
  char ** ptr_domains;
  int * domains_len;
  int num_domains;
  int rr_next_file;
  int * file_done_f;
  pthread_mutex_t * ptr_mutex;
  pthread_mutex_t * ptr_printf_mutex;
} lookup_info_t;

/**
 * @brief Process inputs to main
 *
 * Parses the arguments to main and places into a structure.
 *
 * @param argc The number of arguments
 * @param argv An array of pointers to the arguments
 * @param ptr_lookup_params A pointer to the uninitialized structure
 *
 * @return 0 if successful, -1 otherwise
 */
int process_inputs(int argc, char ** argv, lookup_params_t ** ptr_lookup_params);

/**
 * @brief Free the parameter structure from the heap
 *
 * Frees all pieces of the parameter structure from the heap, and 
 * closes files held within.
 * All portions must be initialized of the program will crash.
 *
 * @param ptr_lookup_params A pointer to the parameter structure
 */
void free_lookup_params(lookup_params_t * ptr_lookup_params);

/**
 * @brief Function for requester threads
 *
 * Reads hostnames from the input files and places them in the shared data.
 * Prints to the requester log file when complete.
 *
 * @param arg A pointer to the structure with all the information for the program.
 */
void * requester(void * arg);

/**
 * @brief Function for resolver threads
 *
 * Reads hostnames from the shared data, resolves to IP addresses, and
 * prints out to the resolver log file.
 *
 * @param arg A pointer to the structure with all the information for the program.
 */
void * resolver(void * arg);

/**
 * @brief Main function for multi-lookup
 *
 * Sets up structures, creates threads, cleans up memory, and
 * prints total run-time to stdout.
 *
 * @param argc The number of arguments
 * @param argv An array of pointers to the arguments
 *
 * @return 0 if successful, -1 otherwise
 */
int main(int argc, char ** argv);

#endif /* __MULTI_LOOKUP_H__ */
