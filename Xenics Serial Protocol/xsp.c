#include "xsp.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 *  @defgroup error Error Handling
 *  @{
 */

// Thread local storage class specifier
#if defined(__GNUC__)
    #define XSPTLS  __thread
#elif defined(_MSC_VER)
    #define XSPTLS  __declspec(thread)
#endif

xspErrorCode XSPTLS g_lastErrorCode = xspErrorOK;

xspErrorCode XSPCALLCV xspLastErrorCode()
{
    return g_lastErrorCode;
}

boole XSPCALLCV xspErrorText(xspErrorCode code, char * const string, dword * size)
{
    char const * _tbuffer = 0;
    dword strlength = 0;

    switch(code)
    {
    case xspErrorOK:
        _tbuffer = XSP_ERROR_OK_STR;
        break;
    case xspErrorCRC:
        _tbuffer = XSP_ERROR_CRC_STR;
        break;
    case xspErrorMessageType:
        _tbuffer = XSP_ERROR_MESSAGE_TYPE_STR;
        break;
    case xspErrorEndTransmissionBlock:
        _tbuffer = XSP_ERROR_MESSAGE_ETB_STR;
        break;
    case xspErrorAddressError:
        _tbuffer = XSP_ERROR_ADDRESS_ERROR_STR;
        break;
    case xspErrorTimeout:
        _tbuffer = XSP_ERROR_TIMEOUT_STR;
        break;
    case xspErrorPayloadLength:
        _tbuffer = XSP_ERROR_PAYLOAD_LENGTH_STR;
        break;
    case xspErrorStartOfMessage:
        _tbuffer = XSP_ERROR_START_OF_MESSAGE;
        break;
    default:
        _tbuffer = XSP_ERROR_UNKNOWN;
    }

    strlength = strlen(_tbuffer) + 1;
    if (!string || strlength > *size)
    {
        *size = strlength;
        return 0;
    }

    strncpy(string, _tbuffer, strlength);

    return 1;
}

/**
 *  @}
 *  @defgroup PrivateFunc Private helper functions
 */
//!@{
/**
 *  Used to determine the amount of bytes needed.
 */
word countStuffBytes(byte const * const bytePtr, word len)
{
    word x, count = 0;
    if (bytePtr && len > 0)
    {
        for(x = 0; x < len; x++)
        {
            switch(bytePtr[x])
            {
            case STX: case ETB: case DLE: case CR:
                count++;
                break;
            }
        }
    }
    
    return count;
}

/**
 *  Used to define the buffer size needed to hold the contents of an xspBinaryMessageContainer with stuffing.
 */
word countStuffBytesBMC(xspBinaryMessageContainer const * const xbmc)
{
    if (xbmc)
    {
        word stuffBytes = 0;

        stuffBytes += countStuffBytes((byte *)&xbmc->packetTag, sizeof(xbmc->packetTag));
        stuffBytes += countStuffBytes((byte *)&xbmc->messageId, sizeof(xbmc->messageId));
        stuffBytes += countStuffBytes((byte *)&xbmc->payloadLength, sizeof(xbmc->payloadLength));
        stuffBytes += countStuffBytes(xbmc->payload, xbmc->payloadLength);
        stuffBytes += countStuffBytes((byte *)&xbmc->crc, sizeof(xbmc->crc));

        return sizeof(xspBinaryMessageContainer) - sizeof(byte *) + xbmc->payloadLength + stuffBytes;
    }

    return 0;
}

/**
 *  1 is big, 0 is little :)
 */
boole endianess()
{
    union
    {
        byte a[2];
        word b;
    } e = { 0x01, 0x02 };

    return (e.a[0] == 2);
}

word swp_w(word v)
{
    union
    {
        byte a[2];
        word b;
    } u1, u2;

    u1.b = v;
    u2.a[0] = u1.a[1];
    u2.a[1] = u1.a[0];
    return u2.b;
}

dword swp_dw(dword v)
{
    union
    {
        byte a[4];
        dword b;
    } u1, u2;

    u1.b = v;
    u2.a[0] = u1.a[3];
    u2.a[1] = u1.a[2];
    u2.a[2] = u1.a[1];
    u2.a[3] = u1.a[0];
    return u2.b;
}

/**
 *  convert a word from host to serial byte order
 */
word htos_w(word v)
{
    if(!endianess()) // little
        return v;

    return swp_w(v);
}

/**
 *  convert a word from serial to host byte order
 */
word stoh_w(word v)
{
    if(!endianess()) // little
        return v;

    return swp_w(v);
}

/**
 *  convert a double word from host to serial byte order
 */
dword htos_dw(dword v)
{
    if(!endianess()) // little
        return v;

    return swp_dw(v);
}

/**
 *  convert a double word from serial to host byte order
 */
dword stoh_dw(dword v)
{
    if(!endianess()) // little
        return v;

    return swp_dw(v);
}

/**
 *  Make sure when using this function, there is enough room for stuffing. use countStuffBytesBMC and countStuffBytes to determine the size needed.
 *  The return value is the exact numer of bytes that were copied into the dst buffer, so basically it should be equal to len.
 */
dword memcpy_stuff(byte * const dst, byte const * const src, dword len)
{
    dword x = 0, offset = 0;

    if (src && dst && len > 0)
    {
        for(x = 0; x < len; x++)
        {
            switch(src[x])
            {
            case STX: case ETB: case DLE: case CR:
                dst[x + offset] = DLE;
                offset++;
                break;
            }

            dst[x + offset] = src[x];
        }
    }
    
    return x + offset; // bytes copied (inc. stuff bytes)
}

/**
 *  Create a buffer at least as big as the src, the value returned gives knowledge about how many bytes are left after stripping the stuff bytes.
 */
dword memcpy_unstuff(byte * const dst, byte const * const src, dword len)
{
    dword x = 0, offset = 0;

    if(src && dst && len > 0)
    {
        for(x = 0; x < len; x++)
        {
            if(src[x] == DLE)
            {
                offset++;
            }
            else
            {
                dst[x - offset] = src[x];
            }
        }
    }

    return x - offset;
}
//!@}

/*
 *  Implemented public functions
 */

xspBinaryMessageContainer * XSPCALLCV xspCreateMessage(byte tag, xspMessageIdentifier messageId, byte const * const payload, word payloadLength)
{   
    xspBinaryMessageContainer *container = (xspBinaryMessageContainer *)malloc(sizeof(xspBinaryMessageContainer));
    xspBinaryMessageContainer _templateContainer = 
    {   // struct initializer
        STX, 
        tag, 
        messageId, 
        payloadLength, 
        payloadLength > 0 ? (byte *)malloc(payloadLength) : 0,
        0,
        ETB 
    };
    
    if(container)
    {
        // copy payload
        if (payload && _templateContainer.payload)
            memcpy(_templateContainer.payload, payload, payloadLength);

        // calculate crc
        _templateContainer.crc = xspCRC8FromBMC(&_templateContainer);

        // copy structure
        memcpy(container, &_templateContainer, sizeof(xspBinaryMessageContainer));
    }

    if (xspValidateBMC(container) != xspErrorOK)
    {
        xspDeleteMessage(container);
        container = 0;
    }

    return container;
}

void XSPCALLCV xspDeleteMessage(xspBinaryMessageContainer * container)
{
    if(container)
    {
        if(container->payload)
        {
            free(container->payload);
        }

        free(container);
    }
}

byte XSPCALLCV xspCRC8(byte const * const message, dword length)
{
    return xspAddCRC8(message, length, 0xFF); // CRC is calculated starting from crc 0xFF
}

byte XSPCALLCV xspAddCRC8(byte const * const message, dword length, byte crc)
{   
    dword i, j; 
    word poly = 0x131;

    for (i = 0; i < length; i++)
    {
        crc = crc ^ message[i];
        for(j = 1; j <= 7; j++)
            if(crc & 128)
                crc = ((crc & 0x7f) * 2) ^ (poly & 0xff);
            else
                crc = ((crc & 0x7f) * 2);
    }

    return crc;
}

byte XSPCALLCV xspCRC8FromBMC( xspBinaryMessageContainer const * const container )
{
    byte crc = 0;

    if (!container)
        return 0;

    crc = xspCRC8(((byte *)container) + 1, 4); // tag, id, len. offset of 1 to skip stx
    crc = xspAddCRC8(container->payload, container->payloadLength, crc); // payload
    
    return crc;
}

boole XSPCALLCV xspPackMessage(xspBinaryMessageContainer const * const xbmc, byte * const buffer, dword * const bufferSize)
{
    if (xbmc)
    {
        dword 
            offset = 0,
            x = 0,
            size = countStuffBytesBMC(xbmc), // calculate the room needed when data is *stuffed*
            payloadLength = 0;

        if (!buffer || *bufferSize < size)
        {
            *bufferSize = size;
            return 0;
        }
        else
            *bufferSize = size; // we assign the calculated size to the bufferSize out argument anyway

        // endianess
        payloadLength = htos_w(xbmc->payloadLength);

        // copy buffer
        memcpy(buffer + offset++, &xbmc->startOfMessage, 1);
        offset += memcpy_stuff(buffer + offset, &xbmc->packetTag, sizeof(byte));
        offset += memcpy_stuff(buffer + offset, &xbmc->messageId, sizeof(byte));
        offset += memcpy_stuff(buffer + offset, (byte *)&payloadLength, sizeof(word));
        offset += memcpy_stuff(buffer + offset, xbmc->payload, xbmc->payloadLength);
        offset += memcpy_stuff(buffer + offset, (byte *)&xbmc->crc, sizeof(xbmc->crc));
        memcpy(buffer + offset, &xbmc->endOfMessage, 1);
   
        return 1;
    }

    return 0;
}

xspBinaryMessageContainer * XSPCALLCV xspUnpackMessage(byte const * const buffer, dword size)
{
    xspBinaryMessageContainer *container = 0;

    if (buffer && size > 0)
    {
        byte tag, id, crc;
        word len;
        byte *tempBuffer = (byte*)malloc(size);
        
        if (tempBuffer)
        {
            // unstuff
            memcpy_unstuff(tempBuffer, buffer, size);
            
            // parse values
            memcpy(&tag, tempBuffer + 1, 1);
            memcpy(&id,  tempBuffer + 2, 1);
            memcpy(&len, tempBuffer + 3, 2);
            memcpy(&crc, tempBuffer + 5 + len, 1);
            
            // create message
            container = xspCreateMessage(tag, (xspMessageIdentifier)id, tempBuffer + 5, len);
            
            // validate message
            if (xspValidateBMC(container) != xspErrorOK)
            {
                // this was not a valid xsp message.
                xspDeleteMessage(container);
                container = 0;
            }
            
            free(tempBuffer);
        }
    }

    return container;
}

xspErrorCode XSPCALLCV xspValidateBMC(xspBinaryMessageContainer const * const container)
{
    if(!container)
    {
        g_lastErrorCode = xspErrorAddressError;
        return g_lastErrorCode;
    }

    // test for startOfMessage & endOfMessage
    if (container->startOfMessage != STX)
    {
        g_lastErrorCode = xspErrorStartOfMessage;
        return g_lastErrorCode;
    }
    
    if (container->endOfMessage != ETB)
    {
        g_lastErrorCode = xspErrorEndTransmissionBlock;
        return g_lastErrorCode;
    }

    // test for valid messageId
    switch(container->messageId)
    {
    case xspMessageRead:
    case xspMessageWrite:
    case xspMessageAck:
    case xspMessageNack:
        break;

    default:
        g_lastErrorCode = xspErrorMessageType;
        return g_lastErrorCode;
    }

    // test for payload length
    if(!(container->payloadLength >= 0 && container->payloadLength < 1024))
    {
        g_lastErrorCode = xspErrorPayloadLength;
        return g_lastErrorCode;
    }

    if (xspCRC8FromBMC(container) != container->crc)
    {
        g_lastErrorCode = xspErrorCRC;
        return g_lastErrorCode;
    }

    g_lastErrorCode = xspErrorOK;
    return g_lastErrorCode;
}

xspBinaryMessageContainer * XSPCALLCV xspCreateReadMessage(byte tag, dword address, dword numberOfDWords)
{
    struct 
    {
        dword address;
        dword numberOfDWords;

    } readData = {0};

    if(numberOfDWords > 256)
    {
        g_lastErrorCode = xspErrorPayloadLength;
        return 0;
    }

    readData.address = address;
    readData.numberOfDWords = numberOfDWords;

    return xspCreateMessage(tag, xspMessageRead, (byte *)&readData, sizeof(readData));
}

xspBinaryMessageContainer * XSPCALLCV xspCreateWriteMessage(byte tag, dword address, dword const * const values, word nrOfDWords)
{
    xspBinaryMessageContainer *container = 0;

    if(values && nrOfDWords > 0)
    {
        word payloadLength = sizeof(dword) * (nrOfDWords + 1);
        container = xspCreateMessage(tag, xspMessageWrite, 0, payloadLength); // payload memory allocation happens here

        if (container)
        {
            memcpy(container->payload, &address, sizeof(dword));
            memcpy(container->payload + sizeof(dword), values, payloadLength - sizeof(dword));
            container->crc = xspCRC8FromBMC(container); // make sure crc is recalculated after modifications!
        }
    }

    if (xspValidateBMC(container) != xspErrorOK)
    {
        xspDeleteMessage(container);
        container = 0;
    }
    
    return container;
}
