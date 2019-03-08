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

#ifndef GODOT_PSD_IMPORTER_H
#define GODOT_PSD_IMPORTER_H

#include <gdnative_api_struct.gen.h>

struct godot_psdimporter {
	int version;
	GDCALLINGCONV void * (*constructor) (godot_object *p_instance, void *p_method_data);
	GDCALLINGCONV void (*destructor) (godot_object *p_instance, void *p_method_data, void *p_user_data);

	GDCALLINGCONV godot_variant (*file_load)(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
	GDCALLINGCONV godot_variant (*file_close) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);

	GDCALLINGCONV godot_variant (*get_layer_count) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
	GDCALLINGCONV godot_variant (*get_layers) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);

	GDCALLINGCONV godot_variant (*is_sprite_frames) (godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args);
};

extern const struct godot_psdimporter godot_psdimporter;

#endif // GODOT_PSD_IMPORTER_H
