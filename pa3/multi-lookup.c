
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
 * @file multi-lookup.c
 * @brief Resolve a set of hostnames to IP addresses
 * 
 * Implementations for functions to resolve hostnames to IP addresses
 * 
 * @author Christopher Morroni
 * @date 2018-03-11
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include "multi-lookup.h"
#include "util.h"

int process_inputs(int argc, char ** argv, lookup_params_t ** ptr_lookup_params)
{
  file_t * ptr_requester_log;
  file_t * ptr_resolver_log;
  file_t * ptr_data_file;
  file_t ** input_files = NULL;
  int num_input_files = 0;
  FILE * temp;
  int temp_int;
  pthread_mutex_t * ptr_temp_mutex;

  // create main struct for parameters
  if( (*ptr_lookup_params = (lookup_params_t *)calloc(1, sizeof(lookup_params_t))) == NULL )
  {
    return -1;
  }

  /*
   * Number of requester threads
   */
  // convert from string
  if( sscanf(argv[PARAM_NUM_REQUESTERS], "%d", &temp_int) != 1 )
  {
    printf("<# requester> should be an integer\n");
    free((void *)*ptr_lookup_params);
    return -1;
  }

  // verify value
  if( temp_int < 1 )
  {
    printf("<# requester> should be more than 0, got %d\n", temp_int);
    free((void *)*ptr_lookup_params);
    return -1;
  }

  // store
  (*ptr_lookup_params)->num_requester = temp_int;

  /*
   * Number of resolver threads
   */
  // convert from string
  if( sscanf(argv[PARAM_NUM_RESOLVERS], "%d", &temp_int) != 1 )
  {
    printf("<# resolver> should be an integer\n");
    free((void *)*ptr_lookup_params);
    return -1;
  }

  // verify value
  if( temp_int < 1 )
  {
    printf("<# resolver> should be more than 0, got %d\n", temp_int);
    free((void *)*ptr_lookup_params);
    return -1;
  }

  // store
  (*ptr_lookup_params)->num_resolver = temp_int;
  
  /*
   * Requester log file
   */
  // make sure file is writable
  if( (temp = fopen(argv[PARAM_NUM_REQUESTER_LOG], "w")) == NULL )
  {
    printf("%s does not exist or does not grant write permissions\n", argv[PARAM_NUM_REQUESTER_LOG]);
    free((void *)*ptr_lookup_params);
    return -1;
  }

  // create struct for file info
  if( (ptr_requester_log = (file_t *)calloc(1, sizeof(file_t))) == NULL )
  {
    free((void *)*ptr_lookup_params);
    fclose(temp);
    return -1;
  }
  ptr_requester_log->ptr_file = temp;
  ptr_requester_log->name = &argv[PARAM_NUM_REQUESTER_LOG];
  ptr_requester_log->name_len = strlen(*ptr_requester_log->name);

  // create mutex for file
  if( (ptr_temp_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t))) == NULL )
  {
    free((void *)*ptr_lookup_params);
    fclose(ptr_requester_log->ptr_file);
    free((void *)ptr_requester_log);
    return -1;
  }
  pthread_mutex_init(ptr_temp_mutex, NULL);
  ptr_requester_log->ptr_mutex = ptr_temp_mutex;

  //store
  (*ptr_lookup_params)->requester_log = ptr_requester_log;
  
  /*
   * Resolver log file
   */
  // make sure file is writable
  if( (temp = fopen(argv[PARAM_NUM_RESOLVER_LOG], "w+")) == NULL )
  {
    printf("%s does not exist or does not grant write permissions\n", argv[PARAM_NUM_RESOLVER_LOG]);
    free((void *)*ptr_lookup_params);
    fclose(ptr_requester_log->ptr_file);
    free((void *)ptr_requester_log);
    return -1;
  }

  // create struct for file info
  if( (ptr_resolver_log = (file_t *)calloc(1, sizeof(file_t))) == NULL )
  {
    free((void *)*ptr_lookup_params);
    fclose(ptr_requester_log->ptr_file);
    free((void *)ptr_requester_log);
    fclose(temp);
    return -1;
  }
  ptr_resolver_log->ptr_file = temp;
  ptr_resolver_log->name = &argv[PARAM_NUM_RESOLVER_LOG];
  ptr_resolver_log->name_len = strlen(*ptr_resolver_log->name);

  // create mutex for file
  if( (ptr_temp_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t))) == NULL )
  {
    free((void *)*ptr_lookup_params);
    fclose(ptr_requester_log->ptr_file);
    free((void *)ptr_requester_log);
    fclose(ptr_resolver_log->ptr_file);
    free((void *)ptr_resolver_log);
    return -1;
  }
  pthread_mutex_init(ptr_temp_mutex, NULL);
  ptr_resolver_log->ptr_mutex = ptr_temp_mutex;

  // store
  (*ptr_lookup_params)->resolver_log = ptr_resolver_log;

  /*
   * Input files
   */

  // allocate array for files
  if( (input_files = (file_t **)malloc(sizeof(file_t *) * (argc - PARAM_NUM_DATA_FILE))) == NULL )
  {
    free((void *)*ptr_lookup_params);
    fclose(ptr_requester_log->ptr_file);
    free((void *)ptr_requester_log);
    fclose(ptr_resolver_log->ptr_file);
    free((void *)ptr_resolver_log);
    return -1;
  }

  // for each input file parameter
  (*ptr_lookup_params)->num_input_files = 0;
  for(int i = PARAM_NUM_DATA_FILE; i < argc; i++)
  {
    // make sure file exists and is readable
    if( (temp = fopen(argv[i], "r")) == NULL )
    {
      printf("%s does not exist or does not grant read access\n", argv[i]);
      (*ptr_lookup_params)->input_files = input_files;
      (*ptr_lookup_params)->num_input_files = num_input_files;
      free_lookup_params(*ptr_lookup_params);
      return -1;
    }

    // create struct for file info
    if( (ptr_data_file = (file_t *)malloc(sizeof(file_t))) == NULL )
    {
      (*ptr_lookup_params)->input_files = input_files;
      (*ptr_lookup_params)->num_input_files = num_input_files;
      free_lookup_params(*ptr_lookup_params);
      return -1;
    }
    ptr_data_file->ptr_file = temp;
    ptr_data_file->name = &argv[i];
    ptr_data_file->name_len = strlen(*ptr_data_file->name);

    // create mutex for file
    if( (ptr_temp_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t))) == NULL )
    {
      (*ptr_lookup_params)->input_files = input_files;
      (*ptr_lookup_params)->num_input_files = num_input_files;
      free_lookup_params(*ptr_lookup_params);
      return -1;
    }
    pthread_mutex_init(ptr_temp_mutex, NULL);
    ptr_data_file->ptr_mutex = ptr_temp_mutex;

    // store
    input_files[num_input_files++] = ptr_data_file;
  }
  
  // store in main struct
  (*ptr_lookup_params)->input_files = input_files;
  (*ptr_lookup_params)->num_input_files = num_input_files;

  return 0;
}

