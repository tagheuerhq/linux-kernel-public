/*
 * host_action.c
 * Allow host to request actions/information over USB control endpoint.
 *
 * Copyright (C) 2021 TAG Heuer Connected
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/kernel.h>

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>

#include "host_action.h"

/* Called when userland wants to read the last received host action */
static ssize_t show_host_action(struct device *dev,
		struct device_attribute *attr, char *buf);

/* Called when userland wants to read challenge_response to verify signature */
static ssize_t show_host_challenge_response(struct device *dev,
		struct device_attribute *attr, char *buf);

/* Called when userland wants read current host_info */
static ssize_t show_host_info(struct device *dev,
		struct device_attribute *attr, char *buf);

/* Called when userland wants to update current host_info.
 * These host_info are sent to the host when requested over USB */
static ssize_t store_host_info(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t len);

static DEVICE_ATTR(host_action, 0400, show_host_action, NULL);
static DEVICE_ATTR(host_challenge_response, 0400, show_host_challenge_response, NULL);
static DEVICE_ATTR(host_info, 0600, show_host_info, store_host_info);

/*
 * Handle Host Action GET_INFO Command
 *
 * Send the current host_info (filled by userland using sysfs attribute)
 * in the USB response */
int handle_cmd_get_info(struct usb_gadget *gadget,
							const struct usb_ctrlrequest *ctrl)
{
	u16 w_length = le16_to_cpu(ctrl->wLength);
	u16 data_length = strlen(gadget->host_info);
	u8 data_direction = (ctrl->bRequestType & USB_DIR_IN);
	struct usb_composite_dev *cdev = get_gadget_data(gadget);
	struct usb_request *req = cdev->req;
	unsigned char *buffer = (unsigned char *)req->buf;

	/* Request Data Direction MUST BE Device -> Host */
	if (data_direction != USB_DIR_IN) {
		ERROR(cdev, "Unsupported Host Action GET_INFO Data Direction: 0x%X\n",
				data_direction);
		return -EOPNOTSUPP;
	}

	/* Check data overflow */
	if (data_length > USB_COMP_EP0_BUFSIZ) {
		ERROR(cdev, "Host Action GET_INFO Data Overflow: %u bytes\n", data_length);
		return -EOPNOTSUPP;
	}

	/* Two first bytes are the data length (Little Endian) (u16) */
	buffer[0] = (data_length >> 0) & 0xFF;
	buffer[1] = (data_length >> 8) & 0xFF;

	/* Remaining bytes are host info string (no null-terminated) */
	strncpy(&buffer[2], gadget->host_info, sizeof(gadget->host_info));

	/* Initialize USB response (data contains host_info) */
	req->length = min(w_length, (u16)(2 + data_length));
	req->context = cdev;
	req->zero = req->length < w_length;

	return 1;
}

/*
 * Handle Host Action SET_ACTION Command
 *
 * Update the last received host action and notify userland */
int handle_cmd_set_action(struct usb_gadget *gadget,
		const struct usb_ctrlrequest *ctrl)
{
	char *envp[] = {"HOST_ACTION_EVENT=RECEIVED", NULL};
	u16 action = le16_to_cpu(ctrl->wIndex);
	u16	w_length = le16_to_cpu(ctrl->wLength);
	u8 data_direction = (ctrl->bRequestType & USB_DIR_IN);
	struct usb_composite_dev *cdev = get_gadget_data(gadget);
	struct usb_request *req = cdev->req;

	/* Request Data Direction MUST BE Host -> Device */
	if (data_direction != USB_DIR_OUT) {
		ERROR(cdev, "Unsupported Host Action SET_ACTION Data Direction: 0x%X\n",
				data_direction);
		return -EOPNOTSUPP;
	}

	/* Update the last received host action */
	gadget->host_action = action;

	/* Notify userland that a new action has been received */
	kobject_uevent_env(&gadget->dev.kobj, KOBJ_CHANGE, envp);

	/* Initialize USB response (no data) */
	req->length = 0;
	req->context = cdev;
	req->zero = req->length < w_length;

	return 1;
}

/*
 * Handle Host Action Command
 */
int handle_command(struct usb_gadget *gadget,
		const struct usb_ctrlrequest *ctrl)
{
	u16	command = le16_to_cpu(ctrl->wValue);
	struct usb_composite_dev *cdev = get_gadget_data(gadget);

	switch (command) {
	case HOST_ACTION_CMD_GET_INFO:
		return handle_cmd_get_info(gadget, ctrl);

	case HOST_ACTION_CMD_SET_ACTION:
		return handle_cmd_set_action(gadget, ctrl);

	default:
		ERROR(cdev, "Unsupported Host Action Command: 0x%X\n", command);
		return -EOPNOTSUPP;
	}
}

/* This request allows to change a byte of challenge_response at the provided
 * offset. By convention the last CHALLENGE_RESPONSE request received
 * defines the current size of the challenge_response (using offset argument).
 * On other words the very last byte of the challenge_response must be modified
 * in the very last USB request.
 *
 * It also means, that for a host to send a xxx bytes challenge_response:
 * xxx USB CHALLENGE_RESPONSE requests must be sent.
 *
 * It is a workaround because our USB Controler (UDC) driver unfortunately
 * do not handle data in control requests from HOST to DEVICE. */
