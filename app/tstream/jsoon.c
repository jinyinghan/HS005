// $Id: jsoon.c 12079 2011-04-11 05:05:55Z cedric.shih $
/*
 * Copyright (c) 2007-2008 Mantaray Technology, Incorporated.
 * Rm. A407, No.18, Si Yuan Street, Taipei, 100, Taiwan.
 * Phone: +886-2-23681570. Fax: +886-2-23682417.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are NOT permitted without specific written permission
 * from above copyright holder.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY MANTARAY TECHNOLOGY INCORPORATED
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * MANTARAY TECHNOLOGY INCORPORATED BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 */

#include <errno.h>
#include <string.h>

#include "jsoon.h"




static int jsoon_new_obj(enum json_type type, const void *value, struct json_object **obj);

static int jsoon_add(struct json_object *obj, const char *name,
		enum json_type type, const void *value,
		struct json_object **child);

const char *jsoon_strerror(enum jsoon_errno err)
{
	switch (err) {
	case JSOON_EOK:
		return "Success.";
	case JSOON_EINVAL:
		return "Invalid argument(s).";
	case JSOON_ENOMEM:
		return "Out of memory.";
	case JSOON_ENOTOBJ:
		return "Not an object type json object.";
	case JSOON_ENOATTR:
		return "No such attribute.";
	case JSOON_ETYPE:
		return "Type mismatch.";
	case JSOON_EEXIST:
		return "Attribute already exists.";
	case JSOON_EOVERFLOW:
		return "Buffer overflows.";
	case JSOON_ENOTARR:
		return "Not an array type json object.";
	case JSOON_ERANGE:
		return "Array index out of bound.";
	case JSOON_EUNKNOWN:
		return "Unexpected error.";
	default:
		return "Unknown error.";
	}
}

int jsoon_new(struct json_object **obj)
{
	if (!obj) {
		return JSOON_EINVAL;
	}

	if (!(*obj = json_object_new_object())) {
		jsoon_error("failed to create json object");
		return JSOON_ENOMEM;
	}

	return 0;
}

void jsoon_free(struct json_object *obj)
{
	if (obj != NULL) {
		json_object_put(obj);
	}
}

int jsoon_find(const struct json_object *obj,
		const char *name, enum json_type type,
		struct json_object **attr, void *value)
{
	int rc;

	if (!obj) {
		jsoon_error("null json object");
		rc = JSOON_EINVAL;
		goto finally;
	}

	if (!name) {
		jsoon_error("null property name");
		rc = JSOON_EINVAL;
		goto finally;
	}

	if (!json_object_is_type((struct json_object *)obj,
			json_type_object)) {
		jsoon_error("not an object type json object");
		rc = JSOON_ENOTOBJ;
		goto finally;
	}

	jsoon_trace("finding \"%s\" type property \"%s\" from %s",
			jsoon_type2str(type), name,
			json_object_to_json_string((struct json_object *)obj));

	if (!(*attr = json_object_object_get(
			(struct json_object*)obj, (char*) name))) {
		rc = 0;
		goto finally;
	}

	if (!json_object_is_type(*attr, type)) {
		jsoon_error("type of property \"%s\" doesn't match: "
				"%s != %s", name, jsoon_type2str(type),
				jsoon_type2str(json_object_get_type(*attr)));
		rc = JSOON_ETYPE;
		goto finally;
	}

	switch (type) {
	case json_type_array:
		if (value != NULL) {
			*((unsigned *)value) = json_object_array_length(*attr);
		}
		break;
	case json_type_int:
		if (value != NULL) {
			*((int *)value) = json_object_get_int(*attr);
		}
		break;
	case json_type_double:
		if (value != NULL) {
			*((double *)value) = json_object_get_int(*attr);
		}
		break;
	case json_type_boolean:
		if (value != NULL) {
			*((int *)value) = json_object_get_boolean(*attr);
		}
		break;
	case json_type_string:
		if (value != NULL) {
			*((const char **)value) = json_object_get_string(*attr);
		}
		break;
	case json_type_object:
		break;
	default:
		jsoon_error("unsupported json type: %d", type);
		rc = JSOON_EUNKNOWN;
		goto finally;
	}
	rc = 0;

finally:
	if (rc) {
		jsoon_error("failed to get \"%s\" type "
			"attribute \"%s\"", jsoon_type2str(type), name);
	}
	return rc;
}

int jsoon_get(const struct json_object *obj,
		const char *name, enum json_type type,
		struct json_object **attr, void *value)
{
	int rc;

	if ((rc = jsoon_find(obj, name, type, attr, value))) {
		goto finally;
	}

