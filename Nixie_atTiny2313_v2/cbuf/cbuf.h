/*
 *  userial -- a USB to I2C converter
 *
 *  Copyright (c) 2008-2009, Thomas Pircher <tehpeh@gmx.net>
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *****************************************************************************/
/**
 * \file
 *
 * Utility functions for circular buffer handling.
 *****************************************************************************/
#ifndef CBUF_H
#define CBUF_H
#include <stdint.h>
#include <stdbool.h>
#include "../main.h"

#define CBF_SIZE        16

/**
 * Data stricture for circular buffers
 *****************************************************************************/
typedef struct {
    uint8_t *pread;             //!< pointer to the next element to read
    uint8_t *pwrite;            //!< pointer to the next free element
    uint8_t data[CBF_SIZE];     //!< array containing the data
} cbf_t;

/**
 * Initialise a circular buffer
 *
 * \param cbf pointer to a cbf_t structure to be initialised
 *****************************************************************************/
void cbf_init(cbf_t *cbf);


/**
 * Test if the circular buffer is empty
 *
 * \param cbf pointer to the cbf_t structure
 * \return true if the buffer is empty, false otherwise
 *****************************************************************************/
static inline bool cbf_isempty(const cbf_t *cbf) {
    return cbf->pread == cbf->pwrite;
}

bool cbf_clear(cbf_t *cbf);


/**
 * Test if the circular buffer is full
 *
 * \param cbf pointer to the cbf_t structure
 * \return true if the buffer is full, false otherwise
 *****************************************************************************/
static inline bool cbf_isfull(const cbf_t *cbf) {
    return (cbf->pwrite == cbf->pread - 1 ||
        (cbf->pread == cbf->data && cbf->pwrite == cbf->data + CBF_SIZE - 1));
}


/**
 * Append one byte to the circular buffer
 *
 * \param cbf pointer to the cbf_t structure
 * \param data data byte to be appended to the circular buffer
 *****************************************************************************/
void cbf_put(cbf_t *cbf, uint8_t data);


/**
 * Retrieve one byte from the circular buffer
 *
 * \param cbf pointer to the cbf_t structure
 * \return the first data from the circular buffer
 *****************************************************************************/
uint8_t cbf_get(cbf_t *cbf);

/**
 * Get the first data from the buffer without discarding it.
 *
 * \param cbf pointer to the cbf_t structure
 * \return the first data from the circular buffer
 *****************************************************************************/
uint8_t cbf_peek(const cbf_t *cbf);


/**
 * Find a character in a circular buffer.
 *
 * \param cbf pointer to the cbf_t structure
 * \param c the character to find
 * \return the position of the character:
 *         0 means the first character,
 *        -1 means the character is not in the circular buffer.
 *****************************************************************************/
int cbf_find(const cbf_t *cbf, uint8_t c);


void cbf_ToStr(cbf_t *Buffer, uint8_t *output);


#endif // !defined CBUF_H
