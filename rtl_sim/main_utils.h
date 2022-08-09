#include "../sim/utils.h"
#include "../sim/bt9_reader.h"

void CheckHeartBeat(UINT64 numIter, UINT64 numMispred)
{
  UINT64 dotInterval=1000000;
  UINT64 lineInterval=30*dotInterval;

 UINT64 d1K   =1000;
 UINT64 d10K  =10000;
 UINT64 d100K =100000;
 UINT64 d1M   =1000000;
 UINT64 d10M  =10000000;
 UINT64 d30M  =30000000;
 UINT64 d60M  =60000000;
 UINT64 d100M =100000000;
 UINT64 d300M =300000000;
 UINT64 d600M =600000000;
 UINT64 d1B   =1000000000;
 UINT64 d10B  =10000000000;


//  if(numIter % lineInterval == 0){ //prints line every 30 million branches
//    printf("\n");
//    fflush(stdout);
//  }
  if(numIter == d1K){ //prints MPKI after 100K branches
    printf("  MPKBr_1K         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d10K){ //prints MPKI after 100K branches
    printf("  MPKBr_10K         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d100K){ //prints MPKI after 100K branches
    printf("  MPKBr_100K         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }
  if(numIter == d1M){
    printf("  MPKBr_1M         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d10M){ //prints MPKI after 100K branches
    printf("  MPKBr_10M         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d30M){ //prints MPKI after 100K branches
    printf("  MPKBr_30M         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d60M){ //prints MPKI after 100K branches
    printf("  MPKBr_60M         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d100M){ //prints MPKI after 100K branches
    printf("  MPKBr_100M         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d300M){ //prints MPKI after 100K branches
    printf("  MPKBr_300M         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d600M){ //prints MPKI after 100K branches
    printf("  MPKBr_600M         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d1B){ //prints MPKI after 100K branches
    printf("  MPKBr_1B         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

  if(numIter == d10B){ //prints MPKI after 100K branches
    printf("  MPKBr_10B         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(numIter));
    fflush(stdout);
  }

}//void CheckHeartBeat