	if (!(*attr)) {
		jsoon_error("property not found: %s", name);
		jsoon_trace("property not found in json object: %s",
				json_object_to_json_string(
						(struct json_object *)obj));
		rc = JSOON_ENOATTR;
		goto finally;
	}

finally:
	return rc;
}

int jsoon_get_obj(const struct json_object *obj, const char *name,
		struct json_object **ptr)
{
	return jsoon_get(obj, name, json_type_object, ptr, NULL);
}

int jsoon_get_array(const struct json_object *obj, const char *name,
		struct json_object **array, unsigned int *len)
{
	return jsoon_get(obj, name, json_type_array, array, len);
}

int jsoon_get_int(const struct json_object *obj, const char *name, int *out)
{
	struct json_object *temp;
	return jsoon_get(obj, name, json_type_int, &temp, out);
}

int jsoon_get_ranged_int(const struct json_object *obj, const char *name,
		int min, int max, int *out)
{
	int rc;

	if (min > max) {
		jsoon_error("min > max: %d > %d", min, max);
		rc = JSOON_EINVAL;
		goto finally;
	}

	if ((rc = jsoon_get_int(obj, name, out))) {
		goto finally;
	}

	if (*out < min || *out > max) {
		rc = JSOON_ERANGE;
		goto finally;
	}

finally:
	return rc;
}

int jsoon_get_double(const struct json_object *obj, const char *name, double *out)
{
	struct json_object *temp;
	return jsoon_get(obj, name, json_type_double, &temp, out);
}

int jsoon_get_ranged_double(const struct json_object *obj,
		const char *name, double min, double max, double *out)
{
	int rc;

	if (min > max) {
		jsoon_error("min > max: %f > %f", min, max);
		rc = JSOON_EINVAL;
		goto finally;
	}

	if ((rc = jsoon_get_double(obj, name, out))) {
		goto finally;
	}

	if (*out < min || *out > max) {
		rc = JSOON_ERANGE;
		goto finally;
	}

finally:
	return rc;
}

int jsoon_get_bool(const struct json_object *obj, const char *name,
		int *out)
{
	struct json_object *temp;
	return jsoon_get(obj, name, json_type_boolean, &temp, out);
}

int jsoon_get_str(const struct json_object *obj, const char *name,
		const char **str)
{
	struct json_object *temp;
	return jsoon_get(obj, name, json_type_string, &temp, str);
}

int jsoon_get_strcpy(const struct json_object *obj, const char *name,
		char *buffer, size_t size)
{
	int rc;
	char *string;
	size_t len;
	struct json_object *temp;

	if (!buffer) {
		return JSOON_EINVAL;
	}

	if ((rc = jsoon_get(obj, name, json_type_string, &temp, &string))) {
		goto finally;
	}

	if ((len = strlen(string)) >= size) {
		jsoon_error("length of string property \"%s\" "
				"exceeds buffer limitation: %d >= %d",
				name, len, size);
		rc = JSOON_EOVERFLOW;
		goto finally;
	}

	memcpy(buffer, string, len + 1);
	rc = 0;

finally:
	return rc;
}

int jsoon_find_obj(const struct json_object *obj, const char *name,
		struct json_object **child)
{
	*child = NULL;
	return jsoon_find(obj, name, json_type_object, child, NULL);
}

int jsoon_find_array(const struct json_object *obj, const char *name,
		struct json_object **array, int *len)
{
	*array = NULL;
	*len = 0;
	return jsoon_find(obj, name, json_type_array, array, len);
}

int jsoon_find_int(const struct json_object *obj, const char *name,
		int preset, int *value)
{
	struct json_object *temp;
	*value = preset;
	return jsoon_find(obj, name, json_type_int, &temp, value);
}

int jsoon_find_ranged_int(const struct json_object *obj,
		const char *name, int preset, int min, int max, int *out)
{
	int rc;

	if (min > max) {
		jsoon_error("min > max: %d > %d", min, max);
		rc = JSOON_EINVAL;
		goto finally;
	}

	if ((rc = jsoon_find_int(obj, name, preset, out))) {
		goto finally;
	}

	if (*out < min || *out > max) {
		rc = JSOON_ERANGE;
		goto finally;
	}

finally:
	return rc;
}

int jsoon_find_double(const struct json_object *obj, const char *name,
		double preset, double *value)
{
	struct json_object *temp;
	*value = preset;
	return jsoon_find(obj, name, json_type_double, &temp, value);
}

int jsoon_find_ranged_double(const struct json_object *obj,
		const char *name, double preset, double min, double max,
		double *out)
{
	int rc;

	if (min > max) {
		jsoon_error("min > max: %f > %f", min, max);
		rc = JSOON_EINVAL;
		goto finally;
	}

	if ((rc = jsoon_find_double(obj, name, preset, out))) {
		goto finally;
	}

	if (*out < min || *out > max) {
		rc = JSOON_ERANGE;
		goto finally;
	}

finally:
	return rc;
}

