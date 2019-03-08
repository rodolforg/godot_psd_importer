/**
* Godot PSD importer
*   Adding PSD importer module to Godot Engine
* Copyright (c) 2018 Rodolfo Ribeiro Gomes
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#include <gdnative_api_struct.gen.h>
#include "psd_importer.h"

const godot_gdnative_core_api_struct *api = NULL;
const godot_gdnative_ext_nativescript_api_struct *nativescript_api = NULL;

static GDCALLINGCONV godot_variant get_version(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);

void GDN_EXPORT godot_gdnative_init(godot_gdnative_init_options *p_options) {
	api = p_options->api_struct;

	// now find our extensions
	for (int i = 0; i < api->num_extensions; i++) {
		switch (api->extensions[i]->type) {
		case GDNATIVE_EXT_NATIVESCRIPT:
				nativescript_api = (godot_gdnative_ext_nativescript_api_struct *)api->extensions[i];
			break;
		default:
			break;
		};
	};	
}

void GDN_EXPORT godot_gdnative_terminate(godot_gdnative_terminate_options *p_options) {
	api = NULL;
	nativescript_api = NULL;
}

void GDN_EXPORT godot_nativescript_init(void *p_handle) {
	godot_instance_create_func create = { NULL, NULL, NULL };
	create.create_func = godot_psdimporter.constructor;

	godot_instance_destroy_func destroy = { NULL, NULL, NULL };
	destroy.destroy_func = godot_psdimporter.destructor;

	nativescript_api->godot_nativescript_register_tool_class(p_handle, "PsdImporter", "Reference", create, destroy);

#define n_methods (sizeof(godot_psdimporter) / sizeof(godot_variant (*)(godot_object *, void *, void *, int , godot_variant **)) - 3)

	struct {
		GDCALLINGCONV godot_variant (*method_ptr)(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
		const char *method_name;
	} method_list[n_methods] = {
		{godot_psdimporter.file_load, "file_load"},
		{godot_psdimporter.file_close, "file_close"},
		{godot_psdimporter.get_layer_count, "get_layer_count"},
		{godot_psdimporter.get_layers, "get_layers"},
		{godot_psdimporter.is_sprite_frames, "is_sprite_frames"},
	};

	godot_instance_method method_struct = { NULL, NULL, NULL };
	godot_method_attributes attributes = { GODOT_METHOD_RPC_MODE_DISABLED };
	
	for (int i=0; i < n_methods; i++) {
		method_struct.method = method_list[i].method_ptr;
		nativescript_api->godot_nativescript_register_method(p_handle, "PsdImporter", method_list[i].method_name, attributes, method_struct);
	}
	
	method_struct.method = get_version;
	method_struct.method_data = &godot_psdimporter.version;
	nativescript_api->godot_nativescript_register_method(p_handle, "PsdImporter", "get_version", attributes, method_struct);
}

static GDCALLINGCONV godot_variant get_version(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	api->godot_variant_new_int(&ret, *(int*)p_method_data);
	return ret;
}