void free_lookup_params(lookup_params_t * ptr_lookup_params)
{
  fclose(ptr_lookup_params->requester_log->ptr_file);
  free((void *)ptr_lookup_params->requester_log->ptr_mutex);
  free((void *)ptr_lookup_params->requester_log);

  fclose(ptr_lookup_params->resolver_log->ptr_file);
  free((void *)ptr_lookup_params->resolver_log->ptr_mutex);
  free((void *)ptr_lookup_params->resolver_log);

  for(int i = 0; i < ptr_lookup_params->num_input_files; i++)
  {
    fclose(ptr_lookup_params->input_files[i]->ptr_file);
    free((void *)(ptr_lookup_params->input_files[i]->ptr_mutex));
    free((void *)(ptr_lookup_params->input_files[i]));
  }
  free((void *)ptr_lookup_params->input_files);
  free((void *)ptr_lookup_params);
}

void * requester(void * arg)
{
  lookup_info_t * ptr_lookup_info = (lookup_info_t *)arg;
  lookup_params_t * ptr_lookup_params = ptr_lookup_info->ptr_lookup_params;
  int curr_file_idx = 0;
  file_t * ptr_curr_file;
  int num_files = 0;

  char * str_in;
  char * temp_str_in;
  int str_in_len;
  if( (str_in = (char *)malloc(1025)) == NULL )
  {
    pthread_mutex_lock(ptr_lookup_info->ptr_printf_mutex);
    printf("Unable to malloc\n");
    pthread_mutex_unlock(ptr_lookup_info->ptr_printf_mutex);
    exit(-1);
  }

  // loop over input files
  while(1)
  {
    // get next file index that has not been completed
    pthread_mutex_lock(ptr_lookup_info->ptr_mutex);
    curr_file_idx = ptr_lookup_info->rr_next_file;
    int i;
    for(i = curr_file_idx;
        i != (curr_file_idx + ptr_lookup_params->num_input_files - 1) % ptr_lookup_params->num_input_files &&
          ptr_lookup_info->file_done_f[i];
        i = (i + 1) % ptr_lookup_params->num_input_files);
    curr_file_idx = i;
    ptr_lookup_info->rr_next_file = (curr_file_idx + 1) % ptr_lookup_params->num_input_files;
    if(ptr_lookup_info->file_done_f[curr_file_idx])
    {
      pthread_mutex_unlock(ptr_lookup_info->ptr_mutex);
      break;
    }
    pthread_mutex_unlock(ptr_lookup_info->ptr_mutex);

    ptr_curr_file = ptr_lookup_params->input_files[curr_file_idx];

    // loop through file
    while(1)
    {
      // read next line
      pthread_mutex_lock(ptr_curr_file->ptr_mutex);
      if(ptr_curr_file->ptr_file == NULL)
      {
        pthread_mutex_unlock(ptr_curr_file->ptr_mutex);
        break;
      }
      if( fscanf(ptr_curr_file->ptr_file, "%1024s", str_in) == EOF )
      {
        pthread_mutex_unlock(ptr_curr_file->ptr_mutex);
        break;
      }
      pthread_mutex_unlock(ptr_curr_file->ptr_mutex);

      // shorten string
      str_in_len = strlen(str_in) + 1;
      if( (temp_str_in = (char *)malloc(str_in_len)) == NULL )
      {
        break;
      }
      memcpy(temp_str_in, str_in, str_in_len);

      // write line to array
      pthread_mutex_lock(ptr_lookup_info->ptr_mutex);
      if( (ptr_lookup_info->ptr_domains = (char **)realloc(ptr_lookup_info->ptr_domains, (ptr_lookup_info->num_domains + 1) * sizeof(char *))) == NULL )
      {
        pthread_mutex_unlock(ptr_lookup_info->ptr_mutex);
        break;
      }
      if( (ptr_lookup_info->domains_len = (int *)realloc(ptr_lookup_info->domains_len, (ptr_lookup_info->num_domains + 1) * sizeof(int))) == NULL )
      {
        pthread_mutex_unlock(ptr_lookup_info->ptr_mutex);
        break;
      }
      ptr_lookup_info->ptr_domains[ptr_lookup_info->num_domains] = temp_str_in;
      ptr_lookup_info->domains_len[ptr_lookup_info->num_domains++] = str_in_len;
      pthread_mutex_unlock(ptr_lookup_info->ptr_mutex);
    }

    // mark file as done and close
    pthread_mutex_lock(ptr_lookup_info->ptr_mutex);
    ptr_lookup_info->file_done_f[curr_file_idx] = 1;
    pthread_mutex_unlock(ptr_lookup_info->ptr_mutex);

    num_files++;
  }

  // print to serviced file
  ptr_curr_file = ptr_lookup_params->requester_log;
  pthread_mutex_lock(ptr_curr_file->ptr_mutex);
  fprintf(ptr_curr_file->ptr_file, "Thread %ld serviced %d files.\n", syscall(SYS_gettid), num_files);
  pthread_mutex_unlock(ptr_curr_file->ptr_mutex);

  free((void *)str_in);

  pthread_exit(0);
}

