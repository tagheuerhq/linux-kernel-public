/*
 * host_action.h
 * Allow host to request actions/information over USB control endpoint.
 *
 * Copyright (C) 2021 TAG Heuer Connected
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef USB__GADGET__HOST__ACTION__H
#define USB__GADGET__HOST__ACTION__H

/* Host Action USB Vendor Request IDs */
#define USB_REQUEST_HOST_ACTION_COMMAND             0xAB
#define USB_REQUEST_HOST_ACTION_CHALLENGE_RESPONSE  0xAC

/* Host Action Command IDs */
#define HOST_ACTION_CMD_GET_INFO    0x01
#define HOST_ACTION_CMD_SET_ACTION  0x02

/* Forward declerations */
struct device;
struct usb_ctrlrequest;
struct usb_gadget;

/*
 * Handle Host Action USB control request
 *  - return 0 if not handled (because not a host action request for instance)
 *  - return positive number if handled
 *  - return negative errno in case of error
 */
int host_action_handle_usb_request(struct usb_gadget *gadget,
								  const struct usb_ctrlrequest *ctrl);

/* Create the sysfs attribute files needed by the Host Action feature */
int host_action_create_sysfs_files(struct device *device);

/* Remove the sysfs attribute files created by the Host Action feature */
void host_action_remove_sysfs_files(struct device *device);

#endif /*  USB__GADGET__HOST__ACTION__H */
