#include <string.h>
#include "stdlib.h"

#include "jpeg_operation.h"
#include "jpeglib.h"

#include "helios_debug.h"
#include "helios_fs.h"

#define JPEG_LOG(msg, ...)      custom_log("jpeg", msg, ##__VA_ARGS__)

int R_V[256];
int G_V[256],G_U[256];
int B_U[256];



const short RGB565_MASK_RED = (0x1F)<<11;
const short RGB565_MASK_GREEN = (0x3F)<<5;
const short RGB565_MASK_BLUE = (0x1F);

static char is_first_uv_init = 1;
static void quec_uv_table_init(void)//��ʼ��uv��
{
	int i;
	for(i=0;i<256;i++)
	{	
	   R_V[i] = (1.402 * (i - 128));
	   G_V[i] = (0.71414*(i - 128));
	   G_U[i] = (0.34413*(i - 128));
	   B_U[i] = (1.772*(i - 128));	 
	}
}


void rgb5652bgr888(short *src, char *dest, int size){
	int i = 0;
    for(i = 0; i<  size; i++){
        unsigned short RGB16 = *src;
        dest[i*3+0] = (RGB16&RGB565_MASK_RED) >> 11;
        dest[i*3+1] = (RGB16&RGB565_MASK_GREEN) >> 5;
        dest[i*3+2] = (RGB16&RGB565_MASK_BLUE);
        dest[i*3+0] <<= 3;
        dest[i*3+1] <<= 2;
        dest[i*3+2] <<= 3;
        src++;
    }

}

int  rgb888torgb565(unsigned char* rgb888_buf, int rgb888_size, unsigned short *rgb565_buf, int rgb565_size)
 
{
 
    int                         i = 0 ;
 
    unsigned char               Red = 0 ;
 
    unsigned char               Green = 0 ;
 
    unsigned char               Blue = 0 ;
 
    int                         count = 0 ;
  
    if(rgb888_buf == NULL || rgb888_size <= 0 || rgb565_buf == NULL || rgb565_size <= 0 || (rgb565_size < (rgb888_size/3) ) )
 
    {
 
        JPEG_LOG("Invail input parameter in %s\n", __FUNCTION__) ;
 
        return -1 ;
 
    }
 
    for(i = 0; i<rgb888_size; i += 3)
 
    {
 
        Red = rgb888_buf[i] >> 3;
 
        Green = rgb888_buf[i+1] >> 2 ;
 
        Blue = rgb888_buf[i+2] >> 3;
 
        
 
        rgb565_buf[count++] = ( (Red<<11)|(Green<<5)|(Blue) ) ;
 
    }
 
 
 
    return count ;
 
}

int yuv420_nv12_to_rgb565_zoom_out(uint_16 *pRgbData, uint_32 rgbWidth, uint_32 rgbHeight, uint_8 *pYuvData, uint_32 yuvWidth, uint_32 yuvHeight)
{
	uint_32 i, j;
	int r, g, b;
	uint_8 nr, ng, nb;
	uint_16 color_value = 0;
	static uint_32 nyuvWidth=0,nyuvHeight=0;
	static uint_32 nrgbWidth=0,nrgbHeight=0;
	uint_32 stepY = yuvWidth;
	uint_32 stepUV = yuvWidth;
	unsigned char *pYdata = pYuvData;
	unsigned char *pUVdata = pYuvData + yuvWidth * yuvHeight;
	uint_32 yuv_i,yuv_j;
	static float zoom_out_val_w_f = 0.0;
	static float zoom_out_val_h_f = 0.0;

	if(is_first_uv_init == 1) {
		quec_uv_table_init();
		is_first_uv_init = 0;
	}

	if ( (!pRgbData) || (!pYuvData) )
	{
		JPEG_LOG("yuv420_nv12_to_rgb565_zoom_out, data is null, pRgbData:0x%x, pSrcRgbData:0x%x\n", pRgbData, pYuvData);
		return -1;
	}

	if ( (yuvWidth < rgbWidth) || (yuvHeight < rgbHeight) )
	{
		JPEG_LOG("yuv420_nv12_to_rgb565_zoom_out, size error, rgb(WxH=%dx%d), yuv(WxH=%dx%d)\n", rgbWidth, rgbWidth, yuvWidth, yuvHeight);
		return -1;
	}
	if(nyuvWidth!=yuvWidth||nyuvHeight!=yuvHeight||nrgbWidth!=rgbWidth||nrgbHeight!=rgbHeight)
	{
		
		zoom_out_val_w_f = (float)((float)yuvWidth / (float)rgbWidth);
		zoom_out_val_h_f = (float)((float)yuvHeight / (float)rgbHeight);
		nyuvWidth = yuvWidth;
		nyuvHeight = yuvHeight;
		nrgbWidth = rgbWidth;
		nrgbHeight = rgbHeight;
	}

	for (i = 0; i < rgbHeight; i++)
	{
		for (j = 0; j < rgbWidth; j++)
		{
			int indexY, indexU, indexV;
			uint_8 Y, U, V;

			/* YUV -> RGB */
			yuv_i = (uint_32)(i * zoom_out_val_h_f);
			yuv_j = (uint_32)(j * zoom_out_val_w_f);

			indexY = yuv_i * stepY + yuv_j;
			Y = pYdata[indexY];

			if (yuv_j % 2 == 0)//��ż��������һ��uv����
			{
				indexU = yuv_i / 2 * stepUV + yuv_j;
				indexV = yuv_i / 2 * stepUV + yuv_j + 1;
				U = pUVdata[indexU];
				V = pUVdata[indexV];
			}
			else if (yuv_j % 2 == 1)
			{
				indexV = yuv_i / 2 * stepUV + yuv_j;
				indexU = yuv_i / 2 * stepUV + yuv_j - 1;
				U = pUVdata[indexU];
				V = pUVdata[indexV];
			}
			/*pony 2021-02-19�޸Ĵ˴�ͨ������ȡֵ�����Ǽ���*/
			r = (Y + R_V[V]);
			g = (Y - G_U[U] - G_V[V]);
			b = (Y + B_U[U]);
			/*��ֹ�������*/
			r = r > 255?255:(r < 0?0:r);
			g = g > 255?255:(g < 0?0:g);
			b = b > 255?255:(b < 0?0:b);
			/* RGB -> RGB565 */
			nr = (uint_8)(r * 31 / 255);
			ng = (uint_8)(g * 63 / 255);
			nb = (uint_8)(b * 31 / 255);
			
			color_value = (uint_16)(((nr & 0x1F)<< 11) | ((ng & 0x3F) << 5) | (nb & 0x1F));
			pRgbData[(i*rgbWidth + j)] = color_value;
		}
	}
	return 0;
}

