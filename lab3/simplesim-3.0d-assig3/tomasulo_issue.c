
#include <limits.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "host.h"
#include "misc.h"
#include "machine.h"
#include "regs.h"
#include "memory.h"
#include "loader.h"
#include "syscall.h"
#include "dlite.h"
#include "options.h"
#include "stats.h"
#include "sim.h"
#include "decode.def"

#include "instr.h"

/* PARAMETERS OF THE TOMASULO'S ALGORITHM */

#define INSTR_QUEUE_SIZE         10

#define RESERV_INT_SIZE    4
#define RESERV_FP_SIZE     2
#define FU_INT_SIZE        2
#define FU_FP_SIZE         1

#define FU_INT_LATENCY     4
#define FU_FP_LATENCY      9

/* IDENTIFYING INSTRUCTIONS */

//unconditional branch, jump or call
#define IS_UNCOND_CTRL(op) (MD_OP_FLAGS(op) & F_CALL || \
                         MD_OP_FLAGS(op) & F_UNCOND)

//conditional branch instruction
#define IS_COND_CTRL(op) (MD_OP_FLAGS(op) & F_COND)

//floating-point computation
#define IS_FCOMP(op) (MD_OP_FLAGS(op) & F_FCOMP)

//integer computation
#define IS_ICOMP(op) (MD_OP_FLAGS(op) & F_ICOMP)

//load instruction
#define IS_LOAD(op)  (MD_OP_FLAGS(op) & F_LOAD)

//store instruction
#define IS_STORE(op) (MD_OP_FLAGS(op) & F_STORE)

//trap instruction
#define IS_TRAP(op) (MD_OP_FLAGS(op) & F_TRAP) 

#define USES_INT_FU(op) (IS_ICOMP(op) || IS_LOAD(op) || IS_STORE(op))
#define USES_FP_FU(op) (IS_FCOMP(op))

#define WRITES_CDB(op) (IS_ICOMP(op) || IS_LOAD(op) || IS_FCOMP(op))

/* FOR DEBUGGING */

//prints info about an instruction
#define PRINT_INST(out,instr,str,cycle)	\
  myfprintf(out, "%d: %s", cycle, str);		\
  md_print_insn(instr->inst, instr->pc, out); \
  myfprintf(stdout, "(%d)\n",instr->index);

#define PRINT_REG(out,reg,str,instr) \
  myfprintf(out, "reg#%d %s ", reg, str);	\
  md_print_insn(instr->inst, instr->pc, out); \
  myfprintf(stdout, "(%d)\n",instr->index);

/* VARIABLES */

//instruction queue for tomasulo
static instruction_t* instr_queue[INSTR_QUEUE_SIZE];
//number of instructions in the instruction queue
static int instr_queue_size = 0;

//reservation stations (each reservation station entry contains a pointer to an instruction)
static instruction_t* reservINT[RESERV_INT_SIZE];
static instruction_t* reservFP[RESERV_FP_SIZE];

//functional units
static instruction_t* fuINT[FU_INT_SIZE];
static instruction_t* fuFP[FU_FP_SIZE];

//common data bus
static instruction_t* commonDataBus = NULL;

//The map table keeps track of which instruction produces the value for each register
static instruction_t* map_table[MD_TOTAL_REGS];

//the index of the last instruction fetched
static int fetch_index = 0;

/* FUNCTIONAL UNITS */


/* RESERVATION STATIONS */


/* 
 * Description: 
 * 	Checks if simulation is done by finishing the very last instruction
 *      Remember that simulation is done only if the entire pipeline is empty
 * Inputs:
 * 	sim_insn: the total number of instructions simulated
 * Returns:
 * 	True: if simulation is finished
 */
static bool is_simulation_done(counter_t sim_insn) {

  /* ECE552: YOUR CODE GOES HERE */

  return true; //ECE552: you can change this as needed; we've added this so the code provided to you compiles
}

/* 
 * Description: 
 * 	Retires the instruction from writing to the Common Data Bus
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void CDB_To_retire(int current_cycle) {

  /* ECE552: YOUR CODE GOES HERE */

}


/* 
 * Description: 
 * 	Moves an instruction from the execution stage to common data bus (if possible)
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void execute_To_CDB(int current_cycle) {

  /* ECE552: YOUR CODE GOES HERE */

}

/* 
 * Description: 
 * 	Moves instruction(s) from the issue to the execute stage (if possible). We prioritize old instructions
 *      (in program order) over new ones, if they both contend for the same functional unit.
 *      All RAW dependences need to have been resolved with stalls before an instruction enters execute.
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void issue_To_execute(int current_cycle) {

  /* ECE552: YOUR CODE GOES HERE */
  instruction_t * i_instr;
  int i, instr_d_cycle;
  int to_exec_instr_d_cycle = current_cycle;
  // look for oldest instruction that is ready
  // we can do this because instructions are issued in order
  for (i = 0; i < RESERV_INT_SIZE; i++) {
// put the oldest reservINT's into the fuINTs if fuINT is NULL
  }

  int to_exec_instr_d_cycle = current_cycle;
  for (i = 0; i < RESERV_FP_SIZE; i++) {
    instr_d_cycle = reservFP[i]->tom_dispatch_cycle;
    if (instr_d_cycle < to_exec_instr_d_cycle) {
      if (reservFP[i]->Q[0] == NULL && reservFP[i]->Q[1] == NULL) {
        to_exec_instr_d_cycle = instr_d_cycle;
        to_exec_instr = reservFP[i];
      }
    }
  }

  if (to_exec_instr_d_cycle == current_cycle) {
    return;
  }
  // check if there's room in the pipeline
  if (fuFP[0] == NULL) {
    if (IS_FCOMP(to_exec_instr)) {
      fuFP[0] = to_exec_instr;
    }
  }  
}

