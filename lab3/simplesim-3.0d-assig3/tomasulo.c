
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

counter_t num_retired_insn;

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
  /* ECE552: BEGIN CODE */
  if (sim_insn == num_retired_insn) {
    return true; //ECE552: you can change this as needed; we've added this so the code provided to you compiles
  }
  return false;
  /* ECE552: END CODE */
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
  /* ECE552: BEGIN CODE */
  //clear bus 
  // if (commonDataBus != NULL) {
  //   printf("CDB->RETIRE: Retired instruction %d\n", commonDataBus->index);
  // }
  commonDataBus = NULL; 
  /*** DO WE NEED TO DO SOMETHING SPECIAL FOR RETIREMENT***/
  /* ECE552: END CODE */

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
  /* ECE552: BEGIN CODE */
  //check which instructions are ready
  instruction_t * send_instr = NULL; 
  int i, send_instr_cycle;
  send_instr_cycle = current_cycle;  
  for(i = 0; i < FU_INT_SIZE; i++){ //iterate through all entries 
    if (fuINT[i] == NULL)
      continue;
    //printf("exec_cycle:%d\n", fuINT[i]->tom_execute_cycle);
    if((current_cycle - fuINT[i]->tom_execute_cycle) >= FU_INT_LATENCY){ //check if instruction is complete
      if(fuINT[i]->tom_dispatch_cycle < send_instr_cycle){ //check if instruction is oldest 
        send_instr = fuINT[i]; //set instruction to send 
        send_instr_cycle = fuINT[i]->tom_dispatch_cycle; //set oldest cycle as curr instruction 
      }
    }
  }
  if (fuFP[0] != NULL && (current_cycle - fuFP[0]->tom_execute_cycle) >= FU_FP_LATENCY){
    if(fuFP[0]->tom_dispatch_cycle < send_instr_cycle){
      send_instr = fuFP[0]; //set instruction to send 
      send_instr_cycle = fuFP[0]->tom_dispatch_cycle; //set oldest cycle as curr instruction 
    }
  }

  // if we have found an oldest instruction ready to move to cdb
  //send instructions to CDB 
  if(send_instr != NULL){
    commonDataBus = send_instr; 
    send_instr->tom_cdb_cycle = current_cycle;

    //delete send_instr entry in hardware 
    if(IS_FCOMP(send_instr->op)){
      fuFP[0] = NULL;
      for(i = 0; i < RESERV_FP_SIZE; i++){ 
        if (reservFP[i] == NULL) continue;
        if(reservFP[i]->index == send_instr->index){
          reservFP[i] = NULL; 
        }
      } 
    } 
    else{ //instruct an interger op 
      for(i = 0; i < FU_INT_SIZE; i++){ 
        if (fuINT[i] == NULL) continue;
        if(fuINT[i]->index == send_instr->index){
          fuINT[i] = NULL; 
        }
      }
      for(i = 0; i < RESERV_INT_SIZE; i++){ 
        if (reservINT[i] == NULL) continue;
        if(reservINT[i]->index == send_instr->index){ 
          reservINT[i] = NULL; 
        }
      }	
    }
  }
  else{ 
    commonDataBus = NULL;
    return; 
  } // no instructions are ready to be sent 
  
  //delete entry in reservation station 
  //broadcast on CDB
  int broad_reg, j; 
  /*** DO WE HAVE TO CHECK BOTH OUPUT REGISTERS***/ 
  //broadcast fp reservation stations
  for(i = 0; i < RESERV_FP_SIZE; i++){ 
    if (reservFP[i] == NULL) continue;
    for (j = 0; j < 3; j++) {
      if (reservFP[i]->Q[j] == NULL) continue;
      if(reservFP[i]->Q[j]->index == send_instr->index)
      {
        reservFP[i]->Q[j] = NULL;
      }
    }
  } 
  //broadcast int reservation stations
  for(i = 0; i < RESERV_INT_SIZE; i++){  
    if (reservINT[i] == NULL) continue;
    for (j = 0; j < 3; j++) {
      if (reservINT[i]->Q[j] == NULL) continue;
      if(reservINT[i]->Q[j]->index == send_instr->index)
      {
        reservINT[i]->Q[j] = NULL;
      }
    }
  }
  // check in map table if instruction index is still ours
  for (i = 0; i < 2; i++) {
    broad_reg = send_instr->r_out[i];
    if (map_table[broad_reg] != NULL) {
      if (map_table[broad_reg]->index == send_instr->index)
      {
        map_table[broad_reg] = NULL; 
      }
    }
  }
  /*** when do we clear it from the map table ***/ 
  /* ECE552: END CODE */
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
  /* ECE552: BEGIN CODE */
  //temporary variables to hold instructions to execute
  instruction_t * to_exec_instr, *to_exec_instr2;
  to_exec_instr = NULL; 
  to_exec_instr2 = NULL; 
  int i, instr_d_cycle;
  int to_exec_instr_d_cycle, to_exec_instr_d_cycle2; 
  to_exec_instr_d_cycle = current_cycle; //first instruction to execute
  to_exec_instr_d_cycle2 = current_cycle; //second instruction to execute (since fuINT has two spots) 
    
