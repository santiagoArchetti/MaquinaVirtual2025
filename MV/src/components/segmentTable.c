#include "../../include/segmentTable.h"
#include <stdio.h>

SegmentTable segmentTable;  // definicion global

// Inicializamos en 0 todos los segmentos
void initSegmentTable() {
    segmentTable.position = 0;
    for (int i = 0; i < SEGMENT_TABLE_SIZE; i++) {
        segmentTable.segment[i] = 0;
    }
}

// Funcion para escribir la base de un segmento
void writeBaseSegment() {
    if (segmentTable.position == 0) {
        // Primer segmento → base = 0 (no modificamos longitud)
        uint16_t length = (uint16_t)(segmentTable.segment[0] & 0xFFFF); //solo la parte baja (se parsea a 16 bits)
        segmentTable.segment[0] = ((uint32_t)0 << 16) | length;  //convierte 0 a 16 bits y lo une con la longitud
    } else {
        // Base = base + longitud del segmento anterior
        uint16_t prevBase, prevLength;
        getSegmentRange(segmentTable.position - 1, &prevBase, &prevLength);

        uint16_t newBase = prevBase + prevLength; 
        uint16_t length  = (uint16_t)(segmentTable.segment[segmentTable.position] & 0xFFFF); //solo la parte baja (se parsea a 16 bits)

        segmentTable.segment[segmentTable.position] = ((uint32_t)newBase << 16) | length;  //convierte la base a 16 bits y lo une con la longitud
    }
}

// Funcion para escribir la longitud de un segmento
void setSegmentDataLength(uint16_t dataLength) {
    if (segmentTable.position >= SEGMENT_TABLE_SIZE) {
        printf("Error: segment table full\n");
        return;
    }
    // Guardamos solo longitud en la parte baja (base todavia = 0)
    segmentTable.segment[segmentTable.position] = (uint32_t)dataLength & 0xFFFF;
    writeBaseSegment();
    segmentTable.position++;  // Avanzar a la siguiente entrada
}

// Consultar rango de un segmento
void getSegmentRange(int segment, uint16_t* valueBase, uint16_t* valueLength) {
    uint32_t entry = segmentTable.segment[segment];
    *valueBase   = (uint16_t)(entry >> 16);     // parte alta = base
    *valueLength = (uint16_t)(entry & 0xFFFF);  // parte baja = length
}
