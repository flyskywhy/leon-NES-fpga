#include<stdio.h>

//Start of Macro definition
#define true 1
#define false 0
typedef   signed char  schar;       /*  8 bits     */
typedef unsigned char  uchar;       /*  8 bits     */
typedef   signed short nt16;       /* 16 bits     */
typedef unsigned short uint16;      /* 16 bits     */
typedef unsigned short ushort;      /* 16 bits     */
typedef unsigned int   uint;        /* 16/32+ bits */
typedef unsigned long  ulong;       /* 32 bits     */
typedef   signed int   int32;       /* 32+ bits    */
#define QUANT_TYPE int16
#define BLOCK_TYPE int16
#define JPGD_INBUFSIZE       4096
//------------------------------------------------------------------------------
// May need to be adjusted if support for other colorspaces/sampling factors is added
#define JPGD_MAXBLOCKSPERMCU 10
//------------------------------------------------------------------------------
#define JPGD_MAXHUFFTABLES   8
#define JPGD_MAXQUANTTABLES  4
#define JPGD_MAXCOMPONENTS   4
#define JPGD_MAXCOMPSINSCAN  4
//------------------------------------------------------------------------------
// Increase this if you increase the max width!
#define JPGD_MAXBLOCKSPERROW 6144
//------------------------------------------------------------------------------
// Max. allocated blocks
#define JPGD_MAXBLOCKS    100
//------------------------------------------------------------------------------
#define JPGD_MAX_HEIGHT 8192
#define JPGD_MAX_WIDTH  8192
#define Width_D			352
#define Height_D		288
//------------------------------------------------------------------------------
/* JPEG specific errors */
#define JPGD_BAD_DHT_COUNTS              -200
#define JPGD_BAD_DHT_INDEX               -201
#define JPGD_BAD_DHT_MARKER              -202
#define JPGD_BAD_DQT_MARKER              -203
#define JPGD_BAD_DQT_TABLE               -204
#define JPGD_BAD_PRECISION               -205
#define JPGD_BAD_HEIGHT                  -206
#define JPGD_BAD_WIDTH                   -207
#define JPGD_TOO_MANY_COMPONENTS         -208
#define JPGD_BAD_SOF_LENGTH              -209
#define JPGD_BAD_VARIABLE_MARKER         -210
#define JPGD_BAD_DRI_LENGTH              -211
#define JPGD_BAD_SOS_LENGTH              -212
#define JPGD_BAD_SOS_COMP_ID             -213
#define JPGD_W_EXTRA_BYTES_BEFORE_MARKER -214
#define JPGD_NO_ARITHMITIC_SUPPORT       -215
#define JPGD_UNEXPECTED_MARKER           -216
#define JPGD_NOT_JPEG                    -217
#define JPGD_UNSUPPORTED_MARKER          -218
#define JPGD_BAD_DQT_LENGTH              -219
#define JPGD_TOO_MANY_BLOCKS             -221
#define JPGD_UNDEFINED_QUANT_TABLE       -222
#define JPGD_UNDEFINED_HUFF_TABLE        -223
#define JPGD_NOT_SINGLE_SCAN             -224
#define JPGD_UNSUPPORTED_COLORSPACE      -225
#define JPGD_UNSUPPORTED_SAMP_FACTORS    -226
#define JPGD_DECODE_ERROR                -227
#define JPGD_BAD_RESTART_MARKER          -228
#define JPGD_ASSERTION_ERROR             -229
#define JPGD_BAD_SOS_SPECTRAL            -230
#define JPGD_BAD_SOS_SUCCESSIVE          -231
#define JPGD_STREAM_READ                 -232
#define JPGD_NOTENOUGHMEM                -233
//------------------------------------------------------------------------------
#define JPGD_GRAYSCALE 0
#define JPGD_YH1V1     1
#define JPGD_YH2V1     2
#define JPGD_YH1V2     3
#define JPGD_YH2V2     4

