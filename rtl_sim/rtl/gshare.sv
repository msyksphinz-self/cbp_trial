module predictor
(
 input logic         i_clk,
 input logic         i_reset_n,

 input logic         i_pred_valid,
 input logic [63: 0] i_pred_pc,
 output logic        o_pred_taken,

 input logic         i_update_valid,
 input logic [63: 0] i_update_pc,
 input logic         i_result_taken
 );

localparam GHR_LEN = 16;
localparam PC_OFFSET = 5 + 2;

localparam TABLE_W = GHR_LEN + PC_OFFSET;
localparam TABLE_SIZE = 1 << TABLE_W;

logic [GHR_LEN-1: 0] r_ghr;

logic [ 1: 0]    r_pht[TABLE_SIZE];

logic [TABLE_W-1: 0] w_pred_pht_index;
assign w_pred_pht_index = {i_pred_pc[GHR_LEN] ^ r_ghr, i_pred_pc[PC_OFFSET-1: 0]};

//
// Prediction
//
always_ff @ (posedge i_clk, negedge i_reset_n) begin
  if (!i_reset_n) begin
    o_pred_taken <= 1'b0;
  end else begin
    if (i_pred_valid) begin
      o_pred_taken <= r_pht[w_pred_pht_index][1];
    end
  end
end


logic [TABLE_W-1: 0] w_update_pht_index;
assign w_update_pht_index = {i_update_pc[GHR_LEN] ^ r_ghr, i_update_pc[PC_OFFSET-1: 0]};

always_ff @ (posedge i_clk, negedge i_reset_n) begin
  if (!i_reset_n) begin
    r_ghr <= 'h0;
    for (int i = 0; i < TABLE_SIZE; i++) begin
      r_pht[i] = 2'b10;
    end
  end else begin
    if (i_update_valid) begin
      r_pht[w_update_pht_index] <= i_result_taken ? inc_counter(r_pht[w_update_pht_index]) :
                                   dec_counter(r_pht[w_update_pht_index]);
      r_ghr <= {r_ghr[GHR_LEN-2: 0], i_result_taken};
    end
  end
end

endmodule // predictor

function automatic logic [ 1: 0] inc_counter(logic [ 1: 0] in);
  if (in != 2'b11) begin
    return in + 'h1;
  end else begin
    return in;
  end
endfunction // inc_counter

function automatic logic [ 1: 0] dec_counter(logic [ 1: 0] in);
  if (in != 2'b00) begin
    return in - 'h1;
  end else begin
    return in;
  end
endfunction // inc_counter
