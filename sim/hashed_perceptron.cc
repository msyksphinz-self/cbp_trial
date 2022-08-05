/*

This code implements a hashed perceptron branch predictor using geometric
history lengths and dynamic threshold setting.

It was written by Daniel A. Jiménez in March 2019. To the extent allowed by
law, the author abdicates all rights to this work and places it in the public
domain.

The original perceptron branch predictor is from Jiménez and Lin, "Dynamic
Branch Prediction with Perceptrons," HPCA 2001.

The idea of using multiple independently indexed tables of perceptron weights
is from Jiménez, "Fast Path-Based Neural Branch Prediction," MICRO 2003 and
later expanded in "Piecewise Linear Branch Prediction" from ISCA 2005.

The idea of using hashes of branch history to reduce the number of independent
tables is documented in three contemporaneous papers:

1. Seznec, "Revisiting the Perceptron Predictor," IRISA technical report, 2004.

2. Tarjan and Skadron, "Revisiting the Perceptron Predictor Again," UVA
technical report, 2004, expanded and published in ACM TACO 2005 as "Merging
path and gshare indexing in perceptron branch prediction"; introduces the term
"hashed perceptron."

3. Loh and Jiménez, "Reducing the Power and Complexity of Path-Based Neural
Branch Prediction," WCED 2005.

The ideas of using "geometric history lengths" i.e. hashing into tables with
histories of exponentially increasing length, as well as dynamically adjusting
the theta parameter, are from Seznec, "The O-GEHL Branch Predictor," from CBP
2004, expanded later as "Analysis of the O-GEometric History Length Branch
Predictor" in ISCA 2005.

This code uses these ideas, but prefers simplicity over absolute accuracy (I
wrote it in about an hour and later spent more time on this comment block than
I did on the code). These papers and subsequent papers by Jiménez and other
authors significantly improve the accuracy of perceptron-based predictors but
involve tricks and analysis beyond the needs of a tool like ChampSim that
targets cache optimizations. If you want accuracy at any cost, see the winners
of the latest branch prediction contest, CBP 2016 as of this writing, but
prepare to have your face melted off by the complexity of the code you find
there. If you are a student being asked to code a good branch predictor for
your computer architecture class, don't copy this code; there are much better
sources for you to plagiarize.

*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashed_perceptron.h"

// this many tables
#define NUM_CPUS 1

#define NTABLES 16

// maximum history length
#define MAXHIST 232

// minimum history length (for table 1; table 0 is biases)
#define MINHIST 3

// speed for dynamic threshold setting
#define SPEED 18

// geometric global history lengths
int history_lengths[NTABLES] = {0, 3, 4, 6, 8, 10, 14, 19, 26, 36, 49, 67, 91, 125, 170, MAXHIST};

// 12-bit indices for the tables
#define LOG_TABLE_SIZE 12
#define TABLE_SIZE (1 << LOG_TABLE_SIZE)

// this many 12-bit words will be kept in the global history
#define NGHIST_WORDS (MAXHIST / LOG_TABLE_SIZE + 1)

// tables of 8-bit weights
int tables[NUM_CPUS][NTABLES][TABLE_SIZE];

// words that store the global history
unsigned int ghist_words[NUM_CPUS][NGHIST_WORDS];

// remember the indices into the tables from prediction to update
unsigned int indices[NUM_CPUS][NTABLES];

// initialize theta to something reasonable,
int theta[NUM_CPUS];

// initialize counter for threshold setting algorithm
int tc[NUM_CPUS];

// perceptron sum
int yout[NUM_CPUS];

PREDICTOR::PREDICTOR (void)
{
  // zero out the weights tables
  memset(tables, 0, sizeof(tables));

  // zero out the global history
  memset(ghist_words, 0, sizeof(ghist_words));

  // make a reasonable theta
  for (int i = 0; i < NUM_CPUS; i++)
    theta[i] = 10;

  if ((m_fp = fopen("hashed_perceptron.log", "w")) == NULL) {
    perror("hashed_perceptron.log");
  }

}

bool PREDICTOR::GetPrediction(uint64_t pc)
{

  // initialize perceptron sum
  int cpu = 0;

  yout[cpu] = 0;

  fprintf (m_fp, "ghist = {");
  for (int i = NGHIST_WORDS-1; i >= 0; i--) {
    fprintf (m_fp, "%03x, ", ghist_words[cpu][i]);
  }
  fprintf (m_fp, "}\n");

  fprintf (m_fp, "PC=%x, ", pc);

  // for each table...
  fprintf (m_fp, "tables = {");
  for (int i = 0; i < NTABLES; i++) {

    // n is the history length for this table
    int n = history_lengths[i];

    // hash global history bits 0..n-1 into x by XORing the words from the
    // ghist_words array
    unsigned int x = 0;

    // most of the words are 12 bits long
    int most_words = n / LOG_TABLE_SIZE;

    // the last word is fewer than 12 bits
    int last_word = n % LOG_TABLE_SIZE;

    // XOR up to the next-to-the-last word
    int j;
    for (j = 0; j < most_words; j++)
      x ^= ghist_words[cpu][j];

    // XOR in the last word
    x ^= ghist_words[cpu][j] & ((1 << last_word) - 1);

    // XOR in the PC to spread accesses around (like gshare)
    x ^= pc;

    // stay within the table size
    x &= TABLE_SIZE - 1;

    // remember this index for update

    indices[cpu][i] = x;

    // add the selected weight to the perceptron sum

    yout[cpu] += tables[cpu][i][x];

    fprintf (m_fp, "{%3x, %2d}, ", x, tables[cpu][i][x]);
  }
  fprintf (m_fp, "}, yout = %d\n", yout[cpu]);

  return yout[cpu] >= 1;
}

void PREDICTOR::UpdatePredictor(UINT64 PC, OpType OPTYPE, bool resolveDir, bool predDir, UINT64 branchTarget)
{

  int cpu = 0;
  // was this prediction correct?

  bool correct = resolveDir == (yout[cpu] >= 1);

  // insert this branch outcome into the global history

  bool b = resolveDir;
  for (int i = 0; i < NGHIST_WORDS; i++) {

    // shift b into the lsb of the current word

    ghist_words[cpu][i] <<= 1;
    ghist_words[cpu][i] |= b;
    // get b as the previous msb of the current word

    b = !!(ghist_words[cpu][i] & TABLE_SIZE);
    ghist_words[cpu][i] &= TABLE_SIZE - 1;

    // fprintf (m_fp, "ghist_words[%d] = %04x, %d, %d, b = %d\n", i, ghist_words[cpu][i],
    //          !(ghist_words[cpu][i] & TABLE_SIZE),
    //          !!(ghist_words[cpu][i] & TABLE_SIZE),
    //          b);
  }

  // get the magnitude of yout

  int a = (yout[cpu] < 0) ? -yout[cpu] : yout[cpu];

  // perceptron learning rule: train if misprediction or weak correct prediction

  if (!correct || a < theta[cpu]) {
    // update weights
    for (int i = 0; i < NTABLES; i++) {
      // which weight did we use to compute yout?

      int* c = &tables[cpu][i][indices[cpu][i]];
      // increment if resolveDir, decrement if not, saturating at 127/-128
      if (resolveDir) {
        if (*c < 127)
          (*c)++;
      } else {
        if (*c > -128)
          (*c)--;
      }
    }

    // dynamic threshold setting from Seznec's O-GEHL paper
    if (!correct) {

      // increase theta after enough mispredictions
      tc[cpu]++;
      if (tc[cpu] >= SPEED) {
        theta[cpu]++;
        tc[cpu] = 0;
      }
    } else if (a < theta[cpu]) {

      // decrease theta after enough weak but correct predictions
      tc[cpu]--;
      if (tc[cpu] <= -SPEED) {
        theta[cpu]--;
        tc[cpu] = 0;
      }
    }
  }

  fprintf (m_fp, "updated      tables = {");
  for (int i = 0; i < NTABLES; i++) {
    int x = indices[cpu][i];
    fprintf (m_fp, "{%3x, %2d}, ", x, tables[cpu][i][x]);
  }
  fprintf (m_fp, "}\n");

}


void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool taken,UINT64 branchTarget)
{

}
