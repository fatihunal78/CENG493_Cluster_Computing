/* pvm program for series summation */
#include <stdio.h>
#include <gmp.h>
#include "pvm3.h"
#define SLAVENAME "seriesslave"

#define NUMBITS 256
#define MAXPROCS 8

#define TIMER_CLEAR	(tv1.tv_sec=tv1.tv_usec=tv2.tv_sec=tv2.tv_usec=0)
#define TIMER_START	gettimeofday(&tv1, (struct timezone*)0)
#define TIMER_STOP	gettimeofday(&tv2, (struct timezone*)0)
#define TIMER_ELAPSED   (tv2.tv_sec-tv1.tv_sec+(tv2.tv_usec-tv1.tv_usec)*1.E-6)


main(int argc, char **argv)
{
    int mytid;                  /* my task id */
    int tids[MAXPROCS];               /* slave task ids */
    int n, m, nproc, i, numt, who[MAXPROCS], msgtype;
    float *p_sum_ptr;
    char results[MAXPROCS][1024];
    mpf_t sum,temp;
    mp_exp_t expon;
    int len;
    int nhost,narch;
    struct pvmhostinfo *hostp;

    char *tempresult;
    char finalresult[1024];

    struct timeval tv1,tv2;
    long t;

    if (argc < 2) {
	printf("Error - please supply number of slave processes on command line\n");
	pvm_exit();
	exit(1);
    }

    nproc = atoi(argv[1]);


    /* enroll in pvm */
    mytid = pvm_mytid();
    pvm_config(&nhost,&narch,&hostp);
    printf("%d hosts desired, %d hosts obtained\n",nproc+1,nhost);
    if(nhost<nproc+1) {
	printf("ERROR - cannot get enough hosts\n");
	printf("%d hosts obtained:\n",nhost);
	for(n=0;n<nhost;n++) {
	    printf("host %d: %s\n",n,hostp[n].hi_name);
	}
	exit(1);
    }

    /* Set number of slaves to start */
    printf("Spawning %d worker tasks ... \n" , nproc);

    /* start up slave tasks */
    /* hostp[0] contains information on zion */
    numt=0;
    for(n=0; n<nproc;n++) {
        printf("attempting spawn on %s\n",hostp[n+1].hi_name);
	m=pvm_spawn(SLAVENAME, (char**)0, 1, hostp[n+1].hi_name, 1, &tids[n]);
	if(m==1) numt++;
	else {
	    printf("ERROR spawning\n");
	    break;
	}
    }
    if( numt < nproc ){
       printf("\n Trouble spawning slaves. Aborting. Error codes are:\n");
       for( i=numt ; i<nproc ; i++ ) {
          printf("TID %d %d\n",i,tids[i]);
       }
       for( i=0 ; i<numt ; i++ ){
          pvm_kill( tids[i] );
       }
       pvm_exit();
       exit(1);
    }

        printf("SUCCESSFUL\n");

	mpf_init2(sum,NUMBITS);
	mpf_init2(temp,NUMBITS);

	TIMER_START;


        /* send out common data */

        pvm_initsend(PvmDataDefault);  /* use default data encoding */
        pvm_pkint(&nproc, 1, 1);       /* pack 1 thing; number of procs */
        pvm_pkint(tids, nproc, 1);     /* pack nproc things; tids array */
        pvm_mcast(tids, nproc, 0);     /* send it to all task-ids in tids */ 
	/* the tid is the sum offset */

        /* wait for return of data from slaves, message tag=5*/
       msgtype = 5;
       for( i=0 ; i<nproc ; i++ ){
         pvm_recv( -1, msgtype );
         pvm_upkint( &who[i], 1, 1 );
	 pvm_upkstr( results[i] ); }   /* unpack data accordingly */
    

       /* add up the results */
        for(i=0;i<nproc;i++) {
	 mpf_set_str(temp, results[i], 10);
	 mpf_add(sum, sum, temp);
       } 


       TIMER_STOP;

       printf("RESULTS for %d slave processors, %d bits of precision\n",
		       nproc,NUMBITS);
       t=1000000*TIMER_ELAPSED;
       printf("TIME: %ld seconds\n",t);

       for(i=0;i<nproc;i++) {
           printf("Results from processor %d:\n\t%s\n",who[i],results[i]);
       }
       tempresult=mpf_get_str(NULL,&expon,10,0,sum);
       finalresult[0]='0';
       finalresult[1]='.';
       strcpy(&finalresult[2],tempresult);
       len=strlen(finalresult);
       sprintf(&(finalresult[len]),"e%ld\0",expon);
       printf("\nSeries Result: %s\n",finalresult);
     
       /* exit from pvm and quit */

  pvm_exit();
}
   

