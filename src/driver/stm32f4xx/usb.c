#include "driver/usb.h"

#include <string.h>

#include <stm32f4xx_ll_gpio.h>

#include <usb.h>
#include <usb_cdc.h>

#include "driver/gpio.h"
#include "driver/stm32f4xx/gpio_pin.h"

#include "util/ring_buffer.h"

#define CDC_EP0_SIZE 0x08
#define CDC_RXD_EP 0x01
#define CDC_TXD_EP 0x81
#define CDC_DATA_SZ 0x40
#define CDC_NTF_EP 0x82
#define CDC_NTF_SZ 0x08

#define BUFFER_SIZE 512

#define CDC_PROTOCOL USB_PROTO_NONE

struct cdc_config {
  struct usb_config_descriptor config;
  struct usb_iad_descriptor comm_iad;
  struct usb_interface_descriptor comm;
  struct usb_cdc_header_desc cdc_hdr;
  struct usb_cdc_call_mgmt_desc cdc_mgmt;
  struct usb_cdc_acm_desc cdc_acm;
  struct usb_cdc_union_desc cdc_union;
  struct usb_endpoint_descriptor comm_ep;
  struct usb_interface_descriptor data;
  struct usb_endpoint_descriptor data_eprx;
  struct usb_endpoint_descriptor data_eptx;
} __attribute__((packed));

static const struct usb_device_descriptor device_desc = {
    .bLength = sizeof(struct usb_device_descriptor),
    .bDescriptorType = USB_DTYPE_DEVICE,
    .bcdUSB = VERSION_BCD(2, 0, 0),
    .bDeviceClass = USB_CLASS_IAD,
    .bDeviceSubClass = USB_SUBCLASS_IAD,
    .bDeviceProtocol = USB_PROTO_IAD,
    .bMaxPacketSize0 = CDC_EP0_SIZE,
    .idVendor = 0x0483,
    .idProduct = 0x5740,
    .bcdDevice = VERSION_BCD(1, 0, 0),
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = INTSERIALNO_DESCRIPTOR,
    .bNumConfigurations = 1,
};

