#ifndef USB_CONTROLLER_H_
#define USB_CONTROLLER_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

#include <linux/hidraw.h>
#include <linux/version.h>
#include <linux/input.h>
#include <libudev.h>


#define USB_CONTROLLER_EXPORT /**< API export macro */
#define USB_CONTROLLER_CALL /**< API call macro */
#define USB_CONTROLLER_EXPORT_CALL USB_CONTROLLER_EXPORT USB_CONTROLLER_CALL

/* Symbolic names for the properties above */
enum device_string_id {
	DEVICE_STRING_MANUFACTURER,
	DEVICE_STRING_PRODUCT,
	DEVICE_STRING_SERIAL,

	DEVICE_STRING_COUNT,
};

struct usb_device_ {
	int device_handle;
	int blocking;
	int uses_numbered_reports;
};


/** hidapi info structure */
struct usb_device_info {
	/** Platform-specific device path */
	char *path;
	/** Device Vendor ID */
	unsigned short vendor_id;
	/** Device Product ID */
	unsigned short product_id;
	/** Serial Number */
	wchar_t *serial_number;
	/** Device Release Number in binary-coded decimal,
	    also known as Device Version Number */
	unsigned short release_number;
	/** Manufacturer String */
	wchar_t *manufacturer_string;
	/** Product string */
	wchar_t *product_string;
	/** Usage Page for this Device/Interface
	    (Windows/Mac only). */
	unsigned short usage_page;
	/** Usage for this Device/Interface
	    (Windows/Mac only).*/
	unsigned short usage;
	/** The USB interface which this logical device
	    represents. Valid on both Linux implementations
	    in all cases, and valid on the Windows implementation
	    only if the device contains more than one interface. */
	int interface_number;

	/** Pointer to the next device */
	struct usb_device_info *next;
};

/* USB HID device property names */
extern const char *device_string_names[];


/** @brief Enumerate the HID Devices.

			This function returns a linked list of all the HID devices
			attached to the system which match vendor_id and product_id.
			If @p vendor_id is set to 0 then any vendor matches.
			If @p product_id is set to 0 then any product matches.
			If @p vendor_id and @p product_id are both set to 0, then
			all HID devices will be returned.

			@ingroup API
			@param vendor_id The Vendor ID (VID) of the types of device
				to open.
			@param product_id The Product ID (PID) of the types of
				device to open.

		    @returns
		    	This function returns a pointer to a linked list of type
		    	struct #usb_device, containing information about the HID devices
		    	attached to the system, or NULL in the case of failure. Free
		    	this linked list by calling usb_free_enumeration().
		*/
extern struct usb_device_info USB_CONTROLLER_EXPORT * USB_CONTROLLER_CALL usb_enumerate(unsigned short vendor_id, unsigned short product_id);


/** @brief Free an enumeration Linked List

		    This function frees a linked list created by hid_enumerate().

			@ingroup API
		    @param devs Pointer to a list of struct_device returned from
		    	      hid_enumerate().
*/
extern void USB_CONTROLLER_EXPORT_CALL  usb_free_enumeration(struct usb_device_info *devs);


/** @brief Initialize the HIDAPI library.

			This function initializes the HIDAPI library. Calling it is not
			strictly necessary, as it will be called automatically by
			hid_enumerate() and any of the hid_open_*() functions if it is
			needed.  This function should be called at the beginning of
			execution however, if there is a chance of HIDAPI handles
			being opened by different threads simultaneously.
			
			@ingroup API

			@returns
				This function returns 0 on success and -1 on error.
*/
extern int USB_CONTROLLER_EXPORT_CALL usb_init(void);

 //======================================================

#endif
