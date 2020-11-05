
/* Includes ------------------------------------------------------------------*/

#include "communication.h"

#include "interface_usb.h"
#include "interface_uart.h"
#include "interface_can.hpp"
#include "interface_i2c.h"

#include "odrive_main.h"
#include "freertos_vars.h"
#include "utils.hpp"

#include <cmsis_os.h>
#include <memory>
//#include <usbd_cdc_if.h>
//#include <usb_device.h>
//#include <usart.h>
#include <gpio.h>

#include <type_traits>

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Global constant data ------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/

uint64_t serial_number;
char serial_number_str[13]; // 12 digits + null termination

float oscilloscope[OSCILLOSCOPE_SIZE] = {0};
size_t oscilloscope_pos = 0;

/* Private constant data -----------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

auto make_protocol_definitions(PWMMapping_t& mapping) {
    return make_protocol_member_list(
        make_protocol_property("pwm_type", &mapping.pwm_type),
        make_protocol_property("endpoint", &mapping.endpoint),
        make_protocol_property("min", &mapping.min),
        make_protocol_property("max", &mapping.max),
        make_protocol_property("enable_deadband", &mapping.enable_deadband),
        make_protocol_property("gpio_direction_pin", &mapping.gpio_direction_pin)
    );
}

auto make_protocol_definitions(DiffSteeringMixerMapping_t& mapping) {
    return make_protocol_member_list(
        make_protocol_property("input_throttle", &mapping.input_throttle),
        make_protocol_property("input_steering", &mapping.input_steering),
        make_protocol_property("endpoint_output_a", &mapping.endpoint_output_a),
        make_protocol_property("endpoint_output_b", &mapping.endpoint_output_b),
        make_protocol_property("direction_a", &mapping.direction_a),
        make_protocol_property("direction_b", &mapping.direction_b),
        make_protocol_property("gpio_update_trigger", &mapping.gpio_update_trigger)
    );
}

/* Function implementations --------------------------------------------------*/

void init_communication(void) {
    printf("hi!\r\n");

    if (odrv.config_.enable_uart0 && uart0) {
        start_uart_server();
    }

    start_usb_server();

    if (odrv.config_.enable_i2c0) {
        start_i2c_server();
    }

    if (odrv.config_.enable_can0) {
        odCAN->start_can_server();
    }
}

extern "C" {
int _write(int file, const char* data, int len) __attribute__((used));
}

// @brief This is what printf calls internally
int _write(int file, const char* data, int len) {
#ifdef USB_PROTOCOL_STDOUT
    usb_stream_output_ptr->process_bytes((const uint8_t *)data, len, nullptr);
#endif
#ifdef UART_PROTOCOL_STDOUT
    uart_stream_output_ptr->process_bytes((const uint8_t *)data, len, nullptr);
#endif
    return len;
}


#include "../autogen/function_stubs.hpp"

ODrive& ep_root = odrv;
#include "../autogen/endpoints.hpp"
