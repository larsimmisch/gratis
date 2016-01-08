#include "epd.h"

// types
typedef enum {           // Image pixel -> Display pixel
	EPD_compensate,  // B -> W, W -> B (Current Image)
	EPD_white,       // B -> N, W -> W (Current Image)
	EPD_inverse,     // B -> N, W -> B (New Image)
	EPD_normal       // B -> B, W -> W (New Image)
} EPD_stage;

typedef enum {
	EPD_BORDER_BYTE_NONE,  // no border byte requred
	EPD_BORDER_BYTE_ZERO,  // border byte == 0x00 requred
	EPD_BORDER_BYTE_SET,   // border byte needs to be set
} EPD_border_byte;

// panel configuration
struct EPD_struct {
	int EPD_Pin_PANEL_ON;
	int EPD_Pin_BORDER;
	int EPD_Pin_DISCHARGE;
	int EPD_Pin_RESET;
	int EPD_Pin_BUSY;

	EPD_size size;
	int base_stage_time;
	int factored_stage_time;
	int lines_per_display;
	int dots_per_line;
	int bytes_per_line;
	int bytes_per_scan;
	bool middle_scan;

	bool pre_border_byte;
	EPD_border_byte border_byte;

	EPD_error status;

	const uint8_t *channel_select;
	size_t channel_select_length;

	uint8_t *line_buffer;
	size_t line_buffer_size;

	timer_t timer;
	SPI_type *spi;
};
