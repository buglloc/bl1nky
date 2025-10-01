#include "usb_hid.h"
#include <stdint.h>

/* USB control state */
volatile uint16_t usb_setup_len = 0;
volatile uint8_t  usb_setup_req = 0;
volatile uint8_t  usb_config = 0;
volatile const uint8_t* descr_ptr = 0;

/* HID state */
volatile __xdata uint8_t hid_cmd_received = 0;  // Flag: new command received
volatile __xdata uint8_t hid_cmd = 0;           // Received command
volatile __xdata uint8_t hid_cmd_data = 0;      // Received command data
volatile __xdata uint8_t hid_response_cmd = 0;  // Response command
volatile __xdata uint8_t hid_response_data = 0; // Response data
volatile __xdata uint8_t hid_idle = 0;          // Idle rate
volatile __xdata uint8_t hid_protocol = 1;      // 0=boot, 1=report

USB_SETUP_REQ usb_setup_req_buf;
#define USB_SETUP_BUFFER ((PUSB_SETUP_REQ)usb_ep0_buf)

/* Device/Config/String descriptors */
__code uint8_t dev_desc[] = {
  0x12,       // [0]  bLength
  0x01,       // [1]  bDescriptorType: Device
  0x10, 0x01, // [2]  bcdUSB: USB 1.1
  0x00,       // [4]  bDeviceClass: Use interface class
  0x00,       // [5]  bDeviceSubClass
  0x00,       // [6]  bDeviceProtocol
  DEFAULT_ENDP0_SIZE, // [7] bMaxPacketSize0
  0x09, 0x12, // [8]  idVendor: 0x1209
  0x00, 0xF6, // [10] idProduct: 0xF600
  0x00, 0x01, // [12] bcdDevice: 1.00
  0x01,       // [14] iManufacturer
  0x02,       // [15] iProduct
  0x03,       // [16] iSerialNumber
  0x01        // [17] bNumConfigurations
};

/* HID Report Descriptor:
 * - 2 byte OUTPUT report (host -> device): [command, data]
 * - 2 byte Feature report (device -> host): [command, data]
 */
 __code uint8_t hid_report_desc[] = {
  0x06, 0xFF, 0x00,  // Usage Page (Vendor Defined 0xFF)
  0x09, 0xCF,        // Usage (Vendor Usage 0xCF)
  0xA1, 0x01,        // Collection (Application)

  // Output Report (Host → Device) - 2 bytes: [command, data]
  0x15, 0x00,        //   Logical Minimum (0)
  0x26, 0xFF, 0x00,  //   Logical Maximum (255)
  0x75, 0x08,        //   Report Size (8 bits)
  0x95, 0x02,        //   Report Count (2 bytes)
  0x09, 0x01,        //   Usage (Vendor Usage 1)
  0x91, 0x02,        //   Output (Data,Var,Abs)

  // Feature Report (Device ↔ Host) - 2 bytes: [command, data]
  0x15, 0x00,        //   Logical Minimum (0)
  0x26, 0xFF, 0x00,  //   Logical Maximum (255)
  0x75, 0x08,        //   Report Size (8 bits)
  0x95, 0x02,        //   Report Count (2 bytes)
  0x09, 0x02,        //   Usage (Vendor Usage 2)
  0xB1, 0x02,        //   Feature (Data,Var,Abs)

  0xC0               // End Collection
};

__code uint8_t cfg_desc[] = {
  // ----- Configuration Descriptor -----
  0x09,           // bLength
  0x02,           // bDescriptorType: CONFIGURATION
  0x29, 0x00,     // wTotalLength: 41 bytes
  0x01,           // bNumInterfaces: 1
  0x01,           // bConfigurationValue
  0x00,           // iConfiguration
  0xA0,           // bmAttributes: bus powered, remote wakeup
  0x32,           // bMaxPower: 100 mA

  // ----- Interface Descriptor -----
  0x09,           // bLength
  0x04,           // bDescriptorType: INTERFACE
  0x00,           // bInterfaceNumber
  0x00,           // bAlternateSetting
  0x01,           // bNumEndpoints: 1 (interrupt IN)
  0x03,           // bInterfaceClass: HID
  0x00,           // bInterfaceSubClass: No subclass
  0x00,           // bInterfaceProtocol: None
  0x00,           // iInterface

  // ----- HID Descriptor -----
  0x09,           // bLength
  0x21,           // bDescriptorType: HID
  0x11, 0x01,     // bcdHID: HID 1.11
  0x00,           // bCountryCode: Not supported
  0x01,           // bNumDescriptors: 1
  0x22,           // bDescriptorType: REPORT
  sizeof(hid_report_desc), 0x00, // wDescriptorLength

  // ----- Endpoint Descriptor (Interrupt IN) -----
  0x07,           // bLength
  0x05,           // bDescriptorType: ENDPOINT
  0x81,           // bEndpointAddress: IN endpoint 1
  0x03,           // bmAttributes: Interrupt
  0x08, 0x00,     // wMaxPacketSize: 8 bytes
  0x0A            // bInterval: 10 ms
};