//------------------------------------------------------------------------------
typedef enum
{
  M_SOF0  = 0xC0,
  M_SOF1  = 0xC1,
  M_SOF2  = 0xC2,
  M_SOF3  = 0xC3,

  M_SOF5  = 0xC5,
  M_SOF6  = 0xC6,
  M_SOF7  = 0xC7,

  M_JPG   = 0xC8,
  M_SOF9  = 0xC9,
  M_SOF10 = 0xCA,
  M_SOF11 = 0xCB,

  M_SOF13 = 0xCD,
  M_SOF14 = 0xCE,
  M_SOF15 = 0xCF,

  M_DHT   = 0xC4,

  M_DAC   = 0xCC,

  M_RST0  = 0xD0,
  M_RST1  = 0xD1,
  M_RST2  = 0xD2,
  M_RST3  = 0xD3,
  M_RST4  = 0xD4,
  M_RST5  = 0xD5,
  M_RST6  = 0xD6,
  M_RST7  = 0xD7,

  M_SOI   = 0xD8,
  M_EOI   = 0xD9,
  M_SOS   = 0xDA,
  M_DQT   = 0xDB,
  M_DNL   = 0xDC,
  M_DRI   = 0xDD,
  M_DHP   = 0xDE,
  M_EXP   = 0xDF,

  M_APP0  = 0xE0,
  M_APP15 = 0xEF,

  M_JPG0  = 0xF0,
  M_JPG13 = 0xFD,
  M_COM   = 0xFE,

  M_TEM   = 0x01,

  M_ERROR = 0x100
} JPEG_MARKER;
//------------------------------------------------------------------------------
#define RST0 0xD0
#define max(a,b) (((a)>(b)) ? (a) : (b))

#define min(a,b) (((a)<(b)) ? (a) : (b))

static int ZAG[64] =
{
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
};

//------------------------------------------------------------------------------
//End of Macro definition
typedef struct huff_tables_tag
{
  uint  look_up[256];
  uchar code_size[256];
  // FIXME: Is 512 tree entries really enough to handle _all_ possible
  // code sets? I think so but not 100% positive.
  uint  tree[512];
} huff_tables_t, *Phuff_tables_t;
//------------------------------------------------------------------------------
typedef struct coeff_buf_tag
{
  uchar *Pdata;

  int block_num_x, block_num_y;
  int block_len_x, block_len_y;

  int block_size;

} coeff_buf_t, *Pcoeff_buf_t;

const char *Psrc_filename;
const char *Pdst_filename;

int read(uchar *Pbuf, int max_bytes_to_read, int *Peof_flag);
int Array_Read(uchar *Pbuf, int max_bytes_to_read);