int yuv420_nv12_to_rgb888(uint_8 *pRgb888Data, uint_32 rgbWidth, uint_32 rgbHeight, uint_8 *pYuvData, uint_32 yuvWidth, uint_32 yuvHeight)
{
	uint_32 i, j;
	int r, g, b;
	static uint_32 nyuvWidth=0,nyuvHeight=0;
	static uint_32 nrgbWidth=0,nrgbHeight=0;
	uint_32 stepY = yuvWidth;
	uint_32 stepUV = yuvWidth;
	unsigned char *pYdata = pYuvData;
	unsigned char *pUVdata = pYuvData + yuvWidth * yuvHeight;
	uint_32 yuv_i,yuv_j;
	static float zoom_out_val_w_f = 0.0;
	static float zoom_out_val_h_f = 0.0;

	if(is_first_uv_init == 1) {
		quec_uv_table_init();
		is_first_uv_init = 0;
	}

	if ( (!pRgb888Data) || (!pYuvData) )
	{
		JPEG_LOG("yuv420_nv12_to_rgb565_zoom_out, data is null, pRgbData:0x%x, pSrcRgbData:0x%x\n", pRgb888Data, pYuvData);
		return -1;
	}

	if ( (yuvWidth < rgbWidth) || (yuvHeight < rgbHeight) )
	{
		JPEG_LOG("yuv420_nv12_to_rgb565_zoom_out, size error, rgb(WxH=%dx%d), yuv(WxH=%dx%d)\n", rgbWidth, rgbWidth, yuvWidth, yuvHeight);
		return -1;
	}
	if(nyuvWidth!=yuvWidth||nyuvHeight!=yuvHeight||nrgbWidth!=rgbWidth||nrgbHeight!=rgbHeight)
	{
		
		zoom_out_val_w_f = (float)((float)yuvWidth / (float)rgbWidth);
		zoom_out_val_h_f = (float)((float)yuvHeight / (float)rgbHeight);
		nyuvWidth = yuvWidth;
		nyuvHeight = yuvHeight;
		nrgbWidth = rgbWidth;
		nrgbHeight = rgbHeight;
	}

	int rbg_cur = 0;
	for (i = 0; i < rgbHeight; i++)
	{
		for (j = 0; j < rgbWidth; j++)
		{
			int indexY, indexU, indexV;
			uint_8 Y, U, V;

			/* YUV -> RGB */
			yuv_i = (uint_32)(i * zoom_out_val_h_f);
			yuv_j = (uint_32)(j * zoom_out_val_w_f);

			indexY = yuv_i * stepY + yuv_j;
			Y = pYdata[indexY];

			if (yuv_j % 2 == 0)//��ż��������һ��uv����
			{
				indexU = yuv_i / 2 * stepUV + yuv_j;
				indexV = yuv_i / 2 * stepUV + yuv_j + 1;
				U = pUVdata[indexU];
				V = pUVdata[indexV];
			}
			else if (yuv_j % 2 == 1)
			{
				indexV = yuv_i / 2 * stepUV + yuv_j;
				indexU = yuv_i / 2 * stepUV + yuv_j - 1;
				U = pUVdata[indexU];
				V = pUVdata[indexV];
			}
			/*pony 2021-02-19�޸Ĵ˴�ͨ������ȡֵ�����Ǽ���*/
			r = (Y + R_V[V]);
			g = (Y - G_U[U] - G_V[V]);
			b = (Y + B_U[U]);
			/*��ֹ�������*/
			r = r > 255?255:(r < 0?0:r);
			g = g > 255?255:(g < 0?0:g);
			b = b > 255?255:(b < 0?0:b);
			/* RGB -> RGB565 */
			pRgb888Data[rbg_cur++] = r;
			pRgb888Data[rbg_cur++] = g;
			pRgb888Data[rbg_cur++] = b;

		}
	}
	return 0;
}