__code unsigned char lang_desc[] = {
  0x04, 0x03,     // Length, STRING descriptor
  0x09, 0x04      // English (US)
};

__code unsigned char product_desc[] = {
  0x0E, 0x03,     // Length (14), STRING
  0x42, 0x00,     // 'B'
  0x6C, 0x00,     // 'l'
  0x31, 0x00,     // '1'
  0x6E, 0x00,     // 'n'
  0x6B, 0x00,     // 'k'
  0x79, 0x00      // 'y'
};

__code unsigned char vendor_desc[] = {
  0x0C, 0x03,     // Length (12), STRING
  0x40, 0x00,     // '@'
  0x55, 0x00,     // 'U'
  0x54, 0x00,     // 'T'
  0x42, 0x00,     // 'B'
  0x4B, 0x00      // 'K'
};

__code unsigned char serial_desc[] = {
  0x0E, 0x03,     // Length (14), STRING
  0x31, 0x00,     // '1'
  0x31, 0x00,     // '1'
  0x39, 0x00,     // '9'
  0x30, 0x00,     // '0'
  0x32, 0x00,     // '2'
  0x31, 0x00      // '1'
};

/* impl */

static void usb_device_cfg(void)
{
  USB_CTRL = 0x00;
  USB_CTRL &= ~bUC_HOST_MODE;                          // Device mode
  USB_CTRL |=  bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN;
  USB_DEV_AD = 0x00;

  USB_CTRL &= ~bUC_LOW_SPEED;                          // Full-speed
  UDEV_CTRL &= ~bUD_LOW_SPEED;
  UDEV_CTRL  = bUD_PD_DIS;                             // Disable DP/DM pull-down
  UDEV_CTRL |= bUD_PORT_EN;                            // Enable physical port
}

static void usb_device_endpoint_cfg(void)
{
  UEP1_DMA = (uint16_t)usb_ep1_buf;                    // EP1 IN buffer
  UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;           // EP1 IN NAK

  UEP0_DMA  = (uint16_t)usb_ep0_buf;                   // EP0 buffer
  UEP4_1_MOD = 0x40;                                   // EP1 IN buffer, EP0 single 64B buffer
  UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;           // EP0 OUT ACK, IN NAK
}

static void usb_device_int_cfg(void)
{
  USB_INT_EN |= bUIE_SUSPEND | bUIE_TRANSFER | bUIE_BUS_RST;
  USB_INT_FG |= 0x1F;                                  // Clear flags
  IE_USB = 1;                                          // Enable USB interrupt
  EA = 1;                                              // Enable global interrupts
}

void usb_hid_init(void)
{
  usb_device_cfg();
  usb_device_endpoint_cfg();
  usb_device_int_cfg();

  UEP0_T_LEN = 0;
  UEP1_T_LEN = 0;

  hid_cmd_received = 0;
  hid_cmd = 0;
  hid_cmd_data = 0;
  hid_response_cmd = 0;
  hid_response_data = 0;
}

uint8_t usb_hid_mounted(void)
{
  return usb_config != 0;
}

uint8_t usb_hid_has_command(void)
{
  return hid_cmd_received;
}

void usb_hid_get_command(uint8_t *cmd, uint8_t *data)
{
  *cmd = hid_cmd;
  *data = hid_cmd_data;
  hid_cmd_received = 0;
}

void usb_hid_send_response(uint8_t cmd, uint8_t data)
{
  hid_response_cmd = cmd;
  hid_response_data = data;
}

