/*******************************************************************************
 *
 * Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file xi2srx_debug.c
 * @addtogroup i2srx_v1_0
 * @{
 * <pre>
 * MODIFICATION HISTORY:
 * Ver   Who    Date     Changes
 * ----- ------ -------- --------------------------------------------------
 * 1.0   kar   01/25/18  Initial release.
 * </pre>
 *****************************************************************************/

/***************************** Include Files *********************************/
#include "xi2srx.h"
#include "xi2srx_hw.h"
#include "xi2srx_debug.h"
#include "xi2srx_chsts.h"
/************************** Constant Definitions *****************************/

/***************** Macros ( Inline Functions) Definitions ********************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 *
 * This function writes XI2s Receiver logs into the buffer.
 *
 * @param InstancePtr is a pointer to the XI2s_Rx_Log instance.
 * @param Event is the log event type.
 * @param Data is the log data.
 *
 * @return None.
 *
 * @note Logging will only be written if the logging is enabled.
 *
 *****************************************************************************/
void XI2s_Rx_LogWrite(XI2s_Rx_Log *InstancePtr, XI2s_Rx_LogEvt Event, u8 Data)
{
	u16 LogBufSize = 0;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Event < XI2S_RX_LOG_EVT_INVALID);

	if (!InstancePtr->IsEnabled)
		return;
	/* Write data and event into log buffer */
	InstancePtr->Items[InstancePtr->Head].Data = Data;
	InstancePtr->Items[InstancePtr->Head].Event = Event;

	/* Update head pointer */
	LogBufSize = sizeof(InstancePtr->Items)/sizeof(XI2s_Rx_LogItem);
	InstancePtr->Head++;
	if (InstancePtr->Head == LogBufSize)
		InstancePtr->Head = 0;
	/* Check tail pointer. When the two pointer are equal, then the buffer
	 * is full. In this case then increment the tail pointer as well to
	 * remove the oldest entry from the buffer.
	 */
	if (InstancePtr->Tail == InstancePtr->Head) {
		InstancePtr->Tail++;
		if (InstancePtr->Tail == LogBufSize)
			InstancePtr->Tail = 0;
	}
}
/*****************************************************************************/
/**
 * This function returns the next item in the logging buffer.
 * @param InstancePtr is a pointer to the XI2s_Rx_Log instance.
 * @return When the buffer is filled, the next log item is returned.
 * When the buffer is empty, NULL is returned.
 * @note None.
 *****************************************************************************/
XI2s_Rx_LogItem *XI2s_Rx_LogRead(XI2s_Rx_Log *InstancePtr)
{
	XI2s_Rx_LogItem *LogPtr = NULL;
	u16 LogBufSize = 0;
	/* Verify whether log is enabled */
	if((InstancePtr)->IsEnabled == FALSE)
		return NULL;
	/* Verify arguments */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if there is any data in the log buffer */
	if (InstancePtr->Tail == InstancePtr->Head)
		return NULL;
	LogPtr = &InstancePtr->Items[InstancePtr->Tail];

	/* Increment tail pointer */
	LogBufSize = sizeof(InstancePtr->Items)/sizeof(XI2s_Rx_LogItem);
	if (InstancePtr->Tail == (LogBufSize - 1))
		InstancePtr->Tail = 0;
	else
		InstancePtr->Tail++;
	return LogPtr;
}
/*****************************************************************************/
/**
 * This function clears the contents of the logging buffer.
 * @param InstancePtr is a pointer to the XI2s_Rx_Log instance.
 * @return None.
 * @note None.
 *****************************************************************************/
void XI2s_Rx_LogReset(XI2s_Rx *InstancePtr)
{
	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	if (InstancePtr->Log.IsEnabled == TRUE)
	{
		InstancePtr->Log.Head = 0;
		InstancePtr->Log.Tail = 0;
	}
}
/*****************************************************************************/
/**
 * This function prints the contents of the logging buffer.
 * @param InstancePtr is a pointer to the XI2s_Rx instance.
 * @return None.
 * @note None.
 *
 *****************************************************************************/
void XI2s_Rx_LogDisplay(XI2s_Rx *InstancePtr)
{
	XI2s_Rx_LogItem *LogPtr = NULL;

	/* Verify arguments */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid((InstancePtr)->Log.IsEnabled == TRUE);
	XI2s_Rx_LogEvt prevEvt = XI2S_RX_LOG_EVT_INVALID;
	int Count = 0;
	xil_printf("--------XI2S Receiver Log Start--------\n\r");
	while (1) {
		LogPtr = XI2s_Rx_LogRead(&InstancePtr->Log);
		/* if buffer is empty, NULL is returned */
		if (LogPtr == NULL) {
			if (Count != 0)
				xil_printf("..Repeats %d times..\n\r", Count);

			break;
		}
		if (prevEvt == LogPtr->Event) {
			Count++;
			continue;
		} else {
			if (Count != 0)
				xil_printf("..Repeats %d times..\n\r", Count);
			Count = 0;
		}
		xil_printf("XI2S Receiver %x: ", InstancePtr->Config.DeviceId);
		switch (LogPtr->Event) {
			case XI2S_RX_AES_BLKCMPLT_EVT:
				xil_printf("AES Block Complete Detected.\n\r");
				break;
			case XI2S_RX_AUD_OVERFLOW_EVT:
				xil_printf("Audio Overflow Detected.\n\r");
				break;
			default:
				xil_printf("Unknown Log Entry.\n\r");
				break;
		}
		prevEvt = LogPtr->Event;
	}
	xil_printf("--------XI2S Receiver Log End--------\n\r");
}
/** @} */