//try to send integer instruction to execute

// 1. look for oldest instruction that is ready we can do this because instructions are issued in order
  for (i = 0; i < RESERV_INT_SIZE; i++) {
    if (reservINT[i] == NULL) {
      continue;
    }
    if (fuINT[0] != NULL) {
      if (reservINT[i]->index == fuINT[0]->index) {
        //printf("ISSUE->EXEC: instruction index %d is in reservINT[%d] and is in fuINT[0]\n", reservINT[i]->index, i);     
	continue;
      }
    }
    if (fuINT[1] != NULL) {
      if (reservINT[i]->index == fuINT[1]->index) {
        //printf("ISSUE->EXEC: instruction index %d is in reservINT[%d] and is in fuINT[1]\n", reservINT[i]->index, i);
        continue;
      }
    }

    // printf("ISSUE->EXEC: instruction index %d is in reservINT[%d] and is not in any fuINT\n", reservINT[i]->index, i);

    instr_d_cycle = reservINT[i]->tom_dispatch_cycle; 
    //check for instructions so see it 2nd oldest and ready 
    if(reservINT[i]->Q[0] == NULL && reservINT[i]->Q[1] == NULL && reservINT[i]->Q[2] == NULL){
      if(instr_d_cycle < to_exec_instr_d_cycle){ 
      	to_exec_instr2 = to_exec_instr; 
        to_exec_instr = reservINT[i]; 
        to_exec_instr_d_cycle = to_exec_instr->tom_dispatch_cycle;
        if (to_exec_instr2 != NULL) {
          to_exec_instr_d_cycle2 = to_exec_instr2->tom_dispatch_cycle;
        }
      }
      else if(instr_d_cycle < to_exec_instr_d_cycle2){
	to_exec_instr2 = reservINT[i];  
	to_exec_instr_d_cycle2 = to_exec_instr2->tom_dispatch_cycle;
      }
      //instructions is not early enough to move to the exec stage
    } /*else {
      if(reservINT[i]->Q[0] != NULL) {
        printf("\tISSUE->EXEC: instruction index %d has a dependency on register %d and cannot execute.\n", reservINT[i]->index, reservINT[i]->Q[0]->index);
      }
      if(reservINT[i]->Q[1] != NULL) {
        printf("\tISSUE->EXEC: instruction index %d has a dependency on register %d and cannot execute.\n", reservINT[i]->index, reservINT[i]->Q[1]->index);
      }
      if(reservINT[i]->Q[2] != NULL) {
        printf("\tISSUE->EXEC: instruction index %d has a dependency on register %d and cannot execute.\n", reservINT[i]->index, reservINT[i]->Q[2]->index);
      }
    } */
  }

  if(fuINT[0] == NULL && fuINT[1] == NULL){ //two free hardware units - try to load both 
    if(to_exec_instr != NULL && ~IS_FCOMP(to_exec_instr->op)){ //make sure ready instruction exists 
      fuINT[0] = to_exec_instr;
      to_exec_instr->tom_execute_cycle = current_cycle; 
    } 
    if(to_exec_instr2 != NULL && ~IS_FCOMP(to_exec_instr2->op)){
      fuINT[1] = to_exec_instr2;
      to_exec_instr2->tom_execute_cycle = current_cycle; 
    }
  }
  else if(fuINT[0] == NULL){
    if(to_exec_instr != NULL && ~IS_FCOMP(to_exec_instr->op)){ //make sure ready instruction exists 
      fuINT[0] = to_exec_instr;
      to_exec_instr->tom_execute_cycle = current_cycle; 
    }
  }
  else if(fuINT[1] == NULL){
    if(to_exec_instr != NULL && ~IS_FCOMP(to_exec_instr->op)){ //make sure ready instruction exists 
      fuINT[1] = to_exec_instr;
      to_exec_instr->tom_execute_cycle = current_cycle; 
    }  
  }
  else{} // not space in int execute hardware -> stall 

  // if (fuINT[0] != NULL) printf("ISSUE->EXEC: Executing instruction index %d on fuINT[0]\n", fuINT[0]->index);
  // if (fuINT[1] != NULL) printf("ISSUE->EXEC: Executing instruction index %d on fuINT[1]\n", fuINT[1]->index);
