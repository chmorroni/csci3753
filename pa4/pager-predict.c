/*
 * File: pager-predict.c
 * Author:       Christopher Morroni
 * Adopted From: Andy Sayler
 *
 * Project: CSCI 3753 Programming Assignment 4
 * Create Date: Unknown
 * Modify Date: 2018/04/15
 * Description:
 * 	This file contains a predictive pageit
 *      implmentation.
 */

#include <stdio.h> 
#include <stdlib.h>

#include "simulator.h"

typedef struct
{
    int last_page;
    int branched_pages[2];
} Pstatus;

void pageit(Pentry q[MAXPROCESSES])
{
    static int init = 0;
    static int tick = 1;
    static Pstatus pred[MAXPROCESSES];

    // init static variables
    if(!init)
    {
        for(int i = 0; i < MAXPROCESSES; i++)
        {
            pred[i].last_page = 0;
            pred[i].branched_pages[0] = 2 + rand() % 10;
            pred[i].branched_pages[1] = 0;
        }
        init = 1;
    }

    // count number of active processes to divide pages without waste
    int active_procs = 0;
    for(int i = 0; i < MAXPROCESSES; i++)
    {
        if(q[i].active) active_procs++;
    }

    // avoid divide by zero in last run of function
    if(active_procs == 0) active_procs = 1;

    // divide out pages per proc
    int pages_per_proc[MAXPROCESSES];
    for(int i = 0; i < MAXPROCESSES; i++)
    {
        pages_per_proc[i] = 0;
    }
    for(int i = 0, j = 0; j < PHYSICALPAGES; i = (i + 1) % MAXPROCESSES)
    {
        pages_per_proc[i]++;
        j++;
    }

    for(int i = 0; i < MAXPROCESSES; i++)
    {
        if(q[i].active)
        {
            int curr_page = q[i].pc / PAGESIZE;
            int curr_pages = 0;

            // find the current number of pages in memory
            for(int j = 0; j < MAXPROCPAGES; j++)
            {
                if(q[i].pages[j]) curr_pages++;
            }

            // reset static variables when the processor loads a new process
            if(q[i].pc == 0)
            {
                pred[i].last_page = 0;
                pred[i].branched_pages[0] = 0;
                pred[i].branched_pages[1] = 0;
            }

            // check to see if there was a branch
            if( curr_page != pred[i].last_page && curr_page != pred[i].last_page + 1 )
            {
                if(pred[i].branched_pages[0] != curr_page)
                {
                    pred[i].branched_pages[1] = pred[i].branched_pages[0];
                    pred[i].branched_pages[0] = curr_page;
                }
            }

            // swap out pages no longer needed
            for(int j = 0; j < MAXPROCPAGES; j++)
            {
                if( q[i].pages[j] &&
                    !(j == curr_page || j == curr_page + 1 ||
                      j == pred[i].branched_pages[0] || j == pred[i].branched_pages[0] + 1 || pred[i].branched_pages[1] ) &&
                    !(pages_per_proc[i] > 5 && j == pred[i].branched_pages[1] + 1) )
                {
                    pageout(i, j);
                }
            }

            // pull in pages, this will just pull in as many as it can
            if( curr_pages < pages_per_proc[i] && !q[i].pages[ curr_page ] )
            {
                if(pagein(i, curr_page))
                {
                    curr_pages++;
                }
            }
            if( curr_pages < pages_per_proc[i] && !q[i].pages[ curr_page + 1 ] )
            {
                if(pagein(i, curr_page + 1))
                {
                    curr_pages++;
                }
            }
            if( curr_pages < pages_per_proc[i] && !q[i].pages[ pred[i].branched_pages[0] ] )
            {
                if(pagein(i, pred[i].branched_pages[0]))
                {
                    curr_pages++;
                }
            }
            if( curr_pages < pages_per_proc[i] && !q[i].pages[ pred[i].branched_pages[0] + 1 ] )
            {
                if(pagein(i, pred[i].branched_pages[0] + 1))
                {
                    curr_pages++;
                }
            }
            if( curr_pages < pages_per_proc[i] && !q[i].pages[ pred[i].branched_pages[1] ] )
            {
                if(pagein(i, pred[i].branched_pages[1]))
                {
                    curr_pages++;
                }
            }
            if( curr_pages < pages_per_proc[i] && !q[i].pages[ pred[i].branched_pages[1] + 1 ] )
            {
                if(pagein(i, pred[i].branched_pages[1] + 1))
                {
                    curr_pages++;
                }
            }

            pred[i].last_page = curr_page;
        }
    }

    /* advance time for next pageit iteration */
    tick++;
} 
