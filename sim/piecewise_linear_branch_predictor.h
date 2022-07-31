#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>

#include "utils.h"
#include <vector>
#include <cstdint>

#define HIST_LENGTH 20
#define ADDR_RANGE 256 //Value of N
#define GA_RANGE 256 //Value of M

class PREDICTOR
{
public:
  PREDICTOR(void);
  bool    GetPrediction(UINT64 PC);
  void    UpdatePredictor(UINT64 PC, OpType OPTYPE,bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool taken, UINT64 branchTarget);

private:

  bool GHR[HIST_LENGTH];
  UINT64 GA[HIST_LENGTH];
  int output;
  int weight[ADDR_RANGE][GA_RANGE][HIST_LENGTH + 1];
};