int jsoon_find_bool(const struct json_object *obj, const char *name,
		int preset, int *value)
{
	struct json_object *temp;
	*value = preset;
	return jsoon_find(obj, name, json_type_boolean, &temp, value);
}

int jsoon_find_str(const struct json_object *obj, const char *name,
		const char *preset, const char **string)
{
	struct json_object *temp;
	*string = preset;
	return jsoon_find(obj, name, json_type_string, &temp, string);
}

int jsoon_find_strcpy(const struct json_object *obj, const char *name,
		const char *preset, char *buffer, size_t size)
{
	int rc;
	const char *string;
	size_t len;
	struct json_object *temp;

	if (!buffer) {
		return JSOON_EINVAL;
	}

	string = preset;

	if ((rc = jsoon_find(obj, name, json_type_string, &temp, &string))) {
		goto finally;
	}

	if ((len = strlen(string)) >= size) {
		jsoon_error("length of string property \"%s\" "
				"exceeds buffer limitation: %d >= %d",
				name, len, size);
		rc = JSOON_EOVERFLOW;
		goto finally;
	}

	memcpy(buffer, string, len + 1);
	rc = 0;

finally:
	return rc;
}

int jsoon_new_obj(enum json_type type, const void *value, struct json_object **obj)
{
	int rc;

	if (!obj) {
		rc = JSOON_EINVAL;
		goto finally;
	}

	switch (type) {
	case json_type_object:
		*obj = json_object_new_object();
		break;
	case json_type_array:
		*obj = json_object_new_array();
		break;
	case json_type_int:
		*obj = json_object_new_int(*((int *)value));
		break;
	case json_type_double:
		*obj = json_object_new_double(*((double *)value));
		break;
	case json_type_boolean:
		*obj = json_object_new_boolean(*((int *)value));
		break;
	case json_type_string:
		if (!value) {
			*obj = NULL;
		} else {
			*obj = json_object_new_string((char *)value);
		}
		break;
	default:
		jsoon_error("unexpected json type: %d", type);
		rc = JSOON_EUNKNOWN;
		goto finally;
	}

	if (value && *obj == NULL) {
		jsoon_error("failed to create json object");
		rc = JSOON_ENOMEM;
		goto finally;
	}

	rc = 0;

finally:
	if (rc != 0) {
		jsoon_error("failed to create json object of type: %s",
				jsoon_type2str(type));
	}
	return rc;
}

int jsoon_add(struct json_object *obj, const char *name,
		enum json_type type, const void *value,
		struct json_object **child)
{
	int rc;

	if (!obj) {
		rc = JSOON_EINVAL;
		goto finally;
	}

	if ((rc = jsoon_find(obj, name, type, child, NULL))) {
		goto finally;
	}

	if ((*child) != NULL) {
		jsoon_error("attribute already exists: %s", name);
		rc = JSOON_EEXIST;
		goto finally;
	}

	if ((rc = jsoon_new_obj(type, value, child))) {
		jsoon_error("failed to add attribute to json object: %s", name);
		goto finally;
	}

	json_object_object_add(obj, (char *)name, *child);
	rc = 0;

finally:
	return rc;
}

int jsoon_add_obj(struct json_object *obj, const char *name,
		struct json_object **child)
{
	return jsoon_add(obj, name, json_type_object, NULL, child);
}

int jsoon_add_array(struct json_object *obj, const char *name,
		struct json_object **array)
{
	return jsoon_add(obj, name, json_type_array, NULL, array);
}

int jsoon_add_int(struct json_object *obj, const char *name, int value)
{
	struct json_object *temp;
	return jsoon_add(obj, name, json_type_int, &value, &temp);
}

int jsoon_add_double(struct json_object *obj, const char *name, double value)
{
	struct json_object *temp;
	return jsoon_add(obj, name, json_type_double, &value, &temp);
}

int jsoon_add_bool(struct json_object *obj, const char *name, int bool)
{
	struct json_object *temp;
	return jsoon_add(obj, name, json_type_boolean, &bool, &temp);
}

int jsoon_add_str(struct json_object *obj, const char *name, const char *str)
{
	struct json_object *temp;
	return jsoon_add(obj, name, json_type_string, str, &temp);
}

int jsoon_array_new(struct json_object **array)
{
	if (!array) {
		return JSOON_EINVAL;
	}

	if (!(*array = json_object_new_array())) {
		jsoon_error("failed to create json array");
		return JSOON_ENOMEM;
	}

	return 0;
}

