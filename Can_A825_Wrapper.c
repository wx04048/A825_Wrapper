#include <canlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "Can_A825_Wrapper.h"

#define A825_SUCCESS                  0
#define A825_NO_DATA                 -1
#define A825_ERR_TIMEOUT             -2
#define A825_ERR_INVALIDARG          -3
#define A825_CHANNEL_UNAVAILABLE     -4
#define A825_RESOURCE_UNAVAILABLE    -5
#define A825_UNKNOWN_ERROR           -6


enum a825_CanType
{
    A825_CAN = 0,
    A825_CANFD = 1
};

enum A825_SendMode
{
    A825_SEND_ASYNC = 0,
    A825_SEND_SYNC = 1
};

enum A825_ReceiveMode
{
    A825_RECEIVE_NONBLOCK = 0,
    A825_RECEIVE_BLOCK = 1
};


typedef enum
{
    CHANNEL_CLOSE,
    CHANNEL_OPEN,
    CHANNEL_BUSY,
    CHANNEL_IDLE
} CanChannelStatus_Type;

typedef struct
{
    int channel_handle;
    int channel_status;
} Can_channel_type;

static void check(char *id, canStatus stat)
{
    if (stat != canOK) {
        char buf[50];
        buf[0] = '\0';
        canGetErrorText(stat, buf, sizeof(buf));
        printf("%s: failed, stat=%d (%s)\n", id, (int)stat, buf);
    }
}

static int get_channel_num (int *chanCount)
{
    canStatus stat;
    stat = canGetNumberOfChannels(chanCount);
    return stat;
}

int Open_channel_fun(int ChId, ChannelInfo *info, int *hnd_ptr)
{
    canHandle hnd;
    canStatus stat;
    int chanCount;

    *hnd_ptr = -1;

    canInitializeLibrary();

    if (canOK != get_channel_num(&chanCount))
    {
        perror("Get can channel info error\n");
        return canERR_NOTFOUND;
    }

    if (info == NULL || ChId >= chanCount)
    {
        return A825_ERR_INVALIDARG;
    }

    if (info->type == A825_CANFD)
    {
        printf("Open a can channel as CANFD\n");

        hnd = canOpenChannel(ChId, canOPEN_CAN_FD | canOPEN_REQUIRE_EXTENDED);
        *hnd_ptr = hnd;
        if (hnd < 0) {
            printf("canOpenChannel %d\n", ChId);
            check("", hnd);
            return A825_CHANNEL_UNAVAILABLE;
        }

        if (info->prop[0].bitrate != 0 && info->prop[1].bitrate != 0)
        {
            if ((info->prop[0].phase1 == 0 && info->prop[0].phase2 == 0) \
                || (info->prop[1].phase1 == 0 && info->prop[1].phase2 == 0))
            {
                stat = canSetBusParams(hnd, info->prop[0].bitrate, 0, 0, 0, 0, 0);
                check("canSetBusParams", stat);
                if (stat != canOK) {
                    Arinc_Canif.Close_channel(hnd);
                    return A825_CHANNEL_UNAVAILABLE;
                }
                stat = canSetBusParamsFd(hnd, info->prop[1].bitrate, 0, 0, 0);
                check("canSetBusParamsFd", stat);
                if (stat != canOK) {
                    Arinc_Canif.Close_channel(hnd);
                    return A825_CHANNEL_UNAVAILABLE;
                }
            }
            else
            {
                stat = canSetBusParams(hnd, info->prop[0].bitrate, info->prop[0].phase1 + info->prop[0].prop, info->prop[0].phase2, info->prop[0].sjw, 1, 0);
                check("canSetBusParams", stat);
                if (stat != canOK) {
                    Arinc_Canif.Close_channel(hnd);
                    return A825_CHANNEL_UNAVAILABLE;
                }
                stat = canSetBusParamsFd(hnd, info->prop[1].bitrate, info->prop[1].phase1 + info->prop[1].prop, info->prop[1].phase2, info->prop[1].sjw);
                check("canSetBusParamsFd", stat);
                if (stat != canOK) {
                    Arinc_Canif.Close_channel(hnd);
                    return A825_CHANNEL_UNAVAILABLE;
                }
            }
            stat = canSetBusOutputControl(hnd, info->driver_mode);
            check("canSetBusOutputControl", stat);
            if (stat != canOK) {
                Arinc_Canif.Close_channel(hnd);
                return A825_CHANNEL_UNAVAILABLE;
            }
            stat = canBusOn(hnd);
            check("canBusOn", stat);
            if (stat != canOK) {
                Arinc_Canif.Close_channel(hnd);
                return A825_CHANNEL_UNAVAILABLE;
            }  
        } 
        else
        {
            printf("info error\n");
            return A825_ERR_INVALIDARG;
        }
    }
    else if (info->type == A825_CAN)
    {
        printf("Open a can channel as can\n");
        hnd = canOpenChannel(ChId, canOPEN_REQUIRE_EXTENDED);
        *hnd_ptr = hnd;
        if (hnd < 0) {
            printf("canOpenChannel %d", ChId);
            check("", hnd);
            return A825_CHANNEL_UNAVAILABLE;
        }

        if (info->prop[0].bitrate != 0)
        {
            if (info->prop[0].phase1 == 0 && info->prop[0].phase2 == 0)
            {
                stat = canSetBusParams(hnd, info->prop[0].bitrate, 0, 0, 0, 0, 0);
                check("canSetBusParams", stat);
                if (stat != canOK) {
                    Arinc_Canif.Close_channel(hnd);
                    return A825_CHANNEL_UNAVAILABLE;
                }
            }
            else
            {
                stat = canSetBusParams(hnd, info->prop[0].bitrate, info->prop[0].phase1 + info->prop[0].prop, info->prop[0].phase2, info->prop[0].sjw, 1, 0);
                check("canSetBusParams", stat);
                if (stat != canOK) {
                    Arinc_Canif.Close_channel(hnd);
                    return A825_CHANNEL_UNAVAILABLE;
                }
            }

            stat = canSetBusOutputControl(hnd, info->driver_mode);
            check("canSetBusOutputControl", stat);
            if (stat != canOK) {
                Arinc_Canif.Close_channel(hnd);
                return A825_CHANNEL_UNAVAILABLE;
            }
            stat = canBusOn(hnd);
            check("canBusOn", stat);
            if (stat != canOK) {
                Arinc_Canif.Close_channel(hnd);
                return A825_CHANNEL_UNAVAILABLE;
            }  
        } 
        else
        {
            printf("info error\n");
            return A825_ERR_INVALIDARG;
        }
    }
    else
    {
        return A825_ERR_INVALIDARG;
    }

    return A825_SUCCESS;
}