int handle_challenge_response(struct usb_gadget *gadget,
		const struct usb_ctrlrequest *ctrl)
{
	u16	w_value = le16_to_cpu(ctrl->wValue);
	u16 offset = le16_to_cpu(ctrl->wIndex);
	u16	w_length = le16_to_cpu(ctrl->wLength);
	u8 data_direction = (ctrl->bRequestType & USB_DIR_IN);
	struct usb_composite_dev *cdev = get_gadget_data(gadget);
	struct usb_request *req = cdev->req;

	/* USB Request MUST BE CHALLENGE_RESPONSE */
	if (ctrl->bRequest != USB_REQUEST_HOST_ACTION_CHALLENGE_RESPONSE) {
		ERROR(cdev, "Unsupported Host Action Request: 0x%X\n", ctrl->bRequest);
		return -EOPNOTSUPP;
	}

	/* USB Request Data Direction MUST BE Host -> Device */
	if (data_direction != USB_DIR_OUT) {
		ERROR(cdev, "Unsupported CHALLENGE_RESPONSE Data Direction: 0x%X\n",
				data_direction);
		return -EOPNOTSUPP;
	}

	/* Check offset/size consistency */
	if (offset >= sizeof(gadget->host_challenge_response)) {
		ERROR(cdev, "Challenge Response offset too big: %u\n", offset);
		return -EOPNOTSUPP;
	}

	/* Update challenge_response value */
	gadget->host_challenge_response[offset] = (w_value & 0xFF);

	/* By convention the last offset received/updated defines the
	 * challenge_response size */
	gadget->host_challenge_response_size = (offset + 1);

	/* Initialize USB response (no data) */
	req->length = 0;
	req->context = cdev;
	req->zero = req->length < w_length;

	return 1;
}

/*
 * Handle Host Action USB control request
 *  - return 0 if not handled (because not a host action request for instance)
 *  - return positive number if handled
 *  - return negative errno in case of error
 */
int host_action_handle_usb_request(struct usb_gadget *gadget,
								  const struct usb_ctrlrequest *ctrl)
{
	if (!gadget) return -EINVAL;
	if (!ctrl) return -EINVAL;

	/* Not a Host Action Request */
	if ((ctrl->bRequestType & USB_TYPE_VENDOR) != USB_TYPE_VENDOR) return 0;
	if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_OTHER) return 0;

	switch (ctrl->bRequest) {
	/* Host Action COMMAND Request */
	case USB_REQUEST_HOST_ACTION_COMMAND:
		return handle_command(gadget, ctrl);

	/* Host Action CHALLENGE_RESPONSE Request */
	case USB_REQUEST_HOST_ACTION_CHALLENGE_RESPONSE:
		return handle_challenge_response(gadget, ctrl);

	default:
		/* Unknown Host Action Request */
		return 0;
	}
}

/* Called when userland wants to read the last received host action */
static ssize_t show_host_action(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct usb_gadget *gadget = dev_to_usb_gadget(dev);

	return scnprintf(buf, PAGE_SIZE, "%u", gadget->host_action);
}

/* Called when userland wants to read challenge_response to verify signature */
static ssize_t show_host_challenge_response(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct usb_gadget *gadget = dev_to_usb_gadget(dev);

	if (gadget->host_challenge_response_size == 0) return 0;

	memcpy(buf, gadget->host_challenge_response,
			gadget->host_challenge_response_size);
	return gadget->host_challenge_response_size;
}

/* Called when userland wants read current host_info */
static ssize_t show_host_info(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct usb_gadget *gadget = dev_to_usb_gadget(dev);

	return scnprintf(buf, PAGE_SIZE, "%s", gadget->host_info);
}

/* Called when userland wants to update current host_info.
 * These host_info are sent to the host when requested over USB */
static ssize_t store_host_info(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf, size_t len)
{
	struct usb_gadget *gadget = dev_to_usb_gadget(dev);

	if (len > sizeof(gadget->host_info)) return -ENOMEM;

	strncpy(gadget->host_info, buf, sizeof(gadget->host_info));

	return len;
}

/* Create the sysfs attribute files needed by the host_action feature */
int host_action_create_sysfs_files(struct device *device)
{
	int ret;

	if (!device) return -EINVAL;

	ret = device_create_file(device, &dev_attr_host_action);
	if (ret) return ret;

	ret = device_create_file(device, &dev_attr_host_challenge_response);
	if (ret) return ret;

	ret = device_create_file(device, &dev_attr_host_info);
	if (ret) return ret;

	return 0;
}

/* Remove the sysfs attribute files created by the host_action feature */
void host_action_remove_sysfs_files(struct device *device)
{
	if (!device) return;

	device_remove_file(device, &dev_attr_host_info);
	device_remove_file(device, &dev_attr_host_challenge_response);
	device_remove_file(device, &dev_attr_host_action);
}
