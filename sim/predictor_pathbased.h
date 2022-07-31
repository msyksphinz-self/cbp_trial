#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include "utils.h"
#include <assert.h>

#define BUDGET 4 // in KiloBytes

// As given in paper
// formula for calculating THETA : floor(1.93*h + 14)
// History lengths taken from Table-1 of the paper
#if BUDGET == 4
  #define LEN_GHR 28
  #define THETA 68          // => 8 bits needed for weights
  #define TABLE_SIZE 141    // (4*8*1024)/(28+1)*8
#elif BUDGET == 8
  #define LEN_GHR 34
  #define THETA 79          //  => 8 bits needed for weights
  #define TABLE_SIZE 234    // (8*8*1024)/(34+1)*8
#elif BUDGET == 32
  #define LEN_GHR 59
  #define THETA 127         //  => 8 bits needed for weights
  #define TABLE_SIZE 546    //  (32*8*1024)/(59+1)*8
#else
  #define LEN_GHR 0
  #define THETA 0
  #define TABLE_SIZE 0
#endif

int debug_n_predictions = 0;
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class PREDICTOR
{
  // 0th bit always set to 1 and never changed because w0 is going to be the bias.
  bool ghr[LEN_GHR + 1];

  // x0, x1, x2, ..., xn
  int weights[TABLE_SIZE][LEN_GHR + 1];   // w0, w1, w2, ..., wn

  int m_temp;

  int v[LEN_GHR+1];
  int r[LEN_GHR+1];

  public:

  PREDICTOR(void)
  {
    ghr[0] = 1;
    for(int i=1; i<=LEN_GHR; i++)
      ghr[i] = 0;              // here 0 is for -1

    for(int i=0; i < TABLE_SIZE; i++) {
      for(int j=0; j <= LEN_GHR; j++) {
        weights[i][j] = 0;
      }
    }

    for (int i = 0; i < LEN_GHR; i++) {
      v[i] = 0;
    }

    printf("\nBUDGET = %d, \nLEN_GHR = %d, TABLE_SIZE = %d, \nTHETA = %d\n\n", BUDGET, LEN_GHR, TABLE_SIZE, THETA);

  }

  // We have 8 bits keeping for all the integers
  // So we keep our weights between -128 t0 127
  int sat_incr(int w) {
    if (w < 127) {
      return w+1;
    } else {
      assert(w == 127);
      return w;
    }
  }

  int sat_decr(int w) {
    if (w > -128) {
      return w-1;
    } else {
      assert(w == -128);
      return w;
    }
  }

  bool GetPrediction(UINT32 PC)
  {
    PC = PC % TABLE_SIZE;
    int temp = r[LEN_GHR] + weights[PC][0];
    m_temp = temp;

    // for (int i = LEN_GHR; i >= 1; i--) {
    for (int j = 1; j <= LEN_GHR; j++) {
      int kj = LEN_GHR - j;
      if (temp >= 0) {
        r[kj+1] = r[kj] + weights[PC][j];
      } else {
        r[kj+1] = r[kj] - weights[PC][j];
      }
    }
    r[0] = 0;

    if(temp >= 0) {
      if (debug_n_predictions){printf("true -> %d\n", temp); debug_n_predictions -= 1;}
      return true;
    } else {
      if (debug_n_predictions){printf("false -> %d\n", temp); debug_n_predictions -= 1;}
      return false;
    }
  }

  void UpdatePredictor(UINT32 PC, OpType opType, bool resolveDir, bool predDir, UINT32 branchTarget)
  {
    PC = PC % TABLE_SIZE;

    int temp = m_temp;
    assert(predDir == (temp >= 0));

    if((abs(temp) <= THETA) || (resolveDir != predDir)) {
      if (resolveDir) {
        weights[PC][0] = sat_incr(weights[PC][0]);
      } else {
        weights[PC][0] = sat_decr(weights[PC][0]);
      }

      for(int j = 1; j <= LEN_GHR; j++) {
        int k = v[j];
        if(resolveDir == ghr[j]) {
          weights[k][j] = sat_incr(weights[k][j]);
        } else {
          weights[k][j] = sat_decr(weights[k][j]);
        }
      }
    }

    for (int i = LEN_GHR; i >= 1; i--) {
      ghr[i] = ghr[i-1];
      v[i] = v[i-1];
    }
    ghr[0] = resolveDir;
    v[0] = PC;
  }

  void TrackOtherInst(UINT32 PC, OpType opType, bool resolveDir, UINT32 branchTarget)
  {

  }


};



/***********************************************************/
#endif