//try to send floating point instruction to execute

  to_exec_instr_d_cycle = current_cycle;

  for (i = 0; i < RESERV_FP_SIZE; i++) {
    if (reservFP[i] == NULL) {
      continue;
    }
    // printf("ISSUE->EXEC: instruction index %d is in reservFP[%d]\n", reservINT[i]->index, i);
    instr_d_cycle = reservFP[i]->tom_dispatch_cycle;
    if (instr_d_cycle < to_exec_instr_d_cycle) {
      if (reservFP[i]->Q[0] == NULL && reservFP[i]->Q[1] == NULL && reservFP[i]->Q[2] == NULL) {
        to_exec_instr_d_cycle = instr_d_cycle;
        to_exec_instr = reservFP[i];
      }
    }
  }

  // check if there's room in the pipeline
  if (fuFP[0] == NULL) {
    if (to_exec_instr != NULL && IS_FCOMP(to_exec_instr->op)) {  
      fuFP[0] = to_exec_instr;
      to_exec_instr->tom_execute_cycle = current_cycle; 
    } else {
      // stall INT
      return;
    }
  }

  // if (fuFP[0] != NULL) printf("ISSUE->EXEC: Executing instruction index %d on fuFP[0]\n", fuFP[0]->index);
  /* ECE552: END CODE */
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
  /* ECE552: BEGIN CODE */
  int i;
  int do_dependency_check = 1;
  instruction_t * d_instr = instr_queue[0];
  // check for reservation station stalls
  if (d_instr == NULL) {
    return;
  }
  if (IS_FCOMP(d_instr->op)) {
    // floating point instruction
    if (reservFP[0] == NULL) {
      reservFP[0] = d_instr;
    } else if (reservFP[1] == NULL) {
      reservFP[1] = d_instr;
    } else {
      // stall FP
      // printf("DISPATCH->ISSUE: Cannot issue instruction index: %d: reservFPs full\n", d_instr->index);
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
      // printf("DISPATCH->ISSUE: Cannot issue instruction index: %d: reservINTs full\n", d_instr->index);
      // stall INT
      return;
    }
  } else {
    // branch instruction
    // printf("\tDISPATCH->ISSUE: instruction index %d is a branch\n", d_instr->index);
    num_retired_insn++;
    do_dependency_check =0;
  }
  
  if (do_dependency_check > 0) {
    //check input dependencies
    if (d_instr->r_in[0] != DNA) {
      if (map_table[d_instr->r_in[0]] != NULL) {
        d_instr->Q[0] = map_table[d_instr->r_in[0]];
      }
    }
    if (d_instr->r_in[1] != DNA) {
      if (map_table[d_instr->r_in[1]] != NULL) {
        d_instr->Q[1] = map_table[d_instr->r_in[1]];
      }
    }
    if (d_instr->r_in[2] != DNA) {
      if (map_table[d_instr->r_in[2]] != NULL) {
        d_instr->Q[2] = map_table[d_instr->r_in[2]];
      }
    }

    // check output dependencies
    if (d_instr->r_out[0] != DNA) {
      map_table[d_instr->r_out[0]] = d_instr;
    }
    if (d_instr->r_out[1] != DNA) {
      map_table[d_instr->r_out[1]] = d_instr;
    }

    d_instr->tom_issue_cycle = current_cycle;
  }

  // remove instruction from instruction queue
  // printf("DISPATCH->ISSUE: Removing %d from queue head\n", instr_queue[0]->index);
  for (i = 0; i < instr_queue_size-1; i++)
  {
    // printf("DISPATCH->ISSUE: Swap-Out: %d, Swap In: %d, Queue Index: %d\n", instr_queue[i]->index, instr_queue[i+1]->index, i);
    instr_queue[i] = instr_queue[i+1];
  }
  instr_queue_size--;
  /* ECE552: END CODE */
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
  /* ECE552: BEGIN CODE */
  // recursively skip trap instructions, return when a non-trap instruction
  // is found or if instruction queue is full
  if (instr_queue_size < INSTR_QUEUE_SIZE) {
    // IFQ not full
    // if (trace == NULL) printf("TRACE NULL @ 452\n");
    instruction_t * trap_inst = get_instr(trace, fetch_index);
    if (IS_TRAP(trap_inst->op)) {
      // printf("found trap at %d\n",  trap_inst->index);
      fetch_index++;
      // printf("\tFETCH: Instruction index %d is a trap\n", trap_inst->index);
      num_retired_insn++;
      fetch (trace);
    } else {
      return;
    }
  } else {
    // IFQ full
    return;
  }
  /* ECE552: END CODE */
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

  /* ECE552: YOUR CODE GOES HERE */
  /* ECE552: BEGIN CODE */
  fetch(trace);
  // printf("FETCH->DISPATCH: Fetch index is %d\n", fetch_index);

  // check if queue is full
  if (instr_queue_size < INSTR_QUEUE_SIZE && fetch_index <= sim_num_insn) {
    // store fetched instruction in queue if queue is not full
    instr_queue[instr_queue_size] = get_instr(trace, fetch_index);
    fetch_index++;

    instr_queue[instr_queue_size]->tom_dispatch_cycle = current_cycle;
    instr_queue_size++;
  } else {
    return;
  }

  /* ECE552: END CODE */
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
     /* ECE552: BEGIN CODE */
     
     if (commonDataBus != NULL){
       // printf("\tCDB != NULL: retire instruction %d\n", commonDataBus->index);
       num_retired_insn++;
       //printf("%d instr%d: cycle %d", num_retired_insn, commonDataBus->index, cycle);
     }
     CDB_To_retire(cycle);
     execute_To_CDB(cycle);
     issue_To_execute(cycle);
     dispatch_To_issue(cycle);
     fetch_To_dispatch(trace, cycle);

     cycle++;
     // printf("*** CYCLE %d ***\n", cycle);
     /* ECE552: END CODE */

     if (is_simulation_done(sim_num_insn))
        break;
  }
  
  return cycle;
}
