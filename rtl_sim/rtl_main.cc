#include <verilated.h>
#include <verilated_fst_c.h>
#include "Vpredictor.h"

#include "../sim/utils.h"
#include "../sim/bt9_reader.h"
#include "main_utils.h"

#include "../sim/predictor_bimodal.h"

int main(int argc, char* argv[])
{
  Verilated::commandArgs(argc, argv);

  ///////////////////////////////////////////////
  // Init variables
  ///////////////////////////////////////////////

  PREDICTOR *brpred = new PREDICTOR();  // this instantiates the predictor code

  ///////////////////////////////////////////////
  // Init RTL
  ///////////////////////////////////////////////

  int time_counter = 0;
  Vpredictor *dut = new Vpredictor();
  VerilatedFstC* tfp = NULL;
  Verilated::traceEverOn(true);
  tfp = new VerilatedFstC;
  dut->trace(tfp, 100);  // Trace 100 levels of hierarchy
  tfp->open("simx.fst");

  dut->i_reset_n = 0;
  dut->i_clk = 1;
  for (int i = 0; i < 10; i++) {
    dut->i_clk = !dut->i_clk;
    dut->eval();
    tfp->dump(time_counter++);
  }
  dut->i_reset_n = 1;
  for (int i = 0; i < 10; i++) {
    dut->i_clk = !dut->i_clk;
    dut->eval();
    tfp->dump(time_counter++);
  }

  ///////////////////////////////////////////////
  // read each trace recrod, simulate until done
  ///////////////////////////////////////////////

  std::string trace_path;
  trace_path = argv[1];
  bt9::BT9Reader bt9_reader(trace_path);

  std::string key = "total_instruction_count:";
  std::string value;
  bt9_reader.header.getFieldValueStr(key, value);
  UINT64     total_instruction_counter = std::stoull(value, nullptr, 0);
  UINT64 current_instruction_counter = 0;
  key = "branch_instruction_count:";
  bt9_reader.header.getFieldValueStr(key, value);
  UINT64     branch_instruction_counter = std::stoull(value, nullptr, 0);
  UINT64     numMispred =0;
  //ver2    UINT64     numMispred_btbMISS =0;
  //ver2    UINT64     numMispred_btbANSF =0;
  //ver2    UINT64     numMispred_btbATSF =0;
  //ver2    UINT64     numMispred_btbDYN =0;

  UINT64 cond_branch_instruction_counter=0;
  //ver2     UINT64 btb_ansf_cond_branch_instruction_counter=0;
  //ver2     UINT64 btb_atsf_cond_branch_instruction_counter=0;
  //ver2     UINT64 btb_dyn_cond_branch_instruction_counter=0;
  //ver2     UINT64 btb_miss_cond_branch_instruction_counter=0;
  UINT64 uncond_branch_instruction_counter=0;

  //ver2    ///////////////////////////////////////////////
  //ver2    // model simple branch marking structure
  //ver2    ///////////////////////////////////////////////
  //ver2    std::map<UINT64, UINT32> myBtb;
  //ver2    map<UINT64, UINT32>::iterator myBtbIterator;
  //ver2
  //ver2    myBtb.clear();

  ///////////////////////////////////////////////
  // read each trace record, simulate until done
  ///////////////////////////////////////////////

  OpType opType;
  UINT64 PC;
  bool branchTaken;
  UINT64 branchTarget;
  UINT64 numIter = 0;

  for (auto it = bt9_reader.begin(); it != bt9_reader.end(); ++it) {
    CheckHeartBeat(++numIter, numMispred); //Here numIter will be equal to number of branches read

    try {
      bt9::BrClass br_class = it->getSrcNode()->brClass();

      //          bool dirDynamic = (it->getSrcNode()->brObservedTakenCnt() > 0) && (it->getSrcNode()->brObservedNotTakenCnt() > 0); //JD2_2_2016
      //          bool dirNeverTkn = (it->getSrcNode()->brObservedTakenCnt() == 0) && (it->getSrcNode()->brObservedNotTakenCnt() > 0); //JD2_2_2016

      //JD2_2_2016 break down branch instructions into all possible types
      opType = OPTYPE_ERROR;

      if ((br_class.type == bt9::BrClass::Type::UNKNOWN) && (it->getSrcNode()->brNodeIndex())) { //only fault if it isn't the first node in the graph (fake branch)
        opType = OPTYPE_ERROR; //sanity check
      }
      //NOTE unconditional could be part of an IT block that is resolved not-taken
      //          else if (dirNeverTkn && (br_class.conditionality == bt9::BrClass::Conditionality::UNCONDITIONAL)) {
      //            opType = OPTYPE_ERROR; //sanity check
      //          }
      //JD_2_22 There is a bug in the instruction decoder used to generate the traces
      //          else if (dirDynamic && (br_class.conditionality == bt9::BrClass::Conditionality::UNCONDITIONAL)) {
      //            opType = OPTYPE_ERROR; //sanity check
      //          }
      else if (br_class.type == bt9::BrClass::Type::RET) {
        if (br_class.conditionality == bt9::BrClass::Conditionality::CONDITIONAL)
          opType = OPTYPE_RET_COND;
        else if (br_class.conditionality == bt9::BrClass::Conditionality::UNCONDITIONAL)
          opType = OPTYPE_RET_UNCOND;
        else {
          opType = OPTYPE_ERROR;
        }
      }
      else if (br_class.directness == bt9::BrClass::Directness::INDIRECT) {
        if (br_class.type == bt9::BrClass::Type::CALL) {
          if (br_class.conditionality == bt9::BrClass::Conditionality::CONDITIONAL)
            opType = OPTYPE_CALL_INDIRECT_COND;
          else if (br_class.conditionality == bt9::BrClass::Conditionality::UNCONDITIONAL)
            opType = OPTYPE_CALL_INDIRECT_UNCOND;
          else {
            opType = OPTYPE_ERROR;
          }
        }
        else if (br_class.type == bt9::BrClass::Type::JMP) {
          if (br_class.conditionality == bt9::BrClass::Conditionality::CONDITIONAL)
            opType = OPTYPE_JMP_INDIRECT_COND;
          else if (br_class.conditionality == bt9::BrClass::Conditionality::UNCONDITIONAL)
            opType = OPTYPE_JMP_INDIRECT_UNCOND;
          else {
            opType = OPTYPE_ERROR;
          }
        }
        else {
          opType = OPTYPE_ERROR;
        }
      }
      else if (br_class.directness == bt9::BrClass::Directness::DIRECT) {
        if (br_class.type == bt9::BrClass::Type::CALL) {
          if (br_class.conditionality == bt9::BrClass::Conditionality::CONDITIONAL) {
            opType = OPTYPE_CALL_DIRECT_COND;
          }
          else if (br_class.conditionality == bt9::BrClass::Conditionality::UNCONDITIONAL) {
            opType = OPTYPE_CALL_DIRECT_UNCOND;
          }
          else {
            opType = OPTYPE_ERROR;
          }
        }
        else if (br_class.type == bt9::BrClass::Type::JMP) {
          if (br_class.conditionality == bt9::BrClass::Conditionality::CONDITIONAL) {
            opType = OPTYPE_JMP_DIRECT_COND;
          }
          else if (br_class.conditionality == bt9::BrClass::Conditionality::UNCONDITIONAL) {
            opType = OPTYPE_JMP_DIRECT_UNCOND;
          }
          else {
            opType = OPTYPE_ERROR;
          }
        }
        else {
          opType = OPTYPE_ERROR;
        }
      }
      else {
        opType = OPTYPE_ERROR;
      }


      PC = it->getSrcNode()->brVirtualAddr();

      branchTaken = it->getEdge()->isTakenPath();
      branchTarget = it->getEdge()->brVirtualTarget();

      //printf("PC: %llx type: %x T %d N %d outcome: %d", PC, (UINT32)opType, it->getSrcNode()->brObservedTakenCnt(), it->getSrcNode()->brObservedNotTakenCnt(), branchTaken);

      /************************************************************************************************************/

      if (opType == OPTYPE_ERROR) {
        if (it->getSrcNode()->brNodeIndex()) { //only fault if it isn't the first node in the graph (fake branch)
          fprintf(stderr, "OPTYPE_ERROR\n");
          printf("OPTYPE_ERROR\n");
          exit(-1); //this should never happen, if it does please email CBP org chair.
        }
      }
      else if (br_class.conditionality == bt9::BrClass::Conditionality::CONDITIONAL) { //JD2_17_2016 call UpdatePredictor() for all branches that decode as conditional
        //printf("COND ");

        //NOTE: contestants are NOT allowed to use the btb* information from ver2 of the infrastructure below:
        //ver2             myBtbIterator = myBtb.find(PC); //check BTB for a hit
        //ver2            bool btbATSF = false;
        //ver2            bool btbANSF = false;
        //ver2            bool btbDYN = false;
        //ver2
        //ver2            if (myBtbIterator == myBtb.end()) { //miss -> we have no history for the branch in the marking structure
        //ver2              //printf("BTB miss ");
        //ver2              myBtb.insert(pair<UINT64, UINT32>(PC, (UINT32)branchTaken)); //on a miss insert with outcome (N->btbANSF, T->btbATSF)
        //ver2              predDir = brpred->GetPrediction(PC, btbANSF, btbATSF, btbDYN);
        //ver2              brpred->UpdatePredictor(PC, opType, branchTaken, predDir, branchTarget, btbANSF, btbATSF, btbDYN);
        //ver2            }
        //ver2            else {
        //ver2              btbANSF = (myBtbIterator->second == 0);
        //ver2              btbATSF = (myBtbIterator->second == 1);
        //ver2              btbDYN = (myBtbIterator->second == 2);
        //ver2              //printf("BTB hit ANSF: %d ATSF: %d DYN: %d ", btbANSF, btbATSF, btbDYN);
        //ver2
        //ver2              predDir = brpred->GetPrediction(PC, btbANSF, btbATSF, btbDYN);
        //ver2              brpred->UpdatePredictor(PC, opType, branchTaken, predDir, branchTarget, btbANSF, btbATSF, btbDYN);
        //ver2
        //ver2              if (  (btbANSF && branchTaken)   // only exhibited N until now and we just got a T -> upgrade to dynamic conditional
        //ver2                 || (btbATSF && !branchTaken)  // only exhibited T until now and we just got a N -> upgrade to dynamic conditional
        //ver2                 ) {
        //ver2                myBtbIterator->second = 2; //2-> dynamic conditional (has exhibited both taken and not-taken in the past)
        //ver2              }
        //ver2            }
        //ver2            //puts("");

        bool predDir = false;

        predDir = brpred->GetPrediction(PC);
        brpred->UpdatePredictor(PC, opType, branchTaken, predDir, branchTarget);

        // PC Input: Predict Request
        dut->i_pred_valid = 1;
        dut->i_pred_pc = PC;
        dut->i_clk = !dut->i_clk; // Toggle clock
        dut->eval();
        tfp->dump(time_counter++);
        dut->i_clk = !dut->i_clk; // Toggle clock
        dut->eval();
        tfp->dump(time_counter++);

        // Predict Response
        dut->i_pred_valid = 0;
        dut->i_pred_pc = 0;
        dut->i_clk = !dut->i_clk; // Toggle clock
        dut->eval();
        tfp->dump(time_counter++);
        bool predDir_rtl = dut->o_pred_taken;
        dut->i_clk = !dut->i_clk; // Toggle clock
        dut->eval();
        tfp->dump(time_counter++);

        // printf ("PC=%08llx, RTL=%d, Model=%d, Result=%d\n", PC, predDir_rtl, predDir, branchTaken);

        if (predDir_rtl != predDir) {
          fprintf (stderr, "Error prediction RTL / Model different! RTL=%d, ISS=%d\n",  predDir_rtl, predDir);
          dut->final();
          tfp->close();
          exit (1);
        }

        // Training
        dut->i_update_valid = 1;
        dut->i_update_pc = PC;
        dut->i_result_taken = branchTaken;
        dut->i_clk = !dut->i_clk; // Toggle clock
        dut->eval();
        tfp->dump(time_counter++);
        dut->i_clk = !dut->i_clk; // Toggle clock
        dut->eval();
        tfp->dump(time_counter++);

        // Training clear
        dut->i_update_valid = 0;
        dut->i_update_pc = 0;
        dut->i_result_taken = 0;
        dut->i_clk = !dut->i_clk; // Toggle clock
        dut->eval();
        tfp->dump(time_counter++);
        dut->i_clk = !dut->i_clk; // Toggle clock
        dut->eval();
        tfp->dump(time_counter++);

        if(predDir != branchTaken){
          numMispred++; // update mispred stats
          //ver2              if(btbATSF)
          //ver2                numMispred_btbATSF++; // update mispred stats
          //ver2              else if(btbANSF)
          //ver2                numMispred_btbANSF++; // update mispred stats
          //ver2              else if(btbDYN)
          //ver2                numMispred_btbDYN++; // update mispred stats
          //ver2              else
          //ver2                numMispred_btbMISS++; // update mispred stats
        }
        cond_branch_instruction_counter++;

        //ver2            if (btbDYN)
        //ver2              btb_dyn_cond_branch_instruction_counter++; //number of branches that have been N at least once after being T at least once
        //ver2            else if (btbATSF)
        //ver2              btb_atsf_cond_branch_instruction_counter++; //number of branches that have been T at least once, but have not yet seen a N after the first T
        //ver2            else if (btbANSF)
        //ver2              btb_ansf_cond_branch_instruction_counter++; //number of cond branches that have not yet been observed T
        //ver2            else
        //ver2              btb_miss_cond_branch_instruction_counter++; //number of cond branches that have not yet been observed T
      }
      else if (br_class.conditionality == bt9::BrClass::Conditionality::UNCONDITIONAL) { // for predictors that want to track unconditional branches
        uncond_branch_instruction_counter++;
        brpred->TrackOtherInst(PC, opType, branchTaken, branchTarget);
      }
      else {
        fprintf(stderr, "CONDITIONALITY ERROR\n");
        printf("CONDITIONALITY ERROR\n");
        exit(-1); //this should never happen, if it does please email CBP org chair.
      }

      /************************************************************************************************************/
    }
    catch (const std::out_of_range & ex) {
      std::cout << ex.what() << '\n';
      break;
    }

  } //for (auto it = bt9_reader.begin(); it != bt9_reader.end(); ++it)


  ///////////////////////////////////////////
  //print_stats
  ///////////////////////////////////////////

  //NOTE: competitors are judged solely on MISPRED_PER_1K_INST. The additional stats are just for tuning your predictors.

  printf("  TRACE \t : %s\n" , trace_path.c_str());
  printf("  NUM_INSTRUCTIONS            \t : %10llu\n",   total_instruction_counter);
  printf("  NUM_BR                      \t : %10llu\n",   branch_instruction_counter-1); //JD2_2_2016 NOTE there is a dummy branch at the beginning of the trace...
  printf("  NUM_UNCOND_BR               \t : %10llu\n",   uncond_branch_instruction_counter);
  printf("  NUM_CONDITIONAL_BR          \t : %10llu\n",   cond_branch_instruction_counter);
  //ver2      printf("  NUM_CONDITIONAL_BR_BTB_MISS \t : %10llu",   btb_miss_cond_branch_instruction_counter);
  //ver2      printf("  NUM_CONDITIONAL_BR_BTB_ANSF \t : %10llu",   btb_ansf_cond_branch_instruction_counter);
  //ver2      printf("  NUM_CONDITIONAL_BR_BTB_ATSF \t : %10llu",   btb_atsf_cond_branch_instruction_counter);
  //ver2      printf("  NUM_CONDITIONAL_BR_BTB_DYN  \t : %10llu",   btb_dyn_cond_branch_instruction_counter);
  printf("  NUM_MISPREDICTIONS          \t : %10llu\n",   numMispred);
  //ver2      printf("  NUM_MISPREDICTIONS_BTB_MISS \t : %10llu",   numMispred_btbMISS);
  //ver2      printf("  NUM_MISPREDICTIONS_BTB_ANSF \t : %10llu",   numMispred_btbANSF);
  //ver2      printf("  NUM_MISPREDICTIONS_BTB_ATSF \t : %10llu",   numMispred_btbATSF);
  //ver2      printf("  NUM_MISPREDICTIONS_BTB_DYN  \t : %10llu",   numMispred_btbDYN);
  printf("  MISPRED_PER_1K_INST         \t : %10.4f\n",   1000.0*(double)(numMispred)/(double)(total_instruction_counter));
  //ver2      printf("  MISPRED_PER_1K_INST_BTB_MISS\t : %10.4f",   1000.0*(double)(numMispred_btbMISS)/(double)(total_instruction_counter));
  //ver2      printf("  MISPRED_PER_1K_INST_BTB_ANSF\t : %10.4f",   1000.0*(double)(numMispred_btbANSF)/(double)(total_instruction_counter));
  //ver2      printf("  MISPRED_PER_1K_INST_BTB_ATSF\t : %10.4f",   1000.0*(double)(numMispred_btbATSF)/(double)(total_instruction_counter));
  //ver2      printf("  MISPRED_PER_1K_INST_BTB_DYN \t : %10.4f",   1000.0*(double)(numMispred_btbDYN)/(double)(total_instruction_counter));
  printf("\n");
}