void usb_hid_interrupt(void)
{
  uint16_t len;

  if (UIF_TRANSFER) {
    switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP)) {
    case (UIS_TOKEN_IN | 1):                         // EP1 IN complete
      UEP1_T_LEN = 0;
      UEP1_CTRL = (UEP1_CTRL & ~MASK_UEP_T_RES) | UEP_T_RES_NAK;
      break;

    case (UIS_TOKEN_SETUP | 0):                      // EP0 SETUP
      len = USB_RX_LEN;
      if (len == sizeof(USB_SETUP_REQ)) {
        usb_setup_len = ((uint16_t)USB_SETUP_BUFFER->wLengthH << 8) | USB_SETUP_BUFFER->wLengthL;
        len = 0;
        usb_setup_req = USB_SETUP_BUFFER->bRequest;

        if ((USB_SETUP_BUFFER->bRequestType & USB_REQ_TYP_MASK) == USB_REQ_TYP_CLASS) {
          // HID class requests
          switch (usb_setup_req) {
          case HID_GET_REPORT:
            // Send current response (command + data) back to host
            usb_ep0_buf[0] = hid_response_cmd;
            usb_ep0_buf[1] = hid_response_data;
            len = 2;
            break;

          case HID_SET_REPORT:
            // Will receive data in EP0 OUT
            len = 0;
            break;

          case HID_GET_IDLE:
            usb_ep0_buf[0] = hid_idle;
            len = 1;
            break;

          case HID_SET_IDLE:
            hid_idle = USB_SETUP_BUFFER->wValueH;
            break;

          case HID_GET_PROTOCOL:
            usb_ep0_buf[0] = hid_protocol;
            len = 1;
            break;

          case HID_SET_PROTOCOL:
            hid_protocol = USB_SETUP_BUFFER->wValueL;
            break;

          default:
            len = 0xFF;  // Stall
            break;
          }
        } else if ((USB_SETUP_BUFFER->bRequestType & USB_REQ_TYP_MASK) == USB_REQ_TYP_STANDARD) {
          // Standard requests
          switch (usb_setup_req) {
          case USB_GET_DESCRIPTOR:
            switch (USB_SETUP_BUFFER->wValueH) {
            case 1:  // Device descriptor
              descr_ptr = dev_desc;
              len = sizeof(dev_desc);
              break;
            case 2:  // Configuration descriptor
              descr_ptr = cfg_desc;
              len = sizeof(cfg_desc);
              break;
            case 3:  // String descriptor
              if (USB_SETUP_BUFFER->wValueL == 0) {
                descr_ptr = lang_desc;
                len = sizeof(lang_desc);
              } else if (USB_SETUP_BUFFER->wValueL == 1) {
                descr_ptr = vendor_desc;
                len = sizeof(vendor_desc);
              } else if (USB_SETUP_BUFFER->wValueL == 2) {
                descr_ptr = product_desc;
                len = sizeof(product_desc);
              } else {
                descr_ptr = serial_desc;
                len = sizeof(serial_desc);
              }
              break;
            case HID_DESCRIPTOR_TYPE_HID:  // HID descriptor
              // Point to HID descriptor within config descriptor (offset 18)
              descr_ptr = cfg_desc + 18;
              len = 9;
              break;
            case HID_DESCRIPTOR_TYPE_REPORT:  // Report descriptor
              descr_ptr = hid_report_desc;
              len = sizeof(hid_report_desc);
              break;
            default:
              len = 0xFF;
              break;
            }
            if (len != 0xFF) {
              if (usb_setup_len > len) usb_setup_len = len;
              len = (usb_setup_len >= DEFAULT_ENDP0_SIZE) ? DEFAULT_ENDP0_SIZE : usb_setup_len;
              memcpy(usb_ep0_buf, descr_ptr, len);
              usb_setup_len -= len;
              descr_ptr += len;
            }
            break;

          case USB_SET_ADDRESS:
            usb_setup_len = USB_SETUP_BUFFER->wValueL;
            break;

          case USB_GET_CONFIGURATION:
            usb_ep0_buf[0] = usb_config;
            if (usb_setup_len >= 1) len = 1;
            break;

          case USB_SET_CONFIGURATION:
            usb_config = USB_SETUP_BUFFER->wValueL;
            break;

          case USB_GET_INTERFACE:
            usb_ep0_buf[0] = 0;
            if (usb_setup_len >= 1) len = 1;
            break;

          case USB_SET_INTERFACE:
            break;

          case USB_CLEAR_FEATURE:
          case USB_SET_FEATURE:
            if ((USB_SETUP_BUFFER->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) {
              switch (USB_SETUP_BUFFER->wIndexL) {
              case 0x81:
                if (usb_setup_req == USB_CLEAR_FEATURE) {
                  UEP1_CTRL = (UEP1_CTRL & ~(bUEP_T_TOG | MASK_UEP_T_RES)) | UEP_T_RES_NAK;
                } else {
                  UEP1_CTRL = (UEP1_CTRL & ~bUEP_T_TOG) | UEP_T_RES_STALL;
                }
                break;
              default:
                len = 0xFF;
                break;
              }
            }
            break;

          case USB_GET_STATUS:
            usb_ep0_buf[0] = 0x00;
            usb_ep0_buf[1] = 0x00;
            len = (usb_setup_len >= 2) ? 2 : usb_setup_len;
            break;

          default:
            len = 0xFF;
            break;
          }
        } else {
          len = 0xFF;  // Unsupported request type
        }
      } else {
        len = 0xFF;  // Malformed SETUP
      }

      if (len == 0xFF) {
        usb_setup_req = 0xFF;
        UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL;
      } else if (len <= DEFAULT_ENDP0_SIZE) {
        UEP0_T_LEN = len;
        UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
      } else {
        UEP0_T_LEN = 0;
        UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
      }
      break;

    case (UIS_TOKEN_IN | 0):                         // EP0 IN
      switch (usb_setup_req) {
      case USB_GET_DESCRIPTOR:
      case HID_GET_REPORT:
        len = (usb_setup_len >= DEFAULT_ENDP0_SIZE) ? DEFAULT_ENDP0_SIZE : usb_setup_len;
        memcpy(usb_ep0_buf, descr_ptr, len);
        usb_setup_len -= len;
        descr_ptr += len;
        UEP0_T_LEN = len;
        UEP0_CTRL ^= bUEP_T_TOG;
        break;

      case USB_SET_ADDRESS:
        USB_DEV_AD = (USB_DEV_AD & bUDA_GP_BIT) | usb_setup_len;
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        break;

      default:
        UEP0_T_LEN = 0;
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        break;
      }
      break;

    case (UIS_TOKEN_OUT | 0):                        // EP0 OUT
      if (usb_setup_req == HID_SET_REPORT) {
        if (U_TOG_OK && USB_RX_LEN >= 2) {
          // Received command + data from host
          hid_cmd = usb_ep0_buf[0];       // Command byte
          hid_cmd_data = usb_ep0_buf[1];  // Data byte
          hid_cmd_received = 1;           // Set flag
          UEP0_T_LEN = 0;
          UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_ACK;  // Send ZLP
        }
      } else {
        UEP0_T_LEN = 0;
        UEP0_CTRL |= UEP_R_RES_ACK | UEP_T_RES_NAK;
      }
      break;

    default:
      break;
    }
    UIF_TRANSFER = 0;
  }

  if (UIF_BUS_RST) {
    UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
    UEP1_CTRL = bUEP_AUTO_TOG | UEP_T_RES_NAK;
    USB_DEV_AD = 0x00;

    UIF_SUSPEND = 0;
    UIF_TRANSFER = 0;
    UIF_BUS_RST = 0;

    usb_config = 0;
    hid_cmd_received = 0;
    hid_cmd = 0;
    hid_cmd_data = 0;
  }

  if (UIF_SUSPEND) {
    UIF_SUSPEND = 0;
    if (USB_MIS_ST & bUMS_SUSPEND) {
      // Enter suspend mode
      SAFE_MOD = 0x55; SAFE_MOD = 0xAA;
      WAKE_CTRL = bWAK_BY_USB;
      PCON |= PD;
      SAFE_MOD = 0x55; SAFE_MOD = 0xAA;
      WAKE_CTRL = 0x00;
    }
  } else {
    USB_INT_FG = 0xFF;
  }
}