//#include "simulator.h"
#include "predictor_pathbased2.h"

#include "utils.h"
#include <assert.h>


PREDICTOR::PREDICTOR() {
}

PREDICTOR::~PREDICTOR() {
}

void print_array(int cols, int rows, int arr[][HISTORYLEN + 1]) {
    for(int row=0; row < rows; row++) {
        for(int col=0; col < cols; col++) {
            printf(" %d ",arr[row][col]);
        }
        std::cout << std::endl;
    }
}
void print_array(int cols, int rows, unsigned int arr[][HISTORYLEN + 1]) {
    for(int row=0; row < rows; row++) {
        for(int col=0; col < cols; col++) {
            printf(" %d ",arr[row][col]);
        }
        std::cout << std::endl;
    }
}
void print_bool_array(int cols, int rows, bool arr[][HISTORYLEN + 1]) {
    for(int row=0; row < rows; row++) {
        for(int col=0; col < cols; col++) {
			std::cout << arr[row][col];
        }
        std::cout << std::endl;
    }
}

void add_address_hist(int address,unsigned int hist[HISTORYLEN + 1]) {
    for (int i = 0; i < HISTORYLEN; i++) {
        hist[i] = hist[i + 1];
    }
    hist[HISTORYLEN] = address;
}


void add_history(bool prediction, bool hist[HISTORYLEN + 1]) {
    for(int i=0; i < HISTORYLEN; i++) {
        hist[i] = hist[i+1];
    }
    hist[HISTORYLEN] = prediction;
}


bool PREDICTOR::GetPrediction(UINT32 pc)
{
  unsigned int i = pc % HASHLEN;
  int y = speculative_res[HISTORYLEN] + weights[i][0];
  bool prediction = (y >= 0);
  int tmp_res[HISTORYLEN + 1] = {0};

  for (int j = 1; j < HISTORYLEN + 1; j++) {
    unsigned int k = HISTORYLEN - j;
    if (prediction) {
      tmp_res[k + 1] = speculative_res[k] + weights[i][j];
    } else {
      tmp_res[k + 1] = speculative_res[k] - weights[i][j];
    }
  }

  for (int i = 0; i < HISTORYLEN + 1; i++) {
    speculative_res[i] = tmp_res[i];
  }

  speculative_res[0] = 0;
  add_history(prediction, spec_global_hist);
  add_address_hist(i, address_hist);
  return prediction;
}

void write_stats(int x, int y) {
	static FILE *fd = NULL;
	if(fd == NULL) {
		std::cout << "open new file" << std::endl;
		if((fd = fopen("/home/documents/uni-ulm/Semester6-singapore/advanced-computer-architecture/projects/project1/stats.csv", "w")) == NULL) {
			fprintf(stderr, "couldn't create and open prediction stat file!\n");
			return;
		}
	}
	fprintf(fd, "%d,%d\n", x,y);
}

void PREDICTOR::UpdatePredictor (UINT32 PC,
                                 OpType opType,
                                 bool resolveDir,
                                 bool predDir,
                                 UINT32 branchTarget)
{
  unsigned int i = PC % HASHLEN;
  int y = res[HISTORYLEN] + weights[i][0];
  if (predDir != resolveDir || (y>=0 && threshold-y>0) || (y<0 && threshold+y>0)) {
    if(resolveDir) {
      weights[i][0] = weights[i][0] + 1;
    } else {
      weights[i][0] = weights[i][0] - 1;
    }
    for (int j = 1; j < HISTORYLEN + 1; j++) {
      unsigned int k = address_hist[HISTORYLEN-j];
      if(resolveDir == spec_global_hist[HISTORYLEN-j]) {
        weights[k][j] = weights[k][j] + 1;
      } else {
        weights[k][j] = weights[k][j] - 1;
      }
    }
  }

  // accurate duplicate correction
  add_history(resolveDir, global_hist);
  int tmp_res[HISTORYLEN + 1] = {0};

  for (int j = 1; j < HISTORYLEN + 1; j++) {
    unsigned int k = HISTORYLEN - j;
    if (resolveDir) {
      tmp_res[k + 1] = res[k] + weights[i][j];
    } else {
      tmp_res[k + 1] = res[k] - weights[i][j];
    }
  }

  for (int i = 0; i < HISTORYLEN + 1; i++) {
    res[i] = tmp_res[i]; // should also copy the first zero here right?
  }
  if (predDir != resolveDir) {
    for (int i = 0; i < HISTORYLEN + 1; i++) {

      spec_global_hist[i] = global_hist[i];
      speculative_res[i] = res[i];
    }
  }

#ifdef DEBUG
  //print_array(HISTORYLEN + 1, 1, &speculative_res);
  //print_array(HISTORYLEN + 1, HASHLEN, weights);
  std::cout <<"update step" << std::endl;
  std::cout << "pc: " << ip % HASHLEN << std::endl;
  std::cout << "pc history: ";
  print_array(HISTORYLEN + 1, 1, &address_hist);
  std::cout << std::endl;
  std::cout <<"global hist" << std::endl;
  print_bool_array(HISTORYLEN + 1, 1, &global_hist);
  std::cout <<"weights" << std::endl;
  print_array(HISTORYLEN + 1, HASHLEN, weights);
  std::cout <<"speculative results" << std::endl;
  print_array(HISTORYLEN + 1, 1, &speculative_res);
#endif
#ifdef DEBUG_STATS
  static int miss = 0;
  static int stats_counter = 0;
  static int counter = 0;

  if(stats_counter == 999) {
    stats_counter = 0;
    //std::cout << ((float) miss) / 10.0 << std::endl;
    write_stats(counter * 1000, (float) miss / 10.0);
    counter ++;
    miss = 0;
  } else {
    if(predDir != resolveDir) miss ++;
    stats_counter++;
  }
#endif
}
