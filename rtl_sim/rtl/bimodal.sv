// Bimodal

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

localparam TABLE_W = 10;
localparam TABLE_SIZE = 1 << TABLE_W;

typedef struct packed {
  logic          valid;
  logic [ 1: 0]  count;
} bimodal_entry_t;

bimodal_entry_t  r_bimodal[TABLE_SIZE-1: 0];

logic [TABLE_W-1: 0] w_pred_index;
assign w_pred_index = i_pred_pc [2 +: TABLE_W];

logic [TABLE_W-1: 0] w_update_index;
assign w_update_index = i_update_pc [2 +: TABLE_W];

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

always_ff @ (posedge i_clk, negedge i_reset_n) begin
  if (!i_reset_n) begin
    for (int i = 0; i < TABLE_SIZE; i++) begin
      r_bimodal[i].valid <= 1'b0;
      r_bimodal[i].count <= 2'b10;
    end
  end else begin
    if (i_update_valid) begin
      r_bimodal[w_update_index].valid <= 1'b1;
      r_bimodal[w_update_index].count <= i_result_taken ? inc_counter(r_bimodal[w_update_index].count) :
                                         dec_counter(r_bimodal[w_update_index].count);
    end
  end // else: !if(dec != 2'b00)
end // always_ff @ (posedge i_clk, negedge i_reset_n)

always_ff @ (posedge i_clk, negedge i_reset_n) begin
  if (!i_reset_n) begin
    o_pred_taken <= 1'b0;
  end else begin
    if (i_pred_valid) begin
      o_pred_taken <= r_bimodal[w_pred_index].count[1];
    end else begin
      o_pred_taken <= 1'b0;
    end
  end
end

logic [TABLE_SIZE-1: 0][ 1: 0]  v_count;
generate for (genvar v_idx = 0; v_idx < TABLE_SIZE; v_idx++) begin : v_loop
  assign v_count[v_idx] = r_bimodal[v_idx].count;
end
endgenerate

endmodule // predictor
