#ifndef ARINC_CANIF_H
#define ARINC_CANIF_H

/***************** return code *****************
#define A825_SUCCESS                  0
#define A825_NO_DATA                 -1
#define A825_ERR_TIMEOUT             -2
#define A825_ERR_INVALIDARG          -3
#define A825_CHANNEL_UNAVAILABLE     -4
#define A825_RESOURCE_UNAVAILABLE    -5
#define A825_UNKNOWN_ERROR           -6

****************** cna type ********************
enum a825_CanType
{
    A825_CAN = 0,
    A825_CANFD = 1
};

****************** send mode *******************
enum A825_SendMode
{
    A825_SEND_ASYNC = 0,
    A825_SEND_SYNC = 1
};

******************* receive mode ***************
enum A825_ReceiveMode
{
    A825_RECEIVE_NONBLOCK = 0,
    A825_RECEIVE_BLOCK = 1
};
******************/

typedef struct
{
    int type; //0: class can; 1: can fd
    struct
    {
        int bitrate;
        int tq;
        int phase1;
        int phase2;
        int sjw;
        int prop;
        int prescaler;
    } prop[2];
    int driver_mode; //0: normal mode; 1: monitor mode
} ChannelInfo;

/**
 *
 * @brief Opens a CAN channel (circuit) and returns a handle which is used
 * in subsequent calls to send, receive or close functions.
 *
 *
 * Channel numbering is dependent on the installed hardware. The first channel
 * always has number 0.
 *
 * For example,
 *
 * If you have a single LAPcan, the channels are numbered 0 and 1.
 *
 * If you have a USBcan Professional, the channels are numbered 0-1
 *     according to the labels on the cables.
 *
 * The virtual channels come after all physical channels.
 *
 * If you are using multiple threads, note that the returned handle is usable
 * only in the context of the thread that created it. That is, you must call
 * Open_channel() in each of the threads in your application that uses the
 * CAN bus. You can open the same channel from multiple threads, but you must
 * call Open_channel() once per thread.
 *
 * If you are using the same channel via multiple handles, note that the
 * default behaviour is that the different handles will "hear" each other just as
 * if each handle referred to a channel of its own. If you open, say, channel 0
 * from thread A and thread B and then send a message from thread A, it will be
 * "received" by thread B.
 *
 * @note The handle returned may be zero which is perfectly valid.
 *
 * @param[in]  ChId  The number of the channel. Channel numbering is hardware dependent.
 * @param[in]  info  Pointer to a ChannelInfo structure containing the configuration details for 
 *                   the can channel
 * @param[out] hnd_ptr  a handle to the opened circuit
 * @return Return 0 on success, or a negative error code on failure.
 *
 */
typedef int (* Open_channel_ptr) (int ChId, ChannelInfo *info, int *hnd_ptr);

/**
 * @brief Closes the channel associated with the handle. If no other threads
 * are using the CAN circuit, it is taken off bus. The handle can not be
 * used for further references to the channel, so any variable containing
 * it should be zeroed.
 *
 *
 * @param[in]  hnd  An open handle to a CAN channel.
 *
 * @return Return 0 on success, or a negative error code on failure.
 */
typedef int (* Close_channel_ptr) (int hnd);

/**
 *
 *
 * @brief This function sends a CAN or CANFD message. If the Sync mode is used, it will be returns when the message has been
 * successfully transmitted, or the timeout expires. If the Async mode is used, the call returns immediately after queuing
 * the message to the driver so the message has not necessarily been transmitted.
 *
 * If you are using the same channel via multiple handles, note that the
 * default behaviour is that the different handles will "hear" each other just as
 * if each handle referred to a channel of its own. If you open, say, channel 0
 * from thread A and thread B and then send a message from thread A, it will be
 * "received" by thread B.
 *
 * @param[in]  mode         The send mode (0 --- Async mode; 1 --- Sync mode) If the Async mode is used
 *                          need to confirm that the data in the buffer has been sent completely by call
 *                          the function of Check_Inbuf_Available().
 * @param[in]  hnd          A handle to an open CAN circuit.
 * @param[in]  messageid    The identifier of the CAN message to send.
 * @param[in]  data         A pointer to the message data, or \c NULL.
 * @param[in]  dlc          The length of the message in bytes.<br>
 *                          For Classic CAN dlc can be at most 8, unless \ref
 *                          canOPEN_ACCEPT_LARGE_DLC is used.<br> For CAN FD dlc
 *                          can be one of the following 0-8, 12, 16, 20, 24, 32,
 *                          48, 64.
 * @param[in] timeout    The timeout, in milliseconds. 0xFFFFFFFF gives an
 *                       infinite timeout.
 * @return Return 0 on success, or a negative error code on failure.
 */
typedef int (* send_frame_ptr) (int mode, int hnd, long messageid, unsigned char *data, unsigned int dlc, unsigned long timeout);

/**
 *
 * @brief Reads a message from the receive buffer. For the BLOCK mode, if no message is available, the
 * function waits until a message arrives or a timeout occurs. However, for the NONBLOCK mode, if no 
 * message is available, the function returns immediately with return code.
 *
 * If you are using the same channel via multiple handles, note that the
 * default behaviour is that the different handles will "hear" each other just as
 * if each handle referred to a channel of its own. If you open, say, channel 0
 * from thread A and thread B and then send a message from thread A, it will be
 * "received" by thread B.
 *
 * @param[in]   mode            The receive mode (0 --- NONBLOCK mode; 1 --- BLOCK mode).
 * @param[in]   hnd             A handle to an open circuit.
 * @param[out]  messageid       Pointer to a buffer which receives the CAN identifier.
 *                              This buffer will only get the identifier. To determine
 *                              whether this identifier was standard (11-bit) or extended
 *                              (29-bit), and/or whether it was remote or not, or if it
 *                              was an error frame, examine the contents of the flag
 *                              argument.
 * @param[out]  data            Pointer to the buffer which receives the message data.
 *                              This buffer must be large enough (i.e. 8 bytes for
 *                              classic CAN and up to 64 bytes for CAN FD).
 * @param[out]  dlc             Pointer to a buffer which receives the message length.
 * @param[out] timestamp        Pointer to a buffer which receives the message time stamp.
 *
 * @param[in]  timeout          If no message is immediately available, this parameter
 *                              gives the number of milliseconds to wait for a message
 *                              before returning. 0xFFFFFFFF gives an infinite timeout.
 * @return Return 0 on success, or a negative error code on failure.
 */
typedef int (* recv_frame_ptr) (int mode, int hnd, long *messageid, unsigned char *data, unsigned int *dlc, unsigned long *timestamp, unsigned long timesout);

/**
 *
 * @brief Waits until all CAN messages for the specified handle are sent, or the
 * timeout period expires.
 *
 * @param[in]  hnd       A handle to an open CAN circuit.
 * @param[in]  timeout   The timeout in milliseconds. 0xFFFFFFFF gives an
 *                       infinite timeout.
 * @return Return 0 on success, or a negative error code on failure.
 */
typedef int (* Check_Inbuf_Available_ptr) (int hnd, int timeout);

/**
 *
 * @brief The function interfaces structure Type.
 *
 */
typedef struct
{
    Open_channel_ptr Open_channel;
    Close_channel_ptr Close_channel;
    send_frame_ptr send_frame;
    recv_frame_ptr recv_frame;
    Check_Inbuf_Available_ptr Check_Inbuf_Available;
} Arinc_Canif_Type;



/**
 *
 * @brief The function interfaces structure dedlaration.
 *
 */
extern const Arinc_Canif_Type Arinc_Canif;
#endif