int  JPEG2RGB565(const char* jpeg_file, rgb_struct *rgb565)
{
	JPEG_LOG("JPEG2RGB565 start\n");
	
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    HeliosFILE * infile;
    int row_stride;
    unsigned char *buffer;



    // 分配和初始化一个decompression结构体
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // 指定源文件
    if ((infile = Helios_fopen(jpeg_file, "rb")) == NULL) {
        JPEG_LOG("can't open %s\n", jpeg_file);
        return -1;
    }
    jpeg_stdio_src(&cinfo, infile);

    // 用jpeg_read_header获得jpg信息
    jpeg_read_header(&cinfo, TRUE);
    /* 源信息 */
    JPEG_LOG("image_width = %d\n", cinfo.image_width);
    JPEG_LOG("image_height = %d\n", cinfo.image_height);
    JPEG_LOG("num_components = %d\n", cinfo.num_components);
	rgb565->height = cinfo.image_height;
	rgb565->width = cinfo.image_width;

    // 设置解压参数,比如放大、缩小
    JPEG_LOG("enter scale M/N:\n");
    //scanf("%d/%d", &cinfo.scale_num, &cinfo.scale_denom);
    JPEG_LOG("scale to : %d/%d\n", cinfo.scale_num, cinfo.scale_denom);

	cinfo.out_color_space=JCS_RGB;

    // 启动解压：jpeg_start_decompress   
    jpeg_start_decompress(&cinfo);

    /* 输出的图象的信息 */
    JPEG_LOG("output_width = %d\n", cinfo.output_width);
    JPEG_LOG("output_height = %d\n", cinfo.output_height);
    JPEG_LOG("output_components = %d\n", cinfo.output_components);//解压的是rgb，故为3元素

    // 一行的数据长度
    row_stride = cinfo.output_width * cinfo.output_components;
	
    buffer = malloc(row_stride);//分配空间用来存储一行数据

    // 循环调用jpeg_read_scanlines来一行一行地获得解压的数据
    unsigned char* buf_rgb565 = malloc(cinfo.output_width*cinfo.output_height*2);
	unsigned char* buf_tmp = buf_rgb565;
    while (cinfo.output_scanline < cinfo.output_height) 
    {
    	buf_tmp = buf_rgb565 + (cinfo.output_width*2*cinfo.output_scanline);
        (void) jpeg_read_scanlines(&cinfo, &buffer, 1);
		rgb888torgb565(buffer, row_stride, (unsigned short*)buf_tmp, cinfo.output_width);

    }

	if(buffer) {
		free(buffer);
	}
	rgb565->buf = buf_rgb565;
	
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return 0;
}




//RGB 压缩成jpeg
int RGB5652JPEG(const char* jpeg_file, unsigned char* rgb_buffer, int width, int height, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int row_stride = 0;
    HeliosFILE* fp = NULL;
    JSAMPROW row_pointer[1];

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);
    fp = Helios_fopen(jpeg_file, "wb");
    if (fp == NULL)
    {
        JPEG_LOG("open HeliosFILE %s failed.\n", jpeg_file);
        return -1;
    }
    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;//设置输入格式
    cinfo.dct_method = JDCT_FLOAT;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, 1);  // todo 1 == true
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = width * cinfo.input_components;

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &rgb_buffer[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
	
    if(0 != Helios_fclose(fp)) {
		JPEG_LOG("fs close failed\n");
		return -1;
	}

    return 0;
}







