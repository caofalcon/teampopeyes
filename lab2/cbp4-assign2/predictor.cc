#include "predictor.h"

/////////////////////////////////////////////////////////////
// 2bitsat
/////////////////////////////////////////////////////////////

#define PREDICTOR_TABLE_SIZE_IN_BITS 8192
char predictor_table[PREDICTOR_TABLE_SIZE_IN_BITS / 2];

void InitPredictor_2bitsat() {
  int i;
  for (i = 0; i < PREDICTOR_TABLE_SIZE_IN_BITS/2; i++) {
    predictor_table[i] = 1;
  }
}

bool GetPrediction_2bitsat(UINT32 PC) {
  int table_index = PC;
  table_index = table_index & (4096-1); 
  return (predictor_table[table_index] > 1) ? TAKEN : NOT_TAKEN;
}

void UpdatePredictor_2bitsat(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
  int table_index = PC;
  table_index = table_index & (PREDICTOR_TABLE_SIZE_IN_BITS/2-1); 
  int curr_value = predictor_table[table_index];
  if (resolveDir == TAKEN) {
    predictor_table[table_index] += (curr_value == 3) ? 0 : 1;
  } else {
    predictor_table[table_index] -= (curr_value == 0) ? 0 : 1;
  }
}

/////////////////////////////////////////////////////////////
// 2level
/////////////////////////////////////////////////////////////

#define PHT_NUM		8
#define PHT_LENGTH	64
#define BHT_LENGTH	512


int bh_table[BHT_LENGTH]; // indexed with PC[13:5] - this table has 6 * 512 bits = 384 bytes
char ph_table[PHT_NUM][PHT_LENGTH]; 	// first index - table number, indexed with PC[4:2];
				   	// second index - entry number, indexed with BHR
                                        // this table has 8 * 64 * 2 bits = 128 bytes

void InitPredictor_2level() {
  int i, j;
  for (i = 0; i < PHT_NUM; i++) {
    for (j = 0; j < PHT_LENGTH; j++) {
      ph_table[i][j] = 1;
    }
  }
  for (i = 0; i < BHT_LENGTH; i++) {
    bh_table[i] = 0;
  }
}

bool GetPrediction_2level(UINT32 PC) {
  int ph_table_index = (PC) & (PHT_NUM - 1);
  int bh_entry_index = (PC >> 3) & (BHT_LENGTH - 1);
  
  return (ph_table[ph_table_index][bh_table[bh_entry_index]] > 1) ? TAKEN : NOT_TAKEN;
}

void UpdatePredictor_2level(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
  int ph_table_index = (PC) & (PHT_NUM - 1);
  int bh_entry_index = (PC >> 3) & (BHT_LENGTH - 1);

  // update 2 bit coutner within ph table with logic for 2bit counter
  // update bhr with resolveDir
  int bh_entry = bh_table[bh_entry_index];
  int curr_value = ph_table[ph_table_index][bh_entry];
  
  if (resolveDir == TAKEN) {
    ph_table[ph_table_index][bh_table[bh_entry_index]] += (curr_value == 3) ? 0 : 1;
    bh_table[bh_entry_index] = (PHT_LENGTH - 1) & ((bh_entry << 1) | 1);
  } else {
    ph_table[ph_table_index][bh_table[bh_entry_index]] -= (curr_value == 0) ? 0 : 1;
    bh_table[bh_entry_index] = (PHT_LENGTH - 1) & (bh_entry << 1);
  }

}

/////////////////////////////////////////////////////////////
// openend
/////////////////////////////////////////////////////////////

#define GBHR_LENGTH 60
#define NUM_OF_PERCEPTRONS 128
short history[GBHR_LENGTH]; // in hardware this would only have 60 bits in total
short weight[NUM_OF_PERCEPTRONS][GBHR_LENGTH];
float theta = (1.93 * GBHR_LENGTH) + 14;
float y;

void InitPredictor_openend() {
    int i, j;
    for ( i = 0; i < GBHR_LENGTH; i++)
    {
        history[i] = 0;
        for (j=0; j < NUM_OF_PERCEPTRONS; j++)
        {
	    weight[j][i] = 0;
        }
    }
}

bool GetPrediction_openend(UINT32 PC) {
    int i;
    y = 0;
    int perceptron_index = PC & (NUM_OF_PERCEPTRONS - 1);
    for (i=0;i<GBHR_LENGTH;i++) { 
        y += weight[perceptron_index][i] * history[i];
    }
    return (y<0)? NOT_TAKEN : TAKEN;
}

void UpdatePredictor_openend(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget) {
    int i;
    int perceptron_index = PC & (NUM_OF_PERCEPTRONS - 1);
    GetPrediction_openend(PC);
    float abs_y = (y < 0) ? 0-y : y;
    int result = (resolveDir == TAKEN) ? 1 : -1;
    if ((resolveDir != predDir) || (abs_y <= theta))
    {
        for (i=0;i<GBHR_LENGTH;i++) {
            weight[perceptron_index][i] = (weight[perceptron_index][i] + result*history[i]);
        }
    }
    for (i = GBHR_LENGTH-1; i > 0; i--) {
        history[i] = history[i-1];
    }
    history[0] = (resolveDir == TAKEN) ? 1 : -1;
}

