module red_pitaya_lock_in (
   // signals
   input                 clk_i           ,  //!< processing clock
   input                 rstn_i          ,  //!< processing reset - active low
   input      [ 14-1: 0] dat_a_i         ,  //!< input data CHA
   input      [ 14-1: 0] dat_b_i         ,  //!< input data CHB
   output     [ 14-1: 0] dat_a_o         ,  //!< output data CHA
   output     [ 14-1: 0] dat_b_o         ,  //!< output data CHB
   output reg [ 8-1:  0] led_o           ,  // LED output
 
   // system bus
   input      [ 32-1: 0] sys_addr        ,  //!< bus address
   input      [ 32-1: 0] sys_wdata       ,  //!< bus write data
   input      [  4-1: 0] sys_sel         ,  //!< bus write byte select
   input                 sys_wen         ,  //!< bus write enable
   input                 sys_ren         ,  //!< bus read enable
   output reg [ 32-1: 0] sys_rdata       ,  //!< bus read data
   output reg            sys_err         ,  //!< bus error indicator
   output reg            sys_ack            //!< bus acknowledge signal
);

localparam  PSR = 12         ;
localparam  ISR = 18         ;
localparam  DSR = 10         ;

//---------------------------------------------------------------------------------
//
//  LED connection

reg [27:0] led_counter;
always @(posedge clk_i) begin
        if (!rstn_i) begin led_counter <= 'h0; 
    end 
    else begin 
        led_counter <= led_counter + 1'b1; 
    end 
end

always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      led_o[8-1:0] <=  8'h0 ;
   end
   else begin
      if (sys_wen) begin
         led_o[4:0] <= led_o[4:0] + 1'b1 ;
      end
      led_o[6:5]   <= {set_1_irst, set_2_irst}   ;
      led_o[7]   <= led_counter[27]   ;
   end
end

//---------------------------------------------------------------------------------
//  PID 11

wire [ 14-1: 0] pid_1_out   ;
reg  [ 14-1: 0] set_1_sp    ;
reg  [ 14-1: 0] set_1_kp    ;
reg  [ 14-1: 0] set_1_ki    ;
reg  [ 14-1: 0] set_1_kd    ;
reg             set_1_irst    ;

red_pitaya_pid_block #(
  .PSR (  PSR   ),
  .ISR (  ISR   ),
  .DSR (  DSR   )      
) i_pid11 (
   // data
  .clk_i        (  clk_i          ),  // clock
  .rstn_i       (  rstn_i         ),  // reset - active low
  .dat_i        (  dat_a_i        ),  // input data
  .dat_o        (  pid_1_out     ),  // output data

   // settings
  .set_sp_i     (  set_1_sp      ),  // set point
  .set_kp_i     (  set_1_kp      ),  // Kp
  .set_ki_i     (  set_1_ki      ),  // Ki
  .set_kd_i     (  set_1_kd      ),  // Kd
  .int_rst_i    (  set_1_irst    )   // integrator reset
);

//---------------------------------------------------------------------------------
//  PID 21

wire [ 14-1: 0] pid_2_out   ;
reg  [ 14-1: 0] set_2_sp    ;
reg  [ 14-1: 0] set_2_kp    ;
reg  [ 14-1: 0] set_2_ki    ;
reg  [ 14-1: 0] set_2_kd    ;
reg             set_2_irst  ;

red_pitaya_pid_block #(
  .PSR (  PSR   ),
  .ISR (  ISR   ),
  .DSR (  DSR   )      
) i_pid21 (
   // data
  .clk_i        (  clk_i          ),  // clock
  .rstn_i       (  rstn_i         ),  // reset - active low
  .dat_i        (  dat_a_i        ),  // input data
  .dat_o        (  pid_2_out     ),  // output data

   // settings
  .set_sp_i     (  set_2_sp      ),  // set point
  .set_kp_i     (  set_2_kp      ),  // Kp
  .set_ki_i     (  set_2_ki      ),  // Ki
  .set_kd_i     (  set_2_kd      ),  // Kd
  .int_rst_i    (  set_2_irst    )   // integrator reset
);

reg  [ 14-1: 0] phase   ;
reg  [ 14-1: 0] gain    ;

//---------------------------------------------------------------------------------
//
//  System bus connection