void * resolver(void * arg)
{
  lookup_info_t * ptr_lookup_info = (lookup_info_t *)arg;
  lookup_params_t * ptr_lookup_params = ptr_lookup_info->ptr_lookup_params;
  file_t * ptr_resolver_log = ptr_lookup_params->resolver_log;
  int domain_idx;
  char * ptr_domain_str;
  int domain_len;
  char ip_str[INET6_ADDRSTRLEN];
  int dns_ret;

  while(1)
  {
    pthread_mutex_lock(ptr_lookup_info->ptr_mutex);

    // check if there are any domains to read or if all files are done
    if(ptr_lookup_info->num_domains <= 0)
    {
      int i;
      for(i = 0; i < ptr_lookup_params->num_input_files && ptr_lookup_info->file_done_f[i]; i++);
      if(i == ptr_lookup_params->num_input_files)
      {
        pthread_mutex_unlock(ptr_lookup_info->ptr_mutex);
        break;
      }
      pthread_mutex_unlock(ptr_lookup_info->ptr_mutex);
      continue;
    }

    // read next domain
    domain_idx = --ptr_lookup_info->num_domains;
    domain_len = ptr_lookup_info->domains_len[domain_idx];
    if( (ptr_domain_str = (char *)malloc(domain_len)) == NULL )
    {
      
    }
    strcpy(ptr_domain_str, ptr_lookup_info->ptr_domains[domain_idx]);
    free((void *)ptr_lookup_info->ptr_domains[domain_idx]);
    if(ptr_lookup_info->num_domains > 0)
    {
      if( (ptr_lookup_info->ptr_domains = (char **)realloc(ptr_lookup_info->ptr_domains, ptr_lookup_info->num_domains * sizeof(char **))) == NULL )
      {
      }
      if( (ptr_lookup_info->domains_len = realloc(ptr_lookup_info->domains_len, ptr_lookup_info->num_domains * sizeof(int))) == NULL )
      {
      }
    }
    else
    {
      free((void *)ptr_lookup_info->ptr_domains);
      free((void *)ptr_lookup_info->domains_len);
      ptr_lookup_info->ptr_domains = NULL;
      ptr_lookup_info->domains_len = NULL;
    }

    pthread_mutex_unlock(ptr_lookup_info->ptr_mutex);

    // get IP
    dns_ret = dnslookup(ptr_domain_str, ip_str, INET6_ADDRSTRLEN);

    // write to file
    pthread_mutex_lock(ptr_resolver_log->ptr_mutex);
    if(dns_ret == UTIL_SUCCESS)
    {
      fprintf(ptr_resolver_log->ptr_file, "%s,%s\n", ptr_domain_str, ip_str);
    }
    else
    {
      fprintf(ptr_resolver_log->ptr_file, "%s,\n", ptr_domain_str);
    }
    pthread_mutex_unlock(ptr_resolver_log->ptr_mutex);

    free((void *)ptr_domain_str);
  }

  pthread_exit(0);
}

