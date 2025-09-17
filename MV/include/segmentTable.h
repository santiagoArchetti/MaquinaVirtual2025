#ifndef SEGMENT_TABLE_H
#define SEGMENT_TABLE_H

#include <stdint.h>

#define SEGMENT_TABLE_SIZE 8

typedef struct {
    uint32_t segment[SEGMENT_TABLE_SIZE];  // 32 bits: [base(16) | length(16)]
    int position;   // inicializar en main/init
} SegmentTable;

extern SegmentTable segmentTable;

void writeBaseSegment(void);  
void getSegmentRange(int segment, uint16_t* valueBase, uint16_t* valueLength);
void setSegmentDataLength(uint16_t dataLength);
void initSegmentTable(void);

#endif
