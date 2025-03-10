/* The interface is defined in the following 
* document https://developer.arm.com/documentation/ihi0051/latest/
*/

interface axi_stream_nbits_interface
#(
	parameter TDATA_WIDTH=64
	parameter TID_WIDTH=1
	parameter TDEST_WIDTH=1
	parameter TUSER_WIDTH=1
)
(
	input logic clk,
	input logic rst_n
);

	logic tvalid,
	logic tready,

	logic [ TDATA_WIDTH - 1 : 0 ] tdata,

	logic [ ( TDATA_WIDTH / 8 ) - 1 : 0 ] tstrb,

	logic [ ( TDATA_WIDTH / 8 ) - 1 : 0 ] tkeep;

	logic tlast;

	logic [ TID_WIDTH - 1 : 0 ] tid;
	logic [ TDEST_WIDTH - 1 : 0 ] tdest
	logic [ TUSER_WIDTH - 1 : 0 ] tuser

	modport master(
		input logic clk;
		input logic tready		
		output logic
	)

	modport slave(
		input logic clk;
		input logic tready		
		output logic
	)


endinterface
