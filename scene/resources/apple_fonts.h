#ifndef APPLE_NATIVE_FONTS_H
#define APPLE_NATIVE_FONTS_H
#if __cplusplus
extern "C" {
#endif

unsigned char* apple_get_font_data_for_font(const char* fontName, int *stream_size);


#if __cplusplus
}   // Extern C
#endif
#endif