always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      phase       <= 14'd0 ;
      gain        <= 14'd0 ;
      set_1_sp    <= 14'd0 ;
      set_1_kp    <= 14'd0 ;
      set_1_ki    <= 14'd0 ;
      set_1_kd    <= 14'd0 ;
      set_1_irst  <=  1'b1 ;
      set_2_sp    <= 14'd0 ;
      set_2_kp    <= 14'd0 ;
      set_2_ki    <= 14'd0 ;
      set_2_kd    <= 14'd0 ;
      set_2_irst  <=  1'b1 ;
   end
   else begin
      if (sys_wen) begin
         if (sys_addr[19:0]==16'h0)     {set_1_irst, set_2_irst} <= sys_wdata[ 2-1:0] ;
         if (sys_addr[19:0]==16'h10)    phase     <= sys_wdata[14-1:0] ;
         if (sys_addr[19:0]==16'h14)    gain      <= sys_wdata[14-1:0] ;
         if (sys_addr[19:0]==16'h18)    set_1_sp  <= sys_wdata[14-1:0] ;
         if (sys_addr[19:0]==16'h1C)    set_1_kp  <= sys_wdata[14-1:0] ;
         if (sys_addr[19:0]==16'h20)    set_1_ki  <= sys_wdata[14-1:0] ;
         if (sys_addr[19:0]==16'h24)    set_1_kd  <= sys_wdata[14-1:0] ;

         if (sys_addr[19:0]==16'h28)    set_2_kp  <= sys_wdata[14-1:0] ;
         if (sys_addr[19:0]==16'h2C)    set_2_ki  <= sys_wdata[14-1:0] ;
         if (sys_addr[19:0]==16'h30)    set_2_kd  <= sys_wdata[14-1:0] ;
      end
   end
end

wire sys_en;
assign sys_en = sys_wen | sys_ren;

always @(posedge clk_i)
if (rstn_i == 1'b0) begin
   sys_err <= 1'b0 ;
   sys_ack <= 1'b0 ;
end else begin
   sys_err <= 1'b0 ;

   casez (sys_addr[19:0])
      20'h00 : begin sys_ack <= sys_en;          sys_rdata <= {{32- 2{1'b0}}, set_2_irst,set_1_irst}       ; end 

      20'h10 : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, phase}             ; end 
      20'h14 : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, gain}              ; end 
      20'h18 : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, set_1_sp}          ; end 
      20'h1C : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, set_1_kp}          ; end 
      20'h20 : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, set_1_ki}          ; end 
      20'h24 : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, set_1_kd}          ; end 

      20'h28 : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, set_2_kp}          ; end 
      20'h2C : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, set_2_ki}          ; end 
      20'h30 : begin sys_ack <= sys_en;          sys_rdata <= {{32-14{1'b0}}, set_2_kd}          ; end 

     default : begin sys_ack <= sys_en;          sys_rdata <=  32'h0                              ; end
   endcase
end

//---------------------------------------------------------------------------------
//  Sum and saturation
/*
wire [ 15-1: 0] out_1_sum   ;
reg  [ 14-1: 0] out_1_sat   ;
wire [ 15-1: 0] out_2_sum   ;
reg  [ 14-1: 0] out_2_sat   ;

assign out_1_sum = $signed(pid_11_out) + $signed(pid_12_out);
assign out_2_sum = $signed(pid_22_out) + $signed(pid_21_out);

always @(posedge clk_i) begin
   if (rstn_i == 1'b0) begin
      out_1_sat <= 14'd0 ;
      out_2_sat <= 14'd0 ;
   end
   else begin
      if (out_1_sum[15-1:15-2]==2'b01) // postitive sat
         out_1_sat <= 14'h1FFF ;
      else if (out_1_sum[15-1:15-2]==2'b10) // negative sat
         out_1_sat <= 14'h2000 ;
      else
         out_1_sat <= out_1_sum[14-1:0] ;

      if (out_2_sum[15-1:15-2]==2'b01) // postitive sat
         out_2_sat <= 14'h1FFF ;
      else if (out_2_sum[15-1:15-2]==2'b10) // negative sat
         out_2_sat <= 14'h2000 ;
      else
         out_2_sat <= out_2_sum[14-1:0] ;
   end
end

assign dat_a_o = out_1_sat ;
assign dat_b_o = out_2_sat ;
*/

endmodule
