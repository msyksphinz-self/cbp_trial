#include "predictor_bimodal.h"

PREDICTOR::PREDICTOR (void)
{
  for (auto i = 0; i < LOCAL_LEN; i++) {
    m_local[i] = 2;   // weakly taken
  }
}

bool
PREDICTOR::GetPrediction (UINT64 PC)
{
  UINT32 index = get_index(PC);
  // printf("prediction index = %d\n", index);
  return m_local[index] >> 1;
}


void
PREDICTOR::UpdatePredictor (UINT64 PC, OpType OPTYPE, bool resolveDir,
                            bool predDir, UINT64 branchTarget)
{
  UINT32 index = get_index(PC);
  m_local[index] += (resolveDir ? 1 : -1);
  if (m_local[index] > 3) {
    m_local[index] = 3;
  } else if (m_local[index] < 0) {
    m_local[index] = 0;
  }
  // printf("update index = %d, %d\n", index, m_local[index]);
}

void
PREDICTOR::TrackOtherInst (UINT64 PC, OpType opType, bool taken,
                           UINT64 branchTarget)
{
}