int yuv420_NV12_to_jpg(char *filename, int img_width, int img_height, unsigned char *pYUVBuffer, int width, int height)
{
	
    HeliosFILE *pJpegFile = NULL;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];  
	unsigned char yuvbuf[width * 3];
	unsigned char *ybase = NULL, *ubase = NULL;
	
    uint_32 yuv_i,yuv_j;
	int i=0, j=0;
	int idx=0;
	
    float zoom_out_val_w_f = 0.0;
    float zoom_out_val_h_f = 0.0;

 
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
 
	if ((pJpegFile = Helios_fopen(filename, "wb")) == NULL)
	{	 
		return -1;
	}	 
	jpeg_stdio_dest(&cinfo, pJpegFile);
 
	// image width and height, in pixels
	cinfo.image_width	   = img_width;
	cinfo.image_height	   = img_height;	 
	cinfo.input_components = 3;    // # of color components per pixel
	cinfo.in_color_space   = JCS_YCbCr;  //colorspace of input image
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 80, TRUE );
  
	cinfo.jpeg_color_space = JCS_YCbCr;
	cinfo.comp_info[0].h_samp_factor = 2;
	cinfo.comp_info[0].v_samp_factor = 2;
  
	jpeg_start_compress(&cinfo, TRUE);
	
  
	memset(yuvbuf, 0, img_width*3);

	
    zoom_out_val_w_f = (float)((float)width / (float)img_width);
    zoom_out_val_h_f = (float)((float)height / (float)img_height);
  
	ybase=pYUVBuffer;
	ubase=pYUVBuffer+width*height;
	while (cinfo.next_scanline < cinfo.image_height)
	{
		idx=0;
		for(i=0;i<img_width;i++)
		{	
			
			yuv_i = (uint_32)(i * zoom_out_val_w_f);
			yuv_j = (uint_32)(j * zoom_out_val_h_f);
			
			yuvbuf[idx++]=ybase[yuv_i + yuv_j * width];
			yuvbuf[idx++]=ubase[yuv_j/2 * width+(yuv_i/2)*2];
			yuvbuf[idx++]=ubase[yuv_j/2 * width+(yuv_i/2)*2+1];
		}  
		row_pointer[0] = yuvbuf;
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
		j++;
	}
	jpeg_finish_compress( &cinfo);
 
	jpeg_destroy_compress(&cinfo);
    if(0 != Helios_fclose(pJpegFile)) {
		JPEG_LOG("fs close failed\n");
		return -1;
	}
	return 0;	 

}

int yuv422_UYVY_to_jpg(char *filename, int img_width, int img_height, unsigned char *pYUVBuffer, int width, int height)
{
	HeliosFILE *pJpegFile = NULL;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[1];
	int i = 0, j = 0;
	unsigned char *ptr;
	unsigned char yuvbuf[width * 3];
	uint_32 yuv_i = 0,yuv_j = 0;
	
	float zoom_out_val_w_f = 0.0;
    float zoom_out_val_h_f = 0.0;
	int index = 0;

	cinfo.err = jpeg_std_error(&jerr);//用于错误信息
	jpeg_create_compress(&cinfo);  //初始化压缩对象
	
	if ((pJpegFile = Helios_fopen(filename, "wb")) == NULL)
	{	 
		return -1;
	}		
	jpeg_stdio_dest(&cinfo, pJpegFile);
	
	cinfo.image_width = img_width;//设置输入图片宽度
	cinfo.image_height = img_height;//设置图片高度
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_YCbCr;//设置输入图片的格式，支持RGB/YUV/YCC等等
	jpeg_set_defaults(&cinfo);//其它参数设置为默认的！
	jpeg_set_quality(&cinfo, 80, TRUE);//设置转化图片质量，范围0-100
	jpeg_start_compress(&cinfo, TRUE);
	
	memset(yuvbuf, 0, img_width*3);
	
	zoom_out_val_w_f = (float)((float)width / (float)img_width);
    zoom_out_val_h_f = (float)((float)height / (float)img_height);

	JPEG_LOG("zoom %f, %f\n", zoom_out_val_w_f, zoom_out_val_h_f);
	
	j = 1;
	
	ptr = pYUVBuffer;
	
	while (cinfo.next_scanline < cinfo.image_height) {
		index = 0; 
		for (i = 0; i < img_width; i ++){//输入的YUV图片格式为标准的YUV444格式，所以需要把YUV422转化成YUV444.
		
			yuv_i = (uint_32)(i * zoom_out_val_w_f);
		
			yuvbuf[index++] = *(ptr + (yuv_i*2)+1);
			yuvbuf[index++] = *(ptr + (yuv_i/2)*4);
			yuvbuf[index++] = *(ptr + (yuv_i/2)*4 + 2);

		}
		
		row_pointer[0] = yuvbuf;
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);//单行图片转换压缩
		yuv_j = (uint_32)(j * zoom_out_val_h_f);
		ptr = pYUVBuffer + yuv_j*width*2;
		j++;	
	}


	
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	
	if(0 != Helios_fclose(pJpegFile)) {
		JPEG_LOG("fs close failed\n");
		return -1;
	}
	return 0;
}


