/**
 * Copyright (C) 2013-2015
 *
 * @author ouyangchengfeng@xiaomi.com
 * @date   2018-10-29
 *
 * @file   property_changed_encoder.c
 *
 * @remark
 *
 */

#include "property_changed_encoder.h"
#include <jsmi.h>

/**
 * {
 *   "did": "57352830",
 *   "siid": 2,
 *   "piid": 1,
 *   "value": 232
 * }
 */

int property_changed_encode(property_operation_t *o, char out[], size_t size)
{
    size_t length = 0;
    jsmi_composer_t composer = JSMI_COMPOSER_INIT(out, size);
    jsmi_compose_start(&composer);

    jsmi_set_array_begin(&composer);

    jsmi_set_value_object_begin(&composer);

    jsmi_set_key(&composer, "did");
    jsmi_set_value_str(&composer, o->did, 0);

    jsmi_set_key(&composer, "siid");
    jsmi_set_value_uint(&composer, o->siid);

    jsmi_set_key(&composer, "piid");
    jsmi_set_value_uint(&composer, o->piid);

    jsmi_set_key(&composer, "value");
    switch (o->value->format)
    {
        case PROPERTY_FORMAT_BOOLEAN:
            jsmi_set_value_bool(&composer, o->value->data.boolean.value);
            break;

        case PROPERTY_FORMAT_STRING:
            jsmi_set_value_str(&composer, o->value->data.string.value, 0);
            break;

        case PROPERTY_FORMAT_NUMBER:
            if (o->value->data.number.type == DATA_NUMBER_INTEGER)
            {
                jsmi_set_value_sint(&composer, o->value->data.number.value.integerValue);
            }
            else
            {
                jsmi_set_value_sint(&composer, o->value->data.number.value.integerValue);
                // jsmi_set_value_float(&composer, o->value->data.number.value.floatValut);
            }
            break;

        default:
            break;
    }

    jsmi_set_value_object_end(&composer);

    jsmi_set_array_end(&composer);

    if (jsmi_compose_finish(&composer, NULL, &length) == MIIO_OK)
    {
        return length;
    }

    return 0;
}