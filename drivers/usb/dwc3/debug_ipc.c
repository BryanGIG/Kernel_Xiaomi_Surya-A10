/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "debug.h"

#include <linux/moduleparam.h>

static unsigned int ep_addr_rxdbg_mask;
module_param(ep_addr_rxdbg_mask, uint, 0644);
static unsigned int ep_addr_txdbg_mask;
module_param(ep_addr_txdbg_mask, uint, 0644);

static int allow_dbg_print(u8 ep_num)
{
	int dir, num;

	/* allow bus wide events */
	if (ep_num == 0xff)
		return 1;

	dir = ep_num & 0x1;
	num = ep_num >> 1;
	num = 1 << num;

	if (dir && (num & ep_addr_txdbg_mask))
		return 1;
	if (!dir && (num & ep_addr_rxdbg_mask))
		return 1;

	return 0;
}

/**
 * dwc3_dbg_print:  prints the common part of the event
 * @addr:   endpoint address
 * @name:   event name
 * @status: status
 * @extra:  extra information
 * @dwc3: pointer to struct dwc3
 */
void dwc3_dbg_print(struct dwc3 *dwc, u8 ep_num, const char *name,
			int status, const char *extra)
{
	if (!allow_dbg_print(ep_num))
		return;

	if (name == NULL)
		return;

	ipc_log_string(dwc->dwc_ipc_log_ctxt, "%02X %-25.25s %4i ?\t%s",
			ep_num, name, status, extra);
}

/**
 * dwc3_dbg_done: prints a DONE event
 * @addr:   endpoint address
 * @td:     transfer descriptor
 * @status: status
 * @dwc3: pointer to struct dwc3
 */
void dwc3_dbg_done(struct dwc3 *dwc, u8 ep_num,
		const u32 count, int status)
{
	if (!allow_dbg_print(ep_num))
		return;

	ipc_log_string(dwc->dwc_ipc_log_ctxt, "%02X %-25.25s %4i ?\t%d",
			ep_num, "DONE", status, count);
}

/**
 * dwc3_dbg_event: prints a generic event
 * @addr:   endpoint address
 * @name:   event name
 * @status: status
 */
void dwc3_dbg_event(struct dwc3 *dwc, u8 ep_num, const char *name, int status)
{
	if (!allow_dbg_print(ep_num))
		return;

	if (name != NULL)
		dwc3_dbg_print(dwc, ep_num, name, status, "");
}

/*
 * dwc3_dbg_queue: prints a QUEUE event
 * @addr:   endpoint address
 * @req:    USB request
 * @status: status
 */
void dwc3_dbg_queue(struct dwc3 *dwc, u8 ep_num,
		const struct usb_request *req, int status)
{
	if (!allow_dbg_print(ep_num))
		return;

	if (req != NULL) {
		ipc_log_string(dwc->dwc_ipc_log_ctxt,
			"%02X %-25.25s %4i ?\t%d %d", ep_num, "QUEUE", status,
			!req->no_interrupt, req->length);
	}
}

/**
 * dwc3_dbg_setup: prints a SETUP event
 * @addr: endpoint address
 * @req:  setup request
 */
void dwc3_dbg_setup(struct dwc3 *dwc, u8 ep_num,
		const struct usb_ctrlrequest *req)
{
	if (!allow_dbg_print(ep_num))
		return;

	if (req != NULL) {
		ipc_log_string(dwc->dwc_ipc_log_ctxt,
			"%02X %-25.25s ?\t%02X %02X %04X %04X %d",
			ep_num, "SETUP", req->bRequestType,
			req->bRequest, le16_to_cpu(req->wValue),
			le16_to_cpu(req->wIndex), le16_to_cpu(req->wLength));
	}
}

/**
 * dwc3_dbg_print_reg: prints a reg value
 * @name:   reg name
 * @reg: reg value to be printed
 */
void dwc3_dbg_print_reg(struct dwc3 *dwc, const char *name, int reg)
{
	if (name == NULL)
		return;

	ipc_log_string(dwc->dwc_ipc_log_ctxt, "%s = 0x%08x", name, reg);
}

void dwc3_dbg_dma_unmap(struct dwc3 *dwc, u8 ep_num, struct dwc3_request *req)
{
	if (ep_num < 2)
		return;

	ipc_log_string(dwc->dwc_dma_ipc_log_ctxt,
		"%02X-%-3.3s %-25.25s 0x%pK 0x%lx %u 0x%lx %d", ep_num >> 1,
		ep_num & 1 ? "IN":"OUT", "UNMAP", &req->request,
		req->request.dma, req->request.length, req->trb_dma,
		req->trb->ctrl & DWC3_TRB_CTRL_HWO);
}

void dwc3_dbg_dma_map(struct dwc3 *dwc, u8 ep_num, struct dwc3_request *req)
{
	if (ep_num < 2)
		return;

	ipc_log_string(dwc->dwc_dma_ipc_log_ctxt,
		"%02X-%-3.3s %-25.25s 0x%pK 0x%lx %u 0x%lx", ep_num >> 1,
		ep_num & 1 ? "IN":"OUT", "MAP", &req->request, req->request.dma,
		req->request.length, req->trb_dma);
}

void dwc3_dbg_dma_dequeue(struct dwc3 *dwc, u8 ep_num, struct dwc3_request *req)
{
	if (ep_num < 2)
		return;

	ipc_log_string(dwc->dwc_dma_ipc_log_ctxt,
		"%02X-%-3.3s %-25.25s 0x%pK 0x%lx 0x%lx", ep_num >> 1,
		ep_num & 1 ? "IN":"OUT", "DEQUEUE", &req->request,
		req->request.dma, req->trb_dma);
}

void dwc3_dbg_dma_queue(struct dwc3 *dwc, u8 ep_num, struct dwc3_request *req)
{
	if (ep_num < 2)
		return;

	ipc_log_string(dwc->dwc_dma_ipc_log_ctxt,
		"%02X-%-3.3s %-25.25s 0x%pK", ep_num >> 1,
		ep_num & 1 ? "IN":"OUT", "QUEUE", &req->request);
}
