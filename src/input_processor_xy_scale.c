/* SPDX-License-Identifier: MIT
 *
 * Simple XY scale input-processor for REL_X/REL_Y devices.
 * Property: scale-q8 (Q8 fixed-point, default 256 == 1.0)
 */

#define DT_DRV_COMPAT zmk_input_processor_xy_scale

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <drivers/input_processor.h>
#include <zephyr/logging/log.h>
#include <zephyr/dt-bindings/input/input-event-codes.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define Q8_ONE 256

struct xy_scale_config {
    int32_t scale_q8;
};

struct xy_scale_data {
    const struct device *proc_dev;
};

static int xy_scale_handle_event(const struct device *dev, struct input_event *event,
                                 uint32_t param1, uint32_t param2,
                                 struct zmk_input_processor_state *state)
{
    ARG_UNUSED(param1);
    ARG_UNUSED(param2);
    ARG_UNUSED(state);

    if (event->type != INPUT_EV_REL) {
        return ZMK_INPUT_PROC_CONTINUE;
    }
    if (event->code != INPUT_REL_X && event->code != INPUT_REL_Y) {
        return ZMK_INPUT_PROC_CONTINUE;
    }

    const struct xy_scale_config *cfg = dev->config;
    int32_t v = (int32_t)event->value;
    v = (v * cfg->scale_q8) / Q8_ONE;
    event->value = v;

    return ZMK_INPUT_PROC_CONTINUE;
}

static int xy_scale_init(const struct device *dev)
{
    struct xy_scale_data *data = dev->data;
    data->proc_dev = dev;
    return 0;
}

static const struct zmk_input_processor_driver_api xy_scale_driver_api = {
    .handle_event = xy_scale_handle_event,
};

#define XY_SCALE_INST(n) \
    static struct xy_scale_data xy_scale_data_##n = {}; \
    static const struct xy_scale_config xy_scale_config_##n = { \
        .scale_q8 = DT_INST_PROP_OR(n, scale_q8, 256), \
    }; \
    DEVICE_DT_INST_DEFINE(n, xy_scale_init, NULL, \
                          &xy_scale_data_##n, &xy_scale_config_##n, \
                          POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
                          &xy_scale_driver_api);

DT_INST_FOREACH_STATUS_OKAY(XY_SCALE_INST)
