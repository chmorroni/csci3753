/*
 * File: pager-lru.c
 * Author:       Christopher Morroni
 * Adopted From: Andy Sayler
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2018/04/15
 * Description:
 * 	This file contains an lru pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

void pageit(Pentry q[MAXPROCESSES]) { 
    
    /* This file contains the stub for an LRU pager */
    /* You may need to add/remove/modify any part of this file */

    /* Static vars */
    static int initialized = 0;
    static int tick = 1; // artificial time
    static int timestamps[MAXPROCESSES][MAXPROCPAGES];

    /* Local vars */
    int proctmp;
    int pagetmp;

    /* initialize static vars on first run */
    if(!initialized){
      	for(proctmp=0; proctmp < MAXPROCESSES; proctmp++){
      	    for(pagetmp=0; pagetmp < MAXPROCPAGES; pagetmp++){
            		timestamps[proctmp][pagetmp] = 0; 
      	    }
      	}
       	initialized = 1;
    }
    
    // set timestamps for currently used pages
    for(proctmp = 0; proctmp < MAXPROCESSES; proctmp++)
    {
        pagetmp = q[proctmp].pc / PAGESIZE;
        timestamps[proctmp][pagetmp] = tick;
    }

    for(proctmp = 0; proctmp < MAXPROCESSES; proctmp++)
    {
        pagetmp = q[proctmp].pc / PAGESIZE;
        if( q[proctmp].active && !q[proctmp].pages[pagetmp] )
        {
            // pull in page
            if(!pagein(proctmp, pagetmp))
            {
                // on fail, swap out oldest page
                int first_proc = 0, first_page = 0;
                for(int i = 0; i < MAXPROCESSES; i++)
                {
                    for(int j = 1; j < MAXPROCPAGES; j++)
                    {
                        if(q[i].pages[j] && timestamps[i][j] < timestamps[first_proc][first_page])
                        {
                            first_proc = i;
                            first_page = j;
                        }
                    }
                }
                pageout(first_proc, first_page);

                // set timestamp of switched out page to now, so we don't have to break
                timestamps[first_proc][first_page] = tick;
            }
        }
    }

    /* advance time for next pageit iteration */
    tick++;
} 
