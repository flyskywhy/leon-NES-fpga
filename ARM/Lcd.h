#ifndef __LCD_H__
#define __LCD_H__

void MoveViewPort(int depth);
void Test_LcdMono(void);
void Test_LcdG4(void);
void Test_LcdG16(void);
void Test_LcdColor(void);

void LcdColor256_Bmp( unsigned char bmp[] ) ;
void LcdG16_Bmp( unsigned char bmp[] ) ;
void LcdG16_Bmp_Overturn( unsigned char bmp[] ) ;

#endif /*__LCD_H__*/
