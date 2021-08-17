#ifndef JPEGOPERATION_H
#define JPEGOPERATION_H


#ifdef __cplusplus
extern "C" {
#endif

#define uint_32 unsigned int
#define uint_8 unsigned char
#define uint_16 unsigned short


typedef struct _rgb_struct{
	int width;
	int height;
	void *buf;
} rgb_struct;

void rgb5652bgr888(short *src, char *dest, int size);


int  JPEG2RGB565(const char* jpeg_file, rgb_struct *rgb565);

int RGB5652JPEG(const char* jpeg_file, unsigned char* rgb_buffer, int width, int height, int quality);

int yuv420_to_jpg(char *filename, int width, int height, unsigned char *pYUVBuffer);

int yuv420_nv12_to_rgb565_zoom_out(uint_16 *pRgbData, uint_32 rgbWidth, uint_32 rgbHeight, uint_8 *pYuvData, uint_32 yuvWidth, uint_32 yuvHeight);

int yuv420_nv12_to_rgb888(uint_8 *pRgb888Data, uint_32 rgbWidth, uint_32 rgbHeight, uint_8 *pYuvData, uint_32 yuvWidth, uint_32 yuvHeight);

int yuv420_NV12_to_jpg(char *filename, int img_width, int img_height, unsigned char *pYUVBuffer, int width, int height);

#ifdef __cplusplus
}
#endif


#endif

