#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>

#include "utils.h"
#include <vector>

class PREDICTOR {

  const static int LOCAL_LEN = 4096;

  int8_t m_local[LOCAL_LEN];

  static UINT32 get_index(UINT64 PC) {
    return (PC >> 2) % LOCAL_LEN;
  }

public:

  PREDICTOR(void);
  bool    GetPrediction(UINT64 PC);
  void    UpdatePredictor(UINT64 PC, OpType OPTYPE,bool resolveDir, bool predDir, UINT64 branchTarget);
  void    TrackOtherInst(UINT64 PC, OpType opType, bool taken, UINT64 branchTarget);
};