int Close_channel_fun (int hnd)
{
    canStatus stat;
    stat = canClose(hnd);
    check("canClose", stat);
    if (stat != canOK)
    {
        return A825_CHANNEL_UNAVAILABLE;
    }
    return A825_SUCCESS;
}

int send_frame_fun (int mode, int hnd, long messageid, unsigned char *data, unsigned int dlc, unsigned long timeout)
{
    canStatus stat;
    int cap;
    if (data == NULL)
    {
        printf("Import data invalid\n");
        return A825_ERR_INVALIDARG;
    }

    stat = canGetHandleData(hnd, canCHANNELDATA_CHANNEL_FLAGS, &cap, sizeof(cap));
    check("canGetHandleData", stat);
    if (stat != canOK)
    {
        Arinc_Canif.Close_channel(hnd);
        return A825_CHANNEL_UNAVAILABLE;
    }
    printf("Channel cap is 0x%x\n", cap);

    if (cap & canCHANNEL_IS_CANFD)
    {
        printf("Send CanFd message\n");
        if (mode == A825_SEND_SYNC)
        {
            stat = canWriteWait(hnd, messageid, data, dlc, canMSG_EXT | canFDMSG_FDF | canFDMSG_BRS, timeout);
            check("canWriteWait FD", stat);
            if (stat != canOK)
            {
                return A825_ERR_TIMEOUT;
            }
        }
        else
        {
            stat = canWrite(hnd, messageid, data, dlc, canMSG_EXT | canFDMSG_FDF | canFDMSG_BRS);
            check("canWrite FD", stat);
            if (stat != canOK)
            {
                return A825_RESOURCE_UNAVAILABLE;
            }
        }
    }
    else if (cap & canCHANNEL_IS_OPEN)
    {
        printf("Send Can message\n");
        if (dlc > 8)
        {
            return A825_ERR_INVALIDARG;
        }
        if (mode == A825_SEND_SYNC)
        {
            stat = canWriteWait(hnd, messageid, data, dlc, canMSG_EXT, timeout);
            check("canWriteWait", stat);
            if (stat != canOK)
            {
                return A825_ERR_TIMEOUT;
            }               
        }
        else
        {
            stat = canWrite(hnd, messageid, data, dlc, canMSG_EXT);
            check("canWrite", stat);
            if (stat != canOK)
            {
                return A825_RESOURCE_UNAVAILABLE;
            }
        }           
    }

    return A825_SUCCESS;
}


int recv_frame_fun (int mode, int hnd, long *messageid, unsigned char *data, unsigned int *dlc, unsigned long *timestamp, unsigned long timesout)
{
    canStatus stat;
    unsigned int canFlags;
    if (data == NULL || messageid == NULL || dlc == NULL || timestamp == NULL)
    {
        printf("Import data invalid\n");
        return A825_ERR_INVALIDARG;        
    }

    if (A825_RECEIVE_NONBLOCK == mode)
    {
        stat = canRead(hnd, messageid, data, dlc, &canFlags, timestamp);
        check("canRead", stat);
        if (stat != canOK) {
            return A825_NO_DATA;
        }        
    }
    else if (A825_RECEIVE_BLOCK == mode)
    {
        stat = canReadWait(hnd, messageid, data, dlc, &canFlags, timestamp, timesout);
        check("canReadWait", stat);
        if (stat != canOK) {
            return A825_ERR_TIMEOUT;
        }   
    }
    else
    {
        return A825_ERR_INVALIDARG;
    }
    return A825_SUCCESS;
}


int Check_Inbuf_Available_fun (int hnd, int timeout)
{
    canStatus stat;

    stat = canWriteSync(hnd, timeout);
    check("canWriteSync", stat);
    if (stat != canOK) {
        return A825_ERR_TIMEOUT;
    }

    return A825_SUCCESS;
}


const Arinc_Canif_Type Arinc_Canif =
{
    &Open_channel_fun,
    &Close_channel_fun,
    &send_frame_fun,
    &recv_frame_fun,
    &Check_Inbuf_Available_fun
};