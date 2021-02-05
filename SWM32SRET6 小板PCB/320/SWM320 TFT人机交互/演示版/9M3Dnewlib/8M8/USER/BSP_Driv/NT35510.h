#ifndef __NT35510_H__
#define __NT35510_H__

void TFTInit(void);
void TFTSetCursor(uint16_t x, uint16_t y);
void TFTDrawPoint(uint16_t x, uint16_t y, uint16_t rgb);
void TFTClear(uint16_t rgb);

#endif //__NT35510_H__