/* 
 * Description: 
 * 	Moves instruction(s) from the dispatch stage to the issue stage
 * Inputs:
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void dispatch_To_issue(int current_cycle) {

  /* ECE552: YOUR CODE GOES HERE */
  instruction_t * d_instr = instr_queue[0];
  // check for reservation station stalls
  if (IS_FCOMP(d_instr->op)) {
    // floating point instruction
    if (reservFP[0] == NULL) {
      reservFP[0] = d_instr;
    } else if (reservFP[1] == NULL) {
      reservFP[1] = d_instr;
    } else {
      // stall FP
      return;
    }
  } else if (IS_ICOMP(d_instr->op) || IS_LOAD(d_instr->op)
                                   || IS_STORE(d_instr->op)) {
    // integer ex/mem instruction
    if (reservINT[0] == NULL) { 
      reservINT[0] = d_instr;
    } else if (reservINT[1] == NULL) { 
      reservINT[1] = d_instr;
    } else if (reservINT[2] == NULL) { 
      reservINT[2] = d_instr;
    } else if (reservINT[3] == NULL) { 
      reservINT[3] = d_instr;
    } else {
      // stall INT
      return;
    }
  } else {
    // branch instruction
  }

  if (d_instr->r_out[0] != NULL) {
    map_table[d_instr->r_out[0]] = d_instr;
  }
  if {d_instr->r_out[1] != NULL) {
    map_table[d_instr->r_out[1]] = d_instr;
  }

  if (d_instr->r_in[0] != NULL) {
    if (map_table[d_instr->r_in[0]] != NULL) {
      d_instr->Q[0] = map_table[d_instr->r_in[0]];
    }
  }
  if (d_instr->r_in[1] != NULL) {
    if (map_table[d_instr->r_in[1]] != NULL) {
      d_instr->Q[1] = map_table[d_instr->r_in[1]];
    }
  }
  if (d_instr->r_in[2] != NULL) {
    if (map_table[d_instr->r_in[2]] != NULL) {
      d_instr->Q[2] = map_table[d_instr->r_in[2]];
    }
  }

  // remove instruction from instruction queue
  instr_queue++;
  instr_queue_size--;

  d_instr->tom_issue_cycle = current_cycle;

}

/* 
 * Description: 
 * 	Grabs an instruction from the instruction trace (if possible)
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * Returns:
 * 	None
 */
void fetch(instruction_trace_t* trace) {

  /* ECE552: YOUR CODE GOES HERE */
  // recursively skip trap instructions, return when a non-trap instruction
  // is found or if instruction queue is full
  if (instr_queue_size < INSTR_QUEUE_SIZE) {
    // IFQ not full
    fetch_index++;
    if (IS_TRAP(get_instr(trace, fetch_index)->op)) {
      fetch (trace);
    } else {
      return;
    }
  } else {
    // IFQ full
    return;
  }
}

/* 
 * Description: 
 * 	Calls fetch and dispatches an instruction at the same cycle (if possible)
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * 	current_cycle: the cycle we are at
 * Returns:
 * 	None
 */
void fetch_To_dispatch(instruction_trace_t* trace, int current_cycle) {

  fetch(trace);

  /* ECE552: YOUR CODE GOES HERE */
  // check if queue is full
  if (instr_queue_size < INSTR_QUEUE_SIZE) {
    // store fetched instruction in queue if queue is not full
    instr_queue[(++instr_queue_size)] = get_instr (trace, fetch_index);
  } else {
    return;
  }

  instr_queue[instr_queue_size]->tom_dispatch_cycle = current_cycle;

}

/* 
 * Description: 
 * 	Performs a cycle-by-cycle simulation of the 4-stage pipeline
 * Inputs:
 *      trace: instruction trace with all the instructions executed
 * Returns:
 * 	The total number of cycles it takes to execute the instructions.
 * Extra Notes:
 * 	sim_num_insn: the number of instructions in the trace
 */
counter_t runTomasulo(instruction_trace_t* trace)
{
  //initialize instruction queue
  int i;
  for (i = 0; i < INSTR_QUEUE_SIZE; i++) {
    instr_queue[i] = NULL;
  }

  //initialize reservation stations
  for (i = 0; i < RESERV_INT_SIZE; i++) {
      reservINT[i] = NULL;
  }

  for(i = 0; i < RESERV_FP_SIZE; i++) {
      reservFP[i] = NULL;
  }

  //initialize functional units
  for (i = 0; i < FU_INT_SIZE; i++) {
    fuINT[i] = NULL;
  }

  for (i = 0; i < FU_FP_SIZE; i++) {
    fuFP[i] = NULL;
  }

  //initialize map_table to no producers
  int reg;
  for (reg = 0; reg < MD_TOTAL_REGS; reg++) {
    map_table[reg] = NULL;
  }
  
  int cycle = 1;
  while (true) {

     /* ECE552: YOUR CODE GOES HERE */

     cycle++;

     if (is_simulation_done(sim_num_insn))
        break;
  }
  
  return cycle;
}