static const struct cdc_config config_desc = {
    .config = {
        .bLength = sizeof(struct usb_config_descriptor),
        .bDescriptorType = USB_DTYPE_CONFIGURATION,
        .wTotalLength = sizeof(struct cdc_config),
        .bNumInterfaces = 2,
        .bConfigurationValue = 1,
        .iConfiguration = NO_DESCRIPTOR,
        .bmAttributes = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
        .bMaxPower = USB_CFG_POWER_MA(100),
    },
    .comm_iad = {
        .bLength = sizeof(struct usb_iad_descriptor),
        .bDescriptorType = USB_DTYPE_INTERFASEASSOC,
        .bFirstInterface = 0,
        .bInterfaceCount = 2,
        .bFunctionClass = USB_CLASS_CDC,
        .bFunctionSubClass = USB_CDC_SUBCLASS_ACM,
        .bFunctionProtocol = CDC_PROTOCOL,
        .iFunction = NO_DESCRIPTOR,
    },
    .comm = {
        .bLength = sizeof(struct usb_interface_descriptor),
        .bDescriptorType = USB_DTYPE_INTERFACE,
        .bInterfaceNumber = 0,
        .bAlternateSetting = 0,
        .bNumEndpoints = 1,
        .bInterfaceClass = USB_CLASS_CDC,
        .bInterfaceSubClass = USB_CDC_SUBCLASS_ACM,
        .bInterfaceProtocol = CDC_PROTOCOL,
        .iInterface = NO_DESCRIPTOR,
    },
    .cdc_hdr = {
        .bFunctionLength = sizeof(struct usb_cdc_header_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_HEADER,
        .bcdCDC = VERSION_BCD(1, 1, 0),
    },
    .cdc_mgmt = {
        .bFunctionLength = sizeof(struct usb_cdc_call_mgmt_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_CALL_MANAGEMENT,
        .bmCapabilities = 0,
        .bDataInterface = 1,

    },
    .cdc_acm = {
        .bFunctionLength = sizeof(struct usb_cdc_acm_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_ACM,
        .bmCapabilities = 0,
    },
    .cdc_union = {
        .bFunctionLength = sizeof(struct usb_cdc_union_desc),
        .bDescriptorType = USB_DTYPE_CS_INTERFACE,
        .bDescriptorSubType = USB_DTYPE_CDC_UNION,
        .bMasterInterface0 = 0,
        .bSlaveInterface0 = 1,
    },
    .comm_ep = {
        .bLength = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType = USB_DTYPE_ENDPOINT,
        .bEndpointAddress = CDC_NTF_EP,
        .bmAttributes = USB_EPTYPE_INTERRUPT,
        .wMaxPacketSize = CDC_NTF_SZ,
        .bInterval = 0xFF,
    },
    .data = {
        .bLength = sizeof(struct usb_interface_descriptor),
        .bDescriptorType = USB_DTYPE_INTERFACE,
        .bInterfaceNumber = 1,
        .bAlternateSetting = 0,
        .bNumEndpoints = 2,
        .bInterfaceClass = USB_CLASS_CDC_DATA,
        .bInterfaceSubClass = USB_SUBCLASS_NONE,
        .bInterfaceProtocol = USB_PROTO_NONE,
        .iInterface = NO_DESCRIPTOR,
    },
    .data_eprx = {
        .bLength = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType = USB_DTYPE_ENDPOINT,
        .bEndpointAddress = CDC_RXD_EP,
        .bmAttributes = USB_EPTYPE_BULK,
        .wMaxPacketSize = CDC_DATA_SZ,
        .bInterval = 0x01,
    },
    .data_eptx = {
        .bLength = sizeof(struct usb_endpoint_descriptor),
        .bDescriptorType = USB_DTYPE_ENDPOINT,
        .bEndpointAddress = CDC_TXD_EP,
        .bmAttributes = USB_EPTYPE_BULK,
        .wMaxPacketSize = CDC_DATA_SZ,
        .bInterval = 0x01,
    },
};

static const struct usb_string_descriptor lang_desc = USB_ARRAY_DESC(USB_LANGID_ENG_US);
static const struct usb_string_descriptor manuf_desc_en = USB_STRING_DESC("Open source USB stack for STM32");
static const struct usb_string_descriptor prod_desc_en = USB_STRING_DESC("CDC Loopback demo");
static const struct usb_string_descriptor *const dtable[] = {
    &lang_desc,
    &manuf_desc_en,
    &prod_desc_en,
};

static struct usb_cdc_line_coding cdc_line = {
    .dwDTERate = 115200,
    .bCharFormat = USB_CDC_1_STOP_BITS,
    .bParityType = USB_CDC_NO_PARITY,
    .bDataBits = 8,
};

static usbd_device udev;
static uint32_t ubuf[0x20];

static uint8_t tx_buffer_data[BUFFER_SIZE];
static volatile bool tx_buffer_in_use = false;
static ring_buffer_t tx_buffer = {
    .buffer = tx_buffer_data,
    .head = 0,
    .tail = 0,
    .size = BUFFER_SIZE,
};

static uint8_t rx_buffer_data[BUFFER_SIZE];
static ring_buffer_t rx_buffer = {
    .buffer = rx_buffer_data,
    .head = 0,
    .tail = 0,
    .size = BUFFER_SIZE,
};

static usbd_respond cdc_getdesc(usbd_ctlreq *req, void **address, uint16_t *length) {
  const uint8_t dtype = req->wValue >> 8;
  const uint8_t dnumber = req->wValue & 0xFF;
  const void *desc;
  uint16_t len = 0;
  switch (dtype) {
  case USB_DTYPE_DEVICE:
    desc = &device_desc;
    break;
  case USB_DTYPE_CONFIGURATION:
    desc = &config_desc;
    len = sizeof(config_desc);
    break;
  case USB_DTYPE_STRING:
    if (dnumber < 3) {
      desc = dtable[dnumber];
    } else {
      return usbd_fail;
    }
    break;
  default:
    return usbd_fail;
  }
  if (len == 0) {
    len = ((struct usb_header_descriptor *)desc)->bLength;
  }
  *address = (void *)desc;
  *length = len;
  return usbd_ack;
}

static usbd_respond cdc_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback) {
  if (((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) == (USB_REQ_INTERFACE | USB_REQ_CLASS) && req->wIndex == 0) {
    switch (req->bRequest) {
    case USB_CDC_SET_CONTROL_LINE_STATE:
      return usbd_ack;
    case USB_CDC_SET_LINE_CODING:
      memcpy(req->data, &cdc_line, sizeof(cdc_line));
      return usbd_ack;
    case USB_CDC_GET_LINE_CODING:
      dev->status.data_ptr = &cdc_line;
      dev->status.data_count = sizeof(cdc_line);
      return usbd_ack;
    default:
      return usbd_fail;
    }
  }
  return usbd_fail;
}

static void cdc_rxonly(usbd_device *dev, uint8_t event, uint8_t ep) {
  uint8_t buf[CDC_DATA_SZ];
  const int32_t len = usbd_ep_read(dev, ep, buf, CDC_DATA_SZ);
  if (len == 0) {
    return;
  }
  ring_buffer_write(&rx_buffer, buf, len);
}

static void cdc_txonly(usbd_device *dev, uint8_t event, uint8_t ep) {
  if (tx_buffer_in_use) {
    usbd_ep_write(dev, ep, 0, 0);
    return;
  }

  uint8_t buf[CDC_DATA_SZ];

  tx_buffer_in_use = true;
  const uint32_t len = ring_buffer_read(&tx_buffer, buf, CDC_DATA_SZ);
  tx_buffer_in_use = false;

  usbd_ep_write(dev, ep, buf, len);
}

static void cdc_rxtx(usbd_device *dev, uint8_t event, uint8_t ep) {
  if (event == usbd_evt_eptx) {
    cdc_txonly(dev, event, ep);
  } else {
    cdc_rxonly(dev, event, ep);
  }
}

static usbd_respond cdc_setconf(usbd_device *dev, uint8_t cfg) {
  switch (cfg) {
  case 0:
    /* deconfiguring device */
    usbd_ep_deconfig(dev, CDC_NTF_EP);
    usbd_ep_deconfig(dev, CDC_TXD_EP);
    usbd_ep_deconfig(dev, CDC_RXD_EP);
    usbd_reg_endpoint(dev, CDC_RXD_EP, 0);
    usbd_reg_endpoint(dev, CDC_TXD_EP, 0);
    return usbd_ack;
  case 1:
    /* configuring device */
    usbd_ep_config(dev, CDC_RXD_EP, USB_EPTYPE_BULK /*| USB_EPTYPE_DBLBUF*/, CDC_DATA_SZ);
    usbd_ep_config(dev, CDC_TXD_EP, USB_EPTYPE_BULK /*| USB_EPTYPE_DBLBUF*/, CDC_DATA_SZ);
    usbd_ep_config(dev, CDC_NTF_EP, USB_EPTYPE_INTERRUPT, CDC_NTF_SZ);
#if ((CDC_TXD_EP & 0x7F) == (CDC_RXD_EP & 0x7F))
    usbd_reg_endpoint(dev, CDC_RXD_EP, cdc_rxtx);
    usbd_reg_endpoint(dev, CDC_TXD_EP, cdc_rxtx);
#else
    usbd_reg_endpoint(dev, CDC_RXD_EP, cdc_rxonly);
    usbd_reg_endpoint(dev, CDC_TXD_EP, cdc_txonly);
#endif
    usbd_ep_write(dev, CDC_TXD_EP, 0, 0);
    return usbd_ack;
  default:
    return usbd_fail;
  }
}

static void cdc_init_usbd(void) {
  usbd_init(&udev, &usbd_hw, CDC_EP0_SIZE, ubuf, sizeof(ubuf));
  usbd_reg_config(&udev, cdc_setconf);
  usbd_reg_control(&udev, cdc_control);
  usbd_reg_descr(&udev, cdc_getdesc);
}

static void usb_hardware_init() {
  gpio_config(&PA11, GPIO_AF | GPIO_PP | GPIO_PULL_NONE);
  gpio_config_af(&PA11, GPIO_AF10_OTG_FS);

  gpio_config(&PA12, GPIO_AF | GPIO_PP | GPIO_PULL_NONE);
  gpio_config_af(&PA12, GPIO_AF10_OTG_FS);

  NVIC_EnableIRQ(OTG_FS_IRQn);
}

void usb_init() {
  usb_hardware_init();

  cdc_init_usbd();
  usbd_enable(&udev, true);
  usbd_connect(&udev, true);
}

uint8_t usb_serial_put(uint8_t data) {
  while (tx_buffer_in_use)
    ;

  tx_buffer_in_use = true;
  uint8_t res = ring_buffer_put(&tx_buffer, data);
  tx_buffer_in_use = false;

  return res;
}

uint8_t usb_serial_get(uint8_t *data) {
  return ring_buffer_get(&rx_buffer, data);
}

uint8_t usb_serial_write(const uint8_t *data, const uint32_t len) {
  while (tx_buffer_in_use)
    ;

  tx_buffer_in_use = true;
  uint8_t res = ring_buffer_write(&tx_buffer, data, len);
  tx_buffer_in_use = false;

  return res;
}

void OTG_FS_IRQHandler() {
  usbd_poll(&udev);
}