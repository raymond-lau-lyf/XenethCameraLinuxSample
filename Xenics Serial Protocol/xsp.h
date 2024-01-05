/**
 *  @file       xsp.h
 *  @brief      Open source implementation of the Xenics serial protocol, XSP in short.
 *              The protocol is used in Xenics' branded cameras to allow transfer of register 
 *              settings, calibration data, overlays ... over serial transport.
 *
 *
 *  @version    0.1alpha
 *  @author     Joachim Stynen
 */
#ifndef _XENICS_LIBXSP_H
#define _XENICS_LIBXSP_H

#ifdef _WIN32
    #ifdef XSPEXPORTDLL
    #define XSPDECLSPEC __declspec(dllexport)
    #else
    #define XSPDECLSPEC __declspec(dllimport)
    #endif
    #define XSPCALLCV   __stdcall
#else
    #define XSPDECLSPEC
    #define XSPCALLCV
#endif

#ifdef __cplusplus
extern "C"
{
#endif

typedef unsigned char  byte;        /**< Unsigned char, 8bit            */
typedef unsigned short word;        /**< Unsigned short, 16bit          */
typedef unsigned long  dword;       /**< Unsigned long, 32bit           */
typedef byte boole;                 /**< Byte to represent a bool type. */

/**
 *  This enum can be used for convenience, current subscribed vendors identifiers are kept here.
 */
typedef enum
{
    xspVendorXenics = 0x1b21         /**< Xenics' vendor id              */

} xspVendors;

/**
 *  Standard ASCII codes used in the protocol.
 */
typedef enum
{
    STX = 0x02,     /**< Start of Text          */
     CR = 0x0D,     /**< Carriage Return        */
    DLE = 0x10,     /**< Data Line Escape       */
    ETB = 0x17      /**< End of Transmit Block  */

} xspAsciiCode;

/**
 *  Message identifier used in the binary messages structure.
 */
typedef enum
{
    xspMessageWrite = 0x81, /**< Identifies a message for writing.                      */
    xspMessageRead  = 0x82, /**< Identifies a message for reading.                      */
    xspMessageAck   = 0x06, /**< Identifies an acknowledge response message.            */ 
    xspMessageNack  = 0x15, /**< Identifies a negative acknowledge response message.    */

} xspMessageIdentifier;

/**
 *  Error string definitions.
 */
#define XSP_ERROR_OK_STR                "No error occurred"
#define XSP_ERROR_CRC_STR               "An invalid CRC was calculated from the received data"
#define XSP_ERROR_MESSAGE_TYPE_STR      "The supplied message type does not exist"
#define XSP_ERROR_MESSAGE_ETB_STR       "No end of transmission block received"
#define XSP_ERROR_ADDRESS_ERROR_STR     "Address does not exist or is invalid"
#define XSP_ERROR_TIMEOUT_STR           "Timeout received"
#define XSP_ERROR_PAYLOAD_LENGTH_STR    "Invalid payload length supplied, maximum is 1024 bytes"
#define XSP_ERROR_START_OF_MESSAGE      "No start of message"
#define XSP_ERROR_UNKNOWN               "An unknown error occurred"

/**
 *  Error codes returned by the device.
 */
typedef enum
{
    xspErrorOK                   = 0,       /**< No error occurred.                                     */
    xspErrorCRC                  = 1 << 0,  /**< An invalid CRC was calculated from the received data.  */
    xspErrorMessageType          = 1 << 1,  /**< The supplied message type does not exist.              */
    xspErrorEndTransmissionBlock = 1 << 2,  /**< No end of transmission block received.                 */
    xspErrorAddressError         = 1 << 3,  /**< Address does not exist or is invalid.                  */
    xspErrorTimeout              = 1 << 4,  /**< Timeout received.                                      */
    xspErrorPayloadLength        = 1 << 5,  /**< Invalid payload length supplied, maximum is 1024.      */
    xspErrorStartOfMessage       = 1 << 6   /**< No start of message exists                             */

} xspErrorCode;

#pragma pack(push, 1)

/**
 *  Binary message container.
 *  
 *  To avoid synchronization issues following notice has to be taken into account.
 *  If any of the following characters (STX (0x02), ETB (0x17), DLE (0x10), CR (0x0D)) appear
 *  in the Packet Tag, Message ID, Payload length, Payload and CRC, then a DLE character
 *  must precede this character. This stuffing does not influence the payload size value.
 *  The stuffing is removed at the receiver before the character is evaluated.
 *
 *  This structure contains the binary message without stuffing. 
 *  A few utility methods are available to (un)stuff the messages into a buffer 
 *
 *      @sa xspPackMessage
 *      @sa xspUnpackMessage
 *
 */
typedef struct 
{
    const byte startOfMessage;              /**< Start of message. (1 byte)                                                                         */
    byte packetTag;                         /**< Can be used by sender to identify replies. The camera will copy this tag in its reply. (1 byte)    */
    byte messageId;                         /**< Message identifier. (1 byte) @sa xspMessageIdentifier                                              */
    word payloadLength;                     /**< Payload length. (2 bytes)                                                                          */
    byte * const payload;                   /**< Payload contents, depending on message id (0 < payloadLength < 1024 bytes)                         */
    byte crc;                               /**< CRC8 checksum, calculated over packetTag, messageId, payloadLength, payload. (1 byte) @sa xspCRC8  */
    const byte endOfMessage;                /**< End of message. (1 byte)                                                                           */

} xspBinaryMessageContainer;

/**
 *  These registers should always be implemented in any camera that complies with the XSP standard.
 *  Starting from register address 0x0, 48 bytes are read from the camera which can be used to represent this structure.
 */
typedef struct 
{
    word productId;         /**< The camera's product identifier, e.g.: 0xf020                                              */
    word vendorId;          /**< The camera's vendor identifier, Xenics' identifier is 0x1b21                               */
    dword serialNumber;     /**< Serial number of the camera.                                                               */
    char cameraName[32];    /**< Zero terminated ASCII string of max 32 characters, including the termination character.    */
    struct  
    {
        byte revision;
        byte version;
        word reserved;
    } xspVersion;           /**< XSP Version number. byte 0 is revision, byte 1 version. bytes 2 and 3 are unused.              */
    struct  
    {
        word buildNr;
        byte revision;
        byte version;
    } fwVersion;            /**< Firmware version number. byte 0 and 1 is the build number, byte 2 revision and byte 3 version  */

} xspCommonRegisters;

#pragma pack(pop)

/**
 *  Create binary message structures, once finished the structure should be deleted using xspDeleteMessage.
 *  To save yourself an extra allocation when working with big payloads, call the function with a zero payload pointer and payloadLength set to the desired size.
 *  The function will return with an allocated payload section with nothing inside. Make sure the CRC is calculated again after data manipulation!
 *  @sa xspDeleteMessage
 *
 *  @param  tag             Can be used by the sender to identify replies. The camera will copy the contents in its reply. (optional, leave zero when unwanted)
 *  @param  messageId       Message identifier. @sa xspMessageIdentifier
 *  @param  payload         The payload, aka the data that will be encoded in the message.
 *  @param  length          The length of the payload.
 *
 *  @retval xspBinaryMessageContainer   A pointer to an allocated binary message structure.
 *
 */
XSPDECLSPEC xspBinaryMessageContainer * XSPCALLCV xspCreateMessage(byte tag, xspMessageIdentifier messageId, byte const * const payload, word length);

/**
 *  Delete a previously allocated binary message structure to avoid memory leakage.
 *  Treat these (de)allocation functions like you would treat malloc/free or new/delete. 
 *  @sa xspCreateMessage
 *
 *  @param  container       The container that will be freed, previously allocated by xspCreateMessage.
 *
 *  @retval void            This function returns nothing.
 *
 */
XSPDECLSPEC void XSPCALLCV xspDeleteMessage(xspBinaryMessageContainer *container);

/**
 *  Pack the message and copy the result into an allocated buffer supplied by the user.
 *  If the buffer is too small to fit the resulting raw message the function will return false with the adjusted bufferSize.
 */
XSPDECLSPEC boole XSPCALLCV xspPackMessage(xspBinaryMessageContainer const * const xbmc, byte * const buffer, dword * const bufferSize);

/**
 *  Unpacks the buffer into a xspBinaryMessageContainer allocated by xspCreateMessage, destroy the returned container with xspDeleteMessage!
 *  @sa xspDeleteMessage
 *  
 *  @param  buffer          User allocated buffer that holds the xsp packet.
 *  @param  bufferSize      The size of the buffer.
 *
 *  @retval xspBinaryMessageContainer   A pointer to an unpacked message. When null the message was invalid.
 *
 */
XSPDECLSPEC xspBinaryMessageContainer * XSPCALLCV xspUnpackMessage(byte const * const buffer, dword size);

/**
 *  Calculate CRC8 from message.
 *  @sa xspAddCRC8
 *  @sa xspCRC8FromBMC
 *
 *  @param  message         The data to calculate the CRC from.
 *  @param  length          The length of the data that needs to be calculated.
 *
 *  @retval byte            The result of the CRC8 calculation.
 *
 */
XSPDECLSPEC byte XSPCALLCV xspCRC8(byte const * const message, dword length);

/**
 *  Calculate CRC8 from message starting with the supplied crc.
 *  @sa xspCRC8
 *  @sa xspCRC8FromBMC
 *
 *  @param  message         The data to calculate the CRC from.
 *  @param  length          The length of the data that needs to be calculated.
 *  @param  crc             Start CRC value, when calculating a fresh crc this should be 0xFF
 *
 *  @retval byte            The result of the CRC8 calculation.
 *
 */
XSPDECLSPEC byte XSPCALLCV xspAddCRC8(byte const * const message, dword length, byte crc);

/**
 *  xspBinaryMessageContainer CRC8 calculation function.
 *  The calculated crc will be returned and also assigned to the supplied xspBinaryMessageContainer crc field.
 *  @sa xspCRC8
 *  @sa xspAddCRC8
 *
 *  @param  container       Message container from which that crc should be calculated.
 *
 *  @retval byte             The result of the CRC8 calculation.
 *
 */
XSPDECLSPEC byte XSPCALLCV xspCRC8FromBMC(xspBinaryMessageContainer const * const container);

/**
 *  Check the validity of a supplied xspBinaryMessageContainer.
 */
XSPDECLSPEC xspErrorCode XSPCALLCV xspValidateBMC(xspBinaryMessageContainer const * const container);

/**
 *  Creates a message for reading register values.
 *
 *  @param  tag             Tag is an optional argument that can be used to identify replies.
 *  @param  address         The start address.
 *  @param  nrOfDWords      The number of dwords to be read.
 *
 *  @retval xspBinaryMessageContainer  If the supplied data was valid the return value points to an allocated xspBinaryMessageContainer, don't forget to destroy it!
 *                                     @sa xspDeleteMessage
 */
XSPDECLSPEC xspBinaryMessageContainer * XSPCALLCV xspCreateReadMessage(byte tag, dword address, dword nrOfDWords);

/**
 *  Creates a message for writing register values.
 *
 *  @param  tag             Tag is an optional argument that can be used to identify replies.
 *  @param  address         The start address.
 *  @param  values          The values to be written.
 *  @param  nrOfDWords      The number of dwords to be written.
 *
 *  @retval xspBinaryMessageContainer   If the supplied data was valid the return value points to an allocated xspBinaryMessageContainer, don't forget to destroy it!
 *                                      @sa xspDeleteMessage
 */
XSPDECLSPEC xspBinaryMessageContainer * XSPCALLCV xspCreateWriteMessage(byte tag, dword address, dword const * const values, word nrOfDWords);

/**
 *  Retrieve the last error code.
 *
 *  @retval xspErrorCode    The last set error code
 */
XSPDECLSPEC xspErrorCode XSPCALLCV xspLastErrorCode();

/**
 *  Convert an xspErrorCode to a readable string.
 *
 *  @param  code            The error code to convert
 *  @param  string          The destination string, when null the size argument is set with the correct size.
 *  @param  size            The size of the supplied buffer, this parameters is also used to return the correct size when the string is null or the supplied initial size is too small.
 *
 *  @retval boole           When successful returns 1 when failed returns 0. This function fails when the string is null or the supplied size is too small.
 */
XSPDECLSPEC boole XSPCALLCV xspErrorText(xspErrorCode code, char * const string, dword * size);

#ifdef __cplusplus
}; // extern "C"
#endif

#endif // _XENICS_LIBXSP_H