int jsoon_array_add(struct json_object *obj, enum json_type type,
		const void *value, struct json_object **child)
{
	int rc;

	if (!obj) {
		rc = JSOON_EINVAL;
		goto finally;
	}

	if (!json_object_is_type((struct json_object *)obj,
			json_type_array)) {
		jsoon_error("not an array type json object");
		rc = JSOON_ENOTARR;
		goto finally;
	}

	jsoon_trace("creating json object into json array");

	if ((rc = jsoon_new_obj(type, value, child)) != 0) {
		goto finally;
	}

	if ((rc = json_object_array_add(obj, *child)) != 0) {
		json_object_put(*child);
		rc = JSOON_EUNKNOWN;
	}

finally:
	if (rc != 0) {
		jsoon_error("failed to create json object into json array");
	}
	return rc;
}

int jsoon_array_add_obj(struct json_object *obj, struct json_object **child)
{
	return jsoon_array_add(obj, json_type_object, NULL, child);
}

int jsoon_array_add_array(struct json_object *json_array, struct json_object **array)
{
	return jsoon_array_add(json_array, json_type_array, NULL, array);
}

int jsoon_array_add_str(struct json_object *json_array, const char *str)
{
	struct json_object *temp;
	return jsoon_array_add(json_array, json_type_string, str, &temp);
}

int jsoon_array_add_int(struct json_object *json_array, int value)
{
	struct json_object *temp;
	return jsoon_array_add(json_array, json_type_int, &value, &temp);
}

int jsoon_array_add_double(struct json_object *json_array, double value)
{
	struct json_object *temp;
	return jsoon_array_add(json_array, json_type_double, &value, &temp);
}

int jsoon_array_add_bool(struct json_object *json_array, int bool)
{
	struct json_object *temp;
	return jsoon_array_add(json_array, json_type_boolean, &bool, &temp);
}

inline int jsoon_array_get(struct json_object *array, unsigned int idx,
		enum json_type type, struct json_object **elm)
{
	int rc;

	if (!array) {
		rc = JSOON_EINVAL;
		goto finally;
	}

	if (!json_object_is_type((struct json_object *)array,
			json_type_array)) {
		jsoon_error("not an array type json object");
		rc = JSOON_ENOTARR;
		goto finally;
	}

	*elm = json_object_array_get_idx(array, idx);

	if (!(*elm)) {
		rc = JSOON_ERANGE;
		goto finally;
	}

	if (!json_object_is_type(*elm, type)) {
		rc = JSOON_ETYPE;
		goto finally;
	}

	rc = 0;

finally:
	return rc;
}

int jsoon_array_get_obj(struct json_object *array, unsigned int idx,
		struct json_object **elm)
{
	return jsoon_array_get(array, idx, json_type_object, elm);
}

int jsoon_array_get_array(struct json_object *array, unsigned int idx,
		struct json_object **elm)
{
	return jsoon_array_get(array, idx, json_type_array, elm);
}

int jsoon_array_get_str(struct json_object *array, unsigned int idx,
		const char **value)
{
	int rc;
	struct json_object *elm;

	if ((rc = jsoon_array_get(array, idx, json_type_string, &elm))) {
		goto finally;
	}

	*value = json_object_get_string(elm);

finally:
	return rc;
}

int jsoon_array_get_int(struct json_object *array, unsigned int idx,
		int *value)
{
	int rc;
	struct json_object *elm;

	if ((rc = jsoon_array_get(array, idx, json_type_int, &elm))) {
		goto finally;
	}

	*value = json_object_get_int(elm);

finally:
	return rc;
}

int jsoon_array_get_double(struct json_object *array, unsigned int idx,
		double *value)
{
	int rc;
	struct json_object *elm;

	if ((rc = jsoon_array_get(array, idx, json_type_double, &elm))) {
		goto finally;
	}

	*value = json_object_get_double(elm);

finally:
	return rc;
}

int jsoon_array_get_bool(struct json_object *array, unsigned int idx,
		int *value)
{
	int rc;
	struct json_object *elm;

	if ((rc = jsoon_array_get(array, idx, json_type_boolean, &elm))) {
		goto finally;
	}

	*value = json_object_get_boolean(elm);

finally:
	return rc;
}

const char *jsoon_to_str(const struct json_object *json)
{
	return json_object_to_json_string((struct json_object *)json);
}

const char *jsoon_type2str(enum json_type type)
{
	switch (type) {
	case json_type_null:
		return "null";
	case json_type_boolean:
		return "boolean";
	case json_type_double:
		return "double";
	case json_type_int:
		return "int";
	case json_type_object:
		return "object";
	case json_type_array:
		return "array";
	case json_type_string:
		return "string";
	default:
		return "unknown";
	}
}
