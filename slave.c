/* pvm program for series summation            */
/* using high precision (gmp lib)              */
/* this is the slave program                   */
#include <stdio.h>
#include <math.h>
#include <gmp.h>
#include "pvm3.h"
#define MASTERNAME "seriesmaster"
#define MAX 100000

#define NUMBITS 256
#define MAXPROCS 8

main()
{
    int mytid;                  /* my task id */
    int tids[MAXPROCS];               /* slave task ids */
    int i, n, m, nproc, me, master, msgtype, factor;
    char *temp;
    char contribution[1024];
    mp_exp_t expon;
    int len;

    /* declare and initialize high precision data types */

    mpf_t partialsum,summand,ONE;
    mpf_init2(ONE,NUMBITS);
    mpf_init2(partialsum,NUMBITS);
    mpf_init2(summand,NUMBITS);
    mpf_set_ui(partialsum,0);
    mpf_set_ui(ONE,1);

    /* enroll in pvm */
    mytid = pvm_mytid();

    /* Receive data from master, unpack it into local variables */
    msgtype = 0;
    pvm_recv( -1, msgtype );
        pvm_upkint(&nproc, 1, 1);
        pvm_upkint(tids, nproc, 1);

    /* Determine which slave I am (0 -- nproc-1) */
    /* this determines which numbers to sum      */
    for( i=0; i<nproc ; i++ )
       if( mytid == tids[i] ){ me = i; break; }

    /* add up my partial sum */

    for(n=1;n<=MAX;n++){
      factor=(nproc*n-me);
      mpf_set_ui(summand,factor);
      mpf_div(summand,ONE,summand);
      mpf_mul(summand, summand, summand);
      mpf_add(partialsum, partialsum, summand);}

    /* mpf_get_str and mpf_set_str are not symmetric
     * must build proper string format */
    temp = mpf_get_str(NULL,&expon,10,0,partialsum);
    contribution[0]='0';
    contribution[1]='.';
    strcpy(&contribution[2],temp);
    len = strlen(contribution);
    sprintf(&(contribution[len]),"e%ld\0",expon);

   /* Send result to master */
    pvm_initsend( PvmDataDefault );
    pvm_pkint( &me, 1, 1 );
    pvm_pkstr( contribution );
    msgtype = 5;
    master = pvm_parent();
    pvm_send( master, msgtype );

    /* deallocate the mpf types */

    mpf_clear(ONE);
    mpf_clear(summand);
    mpf_clear(partialsum);

    /* Program finished. Exit PVM before stopping */
    pvm_exit();
}