//The JPEG_DECODER's variable definition and function definition

  void free_all_blocks(void);

  void terminate(int status);

  void *alloc(int n);

  void word_clear(void *p, ushort c, uint n);

  void prep_in_buffer(void);

  void read_dht_marker(void);

  void read_dqt_marker(void);

  void read_sof_marker(void);

  void skip_variable_marker(void);

  void read_dri_marker(void);

  void read_sos_marker(void);

  int next_marker(void);

  int process_markers(void);

  void locate_soi_marker(void);

  void locate_sof_marker(void);

  int locate_sos_marker(void);

  void init();

  void create_look_ups(void);

  void fix_in_buffer(void);

  void transform_block(int num_block);
  void decode_next_row(void);
 void check_quant_tables(void);

  void check_huff_tables(void);

  void calc_mcu_block_order(void);

  int init_scan(void);

  void init_frame(void);

  void process_restart(void);

  void init_sequential(void);

  void decode_start(void);

  void decode_init();

  void find_eoi(void);
  uint rol(uint i, uchar j);
  uint get_char_1(void);
  uint get_char_2(int *Ppadding_flag);
  void stuff_char(uchar q);
  uchar get_octet(void);
  uint get_bits_1(int num_bits);
  uint get_bits_2(int numbits);

  int   image_x_size;
  int   image_y_size;

  QUANT_TYPE *quant[JPGD_MAXQUANTTABLES];    /* pointer to quantization tables */

  int   scan_type;                      /* Grey, Yh1v1, Yh1v2, Yh2v1, Yh2v2,
                                           CMYK111, CMYK4114 */

  int   comps_in_frame;                 /* # of components in frame */
  int   comp_h_samp[JPGD_MAXCOMPONENTS];     /* component's horizontal sampling factor */
  int   comp_v_samp[JPGD_MAXCOMPONENTS];     /* component's vertical sampling factor */
  int   comp_quant[JPGD_MAXCOMPONENTS];      /* component's quantization table selector */
  int   comp_ident[JPGD_MAXCOMPONENTS];      /* component's ID */

  int   comp_h_blocks[JPGD_MAXCOMPONENTS];
  int   comp_v_blocks[JPGD_MAXCOMPONENTS];

  int   comps_in_scan;                  /* # of components in scan */
  int   comp_list[JPGD_MAXCOMPSINSCAN];      /* components in this scan */
  int   comp_dc_tab[JPGD_MAXCOMPONENTS];     /* component's DC Huffman coding table selector */
  int   comp_ac_tab[JPGD_MAXCOMPONENTS];     /* component's AC Huffman coding table selector */

  int   spectral_start;                 /* spectral selection start */
  int   spectral_end;                   /* spectral selection end   */
  int   successive_low;                 /* successive approximation low */
  int   successive_high;                /* successive approximation high */

  int   max_mcu_x_size;                 /* MCU's max. X size in pixels */
  int   max_mcu_y_size;                 /* MCU's max. Y size in pixels */

  int   blocks_per_mcu;
  int   max_blocks_per_row;
  int   mcus_per_row, mcus_per_col;

  int   mcu_org[JPGD_MAXBLOCKSPERMCU];

  int   total_lines_left;               /* total # lines left in image */
  int   mcu_lines_left;                 /* total # lines left in this MCU */

  int   real_dest_bytes_per_scan_line;
  int   dest_bytes_per_scan_line;        /* rounded up */
  int   dest_bytes_per_pixel;            /* currently, 4 (RGB) or 1 (Y) */

  void  *blocks[JPGD_MAXBLOCKS];         /* list of all dynamically allocated blocks */

  int eob_run;

  int block_y_mcu[JPGD_MAXCOMPONENTS];

  uchar *Pin_buf_ofs;
  int in_buf_left;
  int tem_flag;
  int eof_flag;

  uchar padd_1[128];
  uchar in_buf[JPGD_INBUFSIZE + 128];
  uchar padd_2[128];

  int   bits_left;

  uint bit_buf;
  
  uint  saved_mm1[2];

  int   restart_interval;
  int   restarts_left;
  int   next_restart_num;

  int   max_mcus_per_row;
  int   max_blocks_per_mcu;

  int   max_mcus_per_col;

  uint *component[JPGD_MAXBLOCKSPERMCU];   /* points into the lastdcvals table */
  uint  last_dc_val[JPGD_MAXCOMPONENTS];



  BLOCK_TYPE *block_seg[JPGD_MAXBLOCKSPERMCU];
  int block_max_zag_set;

  uchar *Psample_buf;
  uchar *MCUTempStore;
  uchar *Pcolor_buf;

  uchar *Pjpg_out;

  int X_Offset_LH,Y_Offset_LH;

  int   crr[256];
  int   cbb[256];
  int   padd;
  long  crg[256];
  long  cbg[256];

  uchar *scan_line_0;
  uchar *scan_line_1;

  BLOCK_TYPE temp_block[64];

  int error_code;
  int ready_flag;

  int total_bytes_read;

  int *LineData;
  uchar *LineStore;             

  int begin(void);

  int decode_image(int ZoomIn, int Multiple, int X_Offset, int Y_Offset);
  int decode_mcu(void);

  void Transform_block(void);
  void store(int num_mcu_row,int num_mcu_col);


  int get_error_code(void);
  int get_width(void);
  int get_height(void);
  int get_num_components(void);
  int get_bytes_per_pixel(void);
  int get_bytes_per_scan_line(void);
  int get_total_bytes_read(void);
  void idct(BLOCK_TYPE *data, uchar *Pdst_ptr);

  //Add later just because it will cause error
  void make_huff_table(
    int index,
    Phuff_tables_t hs);
  int huff_decode(Phuff_tables_t Ph);
  Phuff_tables_t h[JPGD_MAXHUFFTABLES];
  Pcoeff_buf_t dc_coeffs[JPGD_MAXCOMPONENTS];
  Pcoeff_buf_t ac_coeffs[JPGD_MAXCOMPONENTS];
  Phuff_tables_t dc_huff_seg[JPGD_MAXBLOCKSPERMCU];
  Phuff_tables_t ac_huff_seg[JPGD_MAXBLOCKSPERMCU];
  //End of Add later

  uchar *huff_num[JPGD_MAXHUFFTABLES];  /* pointer to number of Huffman codes per bit size */
  uchar *huff_val[JPGD_MAXHUFFTABLES];  /* pointer to Huffman codes per bit size */
  
  //End of JPEG_DECODER definition
  void ISR(int temp);
  void Msg(char *msg);

  //Function used for ZoomIn and ZoomOut
int BackDisposeZoomOut(int Zoom, int mcu_row_num, int mcu_col_num, int index);
  void BackDisposeZoomIn(int Zoom, int mcu_row_num, int mcu_col_num);

  //Function used for UpSample
  void UpSample(int Zoom);

  //Functions used for DownSample
  void DownSampleColor(int Zoom, int index);
  void DownSampleGray(int Zoom, int index);
  void DownSample( int Zoom, int index);

  //Functions used for Save ALine To SDRAM
  void SaveALineToSDRAM(int Line_Num,int Column_Num,int mcu_line_num,int Zoom);
  void SaveChromToSDRAMH2(int Line_Num, int Column_Num,int mcu_line_num, int Zoom);
  void SaveChromToSDRAMH1(int Line_Num, int Column_Num,int mcu_line_num, int Zoom);
  
  void SaveALineToSDRAMLum(int Line_Num, int Column_Num,int mcu_line_num, int Zoom);