int main(int argc, char ** argv)
{
  // get start time
  struct timeval start_time, end_time, elapsed_time;
  struct timezone time_zone;
  time_zone.tz_minuteswest = 0;
  time_zone.tz_dsttime = 0;
  gettimeofday(&start_time, &time_zone);

  lookup_params_t * ptr_lookup_params;
  lookup_info_t * ptr_lookup_info;
  int * file_done_f;
  pthread_mutex_t * ptr_temp_mutex;

  // process input parameters
  if(argc < MIN_NUM_PARAMS)
  {
    printf(USAGE_DECLARATION);
    return -1;
  }
  if(process_inputs(argc, argv, &ptr_lookup_params) != 0) return -1;

  /*
   * Main initialization
   */
  // create main struct
  if( (ptr_lookup_info = (lookup_info_t *)malloc(sizeof(lookup_info_t))) == NULL )
  {
    printf("Unable to malloc\n");
    free_lookup_params(ptr_lookup_params);
    return -1;
  }
  ptr_lookup_info->ptr_lookup_params = ptr_lookup_params;
  ptr_lookup_info->ptr_domains = NULL;
  ptr_lookup_info->domains_len = NULL;
  ptr_lookup_info->num_domains = 0;
  ptr_lookup_info->rr_next_file = 0;

  // create array for file done flags
  if( (file_done_f = (int *)malloc(sizeof(int) * ptr_lookup_params->num_input_files)) == NULL )
  {
    printf("Unable to malloc\n");
    free_lookup_params(ptr_lookup_params);
    free((void *)ptr_lookup_info);
    return -1;
  }
  memset(file_done_f, 0, sizeof(int) * ptr_lookup_params->num_input_files);
  ptr_lookup_info->file_done_f = file_done_f;

  // create mutex for struct
  if( (ptr_temp_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t))) == NULL )
  {
    printf("Unable to malloc\n");
    free_lookup_params(ptr_lookup_params);
    free((void *)ptr_lookup_info);
    free((void *)file_done_f);
    return -1;
  }
  pthread_mutex_init(ptr_temp_mutex, NULL);
  ptr_lookup_info->ptr_mutex = ptr_temp_mutex;

  // create printf mutex
  if( (ptr_temp_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t))) == NULL )
  {
    printf("Unable to malloc\n");
    free_lookup_params(ptr_lookup_params);
    free((void *)ptr_lookup_info);
    free((void *)file_done_f);
    return -1;
  }
  pthread_mutex_init(ptr_temp_mutex, NULL);
  ptr_lookup_info->ptr_printf_mutex = ptr_temp_mutex;

  // create array for threads
  int num_threads = ptr_lookup_params->num_requester + ptr_lookup_params->num_resolver;
  pthread_t * thread_array;
  if( (thread_array = (pthread_t *)malloc(sizeof(pthread_t) * num_threads)) == NULL )
  {
    printf("Unable to malloc\n");
    free_lookup_params(ptr_lookup_params);
    free((void *)ptr_lookup_info);
    free((void *)file_done_f);
    return -1;
  }

  // create threads
  pthread_t thread_id;
  pthread_attr_t thread_attr;
  for(int i = 0; i < ptr_lookup_params->num_requester; i++)
  {
    pthread_attr_init(&thread_attr);
    if( pthread_create(&thread_id, &thread_attr, requester, (void *)ptr_lookup_info) )
    {
      printf("Failed to create thread\n");
    }
    thread_array[i] = thread_id;
  }
  for(int i = 0; i < ptr_lookup_params->num_resolver; i++)
  {
    pthread_attr_init(&thread_attr);
    if( pthread_create(&thread_id, &thread_attr, resolver, (void *)ptr_lookup_info) )
    {
      printf("Failed to create thread\n");
    }
    thread_array[i + ptr_lookup_params->num_requester] = thread_id;
  }

  // wait for threads
  for(int i = 0; i < num_threads; i++)
  {
    pthread_join(thread_array[i], NULL);
  }

  // free heap memory
  free_lookup_params(ptr_lookup_params);
  free((void *)file_done_f);
  free((void *)ptr_lookup_info->ptr_mutex);
  free((void *)ptr_lookup_info->ptr_printf_mutex);
  free((void *)ptr_lookup_info);
  free((void *)thread_array);

  // get end time
  gettimeofday(&end_time, &time_zone);
  timersub(&end_time, &start_time, &elapsed_time);
  printf("Program finished. Time elapsed: %ld.%06ld s\n", elapsed_time.tv_sec, elapsed_time.tv_usec);

  return 0;
}
