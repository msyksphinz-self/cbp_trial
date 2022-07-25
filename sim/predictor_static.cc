#include "predictor_static.h"

PREDICTOR::PREDICTOR (void)
{
}

bool
PREDICTOR::GetPrediction (UINT64 PC)
{
  return true;
}


void
PREDICTOR::UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir,
                            bool predDir, UINT64 branchTarget)
{
}

void
PREDICTOR::TrackOtherInst (UINT64 PC, OpType opType, bool taken,
                           UINT64 branchTarget)
{
}
