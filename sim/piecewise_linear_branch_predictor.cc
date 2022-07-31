#include "piecewise_linear_branch_predictor.h"
#include <cstdlib>
#include <cstdint>
#include <iostream>

#define HIST_LENGTH 20
#define ADDR_RANGE 256 //Value of N
#define GA_RANGE 256 //Value of M

#define WEIGHT_WIDTH 8

int _MAX_VAL = (int) (pow(2,WEIGHT_WIDTH-1)); // upper bound
int _MIN_VAL = (-1)*(int) pow(2,WEIGHT_WIDTH-1); // lower bound

int theta = (int)(floor(2.14*(HIST_LENGTH + 1)) + 20.58);

bool predicted;

PREDICTOR::PREDICTOR(void)
{
for (int i = 0 ; i < ADDR_RANGE ; i++)
  {
    for (int j = 0 ; j < GA_RANGE ; j++)
    {
      for (int k = 0 ; k < (HIST_LENGTH+1) ; k++)
      {
        weight[i][j][k] = 0;
      }
    }
  }
  for( int k = 0 ; k < HIST_LENGTH ; k++)
  {
    GA[k] = 0 ;
    GHR[k] = false ;
  }
}


bool PREDICTOR::GetPrediction (UINT64 PC)
{
  int k = PC % ADDR_RANGE;

  //initialise output value to bias weight

  output = weight[k][0][0];

  //prediction strategy

  for (int i = 0 ; i < HIST_LENGTH ; i++) {
    int j = GA[i] % GA_RANGE;
    //int j = GA[i];

    if (GHR[i] == true)
      output += weight[k][j][i+1];
    else
      output -= weight[k][j][i+1];
  }

  predicted = true ? (output >= 0) : false;

  return predicted;
}

void PREDICTOR::UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir,
                                 bool predDir, UINT64 branchTarget)
{
  int k = PC % ADDR_RANGE;

  if ((abs(output) < theta) || predDir != resolveDir) {
    if (resolveDir == true) {
      if(weight[k][0][0] < _MAX_VAL) { // upper bound check to go up
        weight[k][0][0] += 1;
      }
    } else {
      if (  weight[k][0][0] > _MIN_VAL  ) { // lower bound check to go down
        weight[k][0][0] -= 1;
      }
    }

    for (int i = 0 ; i < HIST_LENGTH ; i++) {
      int j = GA[i] % GA_RANGE;
      //int j = GA[i];

      if (GHR[i] == resolveDir)  {
        if( weight[k][j][i+1] < _MAX_VAL) { // upper bound check  to go up
          weight[k][j][i+1] += 1 ;
        }
      } else {
        if(( weight[k][j][i+1] > _MIN_VAL) ) { // lower bound check to go down
          weight[k][j][i+1] -= 1 ;
        }
      }
    }
  }

  for (int i = HIST_LENGTH - 1 ; i >= 1 ; i--)
  {
    GA[i] = GA[i-1];
    GHR[i] = GHR[i-1];
  }

  //GA[0] = PC % GA_RANGE;
  GA[0] = PC;
  GHR[0] = resolveDir;
}

void
PREDICTOR::TrackOtherInst (UINT64 PC, OpType opType, bool taken,
                           UINT64 branchTarget)
{
}
