#include "psd_layer.h"

#include "libpsd.h"

const char * psd_layer_name(const psd_layer_record * record) {
	return record->layer_name;
}

int psd_layer_is_group(const psd_layer_record * record) {
	return record->layer_type == psd_layer_type_folder;
}

int psd_layer_x(const psd_layer_record * record) {
	return record->left;
}

int psd_layer_y(const psd_layer_record * record) {
	return record->top;
}

int psd_layer_width(const psd_layer_record * record) {
	return record->width;
}

int psd_layer_height(const psd_layer_record * record) {
	return record->height;
}

const char * psd_layer_image_data(const psd_layer_record * record) {
	return record->image_data;
}
