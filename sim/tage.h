#include <stdint.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>

#include "utils.h"
#include <vector>

#define Tag uint16_t
#define Index uint16_t
#define Path uint64_t
#define History uint64_t
#define TAGE_BIMODAL_TABLE_SIZE 16384
#define TAGE_MAX_INDEX_BITS 12
#define TAGE_NUM_COMPONENTS 12 // TODO
#define TAGE_BASE_COUNTER_BITS 2
#define TAGE_COUNTER_BITS 3
#define TAGE_USEFUL_BITS 2
#define TAGE_GLOBAL_HISTORY_BUFFER_LENGTH 1024
#define TAGE_PATH_HISTORY_BUFFER_LENGTH 32
#define TAGE_MIN_HISTORY_LENGTH 4
#define TAGE_HISTORY_ALPHA 1.6
#define TAGE_RESET_USEFUL_INTERVAL 512000

struct tage_predictor_table_entry
{
  uint8_t ctr; // The counter on which prediction is based Range - 0-7
  Tag tag; // Stores the tag
  uint8_t useful; // Variable to store the usefulness of the entry Range - 0-3
};

class Tage
{
private:
  /* data */
  int num_branches;                                               // Stores the number of branch instructions since the last useful reset
  uint8_t bimodal_table[TAGE_BIMODAL_TABLE_SIZE];                 // Array represent the counters of the bimodal table
  struct tage_predictor_table_entry predictor_table[TAGE_NUM_COMPONENTS][(1 << TAGE_MAX_INDEX_BITS)];
  uint8_t global_history[TAGE_GLOBAL_HISTORY_BUFFER_LENGTH];      // Stores the global branch history
  uint8_t path_history[TAGE_PATH_HISTORY_BUFFER_LENGTH];          // Stores the last bits of the last N branch PCs
  uint8_t use_alt_on_na;                                          // 4 bit counter to decide between alternate and provider component prediction
  int component_history_lengths[TAGE_NUM_COMPONENTS];             // History lengths used to compute hashes for different components
  uint8_t tage_pred, pred, alt_pred;                              // Final prediction , provider prediction, and alternate prediction
  int pred_comp, alt_comp;                                        // Provider and alternate component of last branch PC
  int STRONG;                                                     //Strength of provider prediction counter of last branch PC

public:
  void init();                                                    // initialise the member variables
  uint8_t predict(uint64_t ip);                                   // return the prediction from tage
  void update(uint64_t ip, uint8_t taken);                        // updates the state of tage

  Index get_bimodal_index(uint64_t ip);                           // helper hash function to index into the bimodal table
  Index get_predictor_index(uint64_t ip, int component);          // helper hash function to index into the predictor table using histories
  Tag get_tag(uint64_t ip, int component);                        // helper hash function to get the tag of particular ip and component
  int get_match_below_n(uint64_t ip, int component);              // helper function to find the hit component strictly before the component argument
  void ctr_update(uint8_t &ctr, int cond, int low, int high);     // counter update helper function (including clipping)
  uint8_t get_prediction(uint64_t ip, int comp);                  // helper function for prediction
  Path get_path_history_hash(int component);                      // hepoer hash function to compress the path history
  History get_compressed_global_history(int inSize, int outSize); // Compress global history of last 'inSize' branches into 'outSize' by wrapping the history

  Tage();
  ~Tage();
};


class PREDICTOR {

 public:

  PREDICTOR(void);
  bool GetPrediction(UINT64 PC);
  void UpdatePredictor(UINT64 PC, OpType OPTYPE,bool resolveDir, bool predDir, UINT64 branchTarget);
  void TrackOtherInst(UINT64 PC, OpType opType, bool taken, UINT64 branchTarget);
};
