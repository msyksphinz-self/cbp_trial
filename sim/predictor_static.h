#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>

#include "utils.h"
#include <vector>

class PREDICTOR {

public:

  PREDICTOR(void);
  bool    GetPrediction(UINT64 PC);
  void    UpdatePredictor(UINT64 PC, OpType OPTYPE,bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool taken, UINT64 branchTarget);
};
