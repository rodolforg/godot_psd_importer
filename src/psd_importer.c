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

#include "register_types.h"
#include "psd_importer.h"
#include <string.h>

#include "psd_parser.h"

typedef struct {
	godot_string filename;
	struct psd_document * doc;
} data_struct;

static GDCALLINGCONV void * constructor(godot_object *p_instance, void *p_method_data) {
	data_struct *data = api->godot_alloc(sizeof(data_struct));
	api->godot_string_new(&data->filename);
	data->doc = NULL;

	return data;
}

static GDCALLINGCONV void destructor(godot_object *p_instance, void *p_method_data, void *p_user_data) {
	data_struct *data = (data_struct *) p_user_data;

	api->godot_string_destroy(&data->filename);

	if (data->doc)
		psd_document_free(data->doc);

	api->godot_free(p_user_data);
}

static GDCALLINGCONV godot_variant file_load(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	bool success = false;

	if (p_num_args == 1) {
		api->godot_string_destroy(&user_data->filename);
		user_data->filename = api->godot_variant_as_string(p_args[0]);

		godot_char_string cstr = api->godot_string_utf8(&user_data->filename);
		const char * filename = api->godot_char_string_get_data(&cstr);

		struct psd_parser * parser = psd_parser_new(filename);
		api->godot_char_string_destroy(&cstr);

		if (parser) {
			user_data->doc = psd_parser_parse(parser);
		}
		psd_parser_free(parser);

		success = user_data->doc != NULL;
	}
	
	api->godot_variant_new_bool(&ret, success);
	return ret;
}

static GDCALLINGCONV godot_variant file_close(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;

	api->godot_string_destroy(&user_data->filename);
	api->godot_string_new(&user_data->filename);

	psd_document_free(user_data->doc);
	user_data->doc = NULL;

	api->godot_variant_new_bool(&ret, true);
	return ret;
}

static GDCALLINGCONV godot_variant get_layer_count(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	if (!user_data || !user_data->doc) {
		api->godot_variant_new_nil(&ret);
	} else {
		api->godot_variant_new_int(&ret, psd_document_children_count(user_data->doc));
	}
	return ret;
}

static GDCALLINGCONV godot_variant extract_psd(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;

	if (!user_data || !user_data->doc || p_num_args != 1) {
		api->godot_variant_new_bool(&ret, false);
		return ret;
	}

	bool success = false;

	if (api->godot_variant_get_type(p_args[0]) == GODOT_VARIANT_TYPE_STRING) {
		godot_string dir_str = api->godot_variant_as_string(p_args[0]);
		godot_char_string cstr = api->godot_string_utf8(&dir_str);
		const char * dir = api->godot_char_string_get_data(&cstr);

		success = true;
		psd_document_save_layers(user_data->doc, dir);

		api->godot_char_string_destroy(&cstr);
		api->godot_string_destroy(&dir_str);
	}
	api->godot_variant_new_bool(&ret, success);
	return ret;
}

static bool _is_sprite_frames(struct psd_document * doc) {
	if (doc == NULL)
		return false;

	bool success = true;

	struct psd_record * record = psd_document_first_child(doc);
	while (record) {
		if (record->is_group) {
			struct psd_layer_group * group = record->data.group;
			struct psd_record * nested_record = psd_layer_group_first_child(group);
			while (nested_record) {
				if (nested_record->is_group) {
					success = false;
					psd_record_free(nested_record);
					break;
				}
				struct psd_record * next_nested_record = psd_layer_group_next_child(group);
				psd_record_free(nested_record);
				nested_record = next_nested_record;
			}
			if (!success) {
				psd_record_free(record);
				break;
			}
		}
		struct psd_record * next_record = psd_document_next_child(doc);
		psd_record_free(record);
		record = next_record;
	}
	return success;
}

static GDCALLINGCONV godot_variant is_sprite_frames(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	if (!user_data || !user_data->doc || p_num_args != 0) {
		api->godot_variant_new_bool(&ret, false);
		return ret;
	}

	bool success = _is_sprite_frames(user_data->doc);

	api->godot_variant_new_bool(&ret, success);
	return ret;
}

static bool _get_sprite_frame_names(struct psd_document * doc, godot_dictionary * dict) {
	if (doc == NULL || dict == NULL)
		return false;
	
	if (!_is_sprite_frames(doc))
		return false;

	bool success = true;
	
	api->godot_dictionary_new(dict);
	
	struct psd_record * animation_record = psd_document_first_child(doc);
	while (animation_record) {
		
		if (animation_record->is_group) {
			struct psd_layer_group * group = animation_record->data.group;
			struct psd_record * frame_record = psd_layer_group_first_child(group);
			godot_pool_string_array array;
			api->godot_pool_string_array_new(&array);
			//api->godot_pool_string_array_resize(&array, psd_layer_group_children_count(group));
			while (frame_record) {
				godot_string string;
				api->godot_string_new(&string);
				api->godot_string_parse_utf8(&string, psd_record_name(frame_record));
				api->godot_pool_string_array_push_back(&array, &string);
				api->godot_string_destroy(&string);

				struct psd_record * next_frame_record = psd_layer_group_next_child(group);
				psd_record_free(frame_record);
				if (!success)
					break;
				frame_record = next_frame_record;
			}

			if (!success) {
				api->godot_pool_string_array_destroy(&array);
				psd_record_free(animation_record);
				break;
			}

			godot_string key_str;
			api->godot_string_new(&key_str);
			api->godot_string_parse_utf8(&key_str, psd_record_name(animation_record));

			godot_variant key;
			api->godot_variant_new_string(&key, &key_str);

			godot_variant value;
			api->godot_variant_new_pool_string_array(&value, &array);

			api->godot_dictionary_set(dict, &key, &value);
			api->godot_string_destroy(&key_str);
			api->godot_pool_string_array_destroy(&array);
			api->godot_variant_destroy(&value);
			api->godot_variant_destroy(&key);
		}
		struct psd_record * next_animation_record = psd_document_next_child(doc);
		psd_record_free(animation_record);
		animation_record = next_animation_record;
	}
	if (!success)
		api->godot_dictionary_destroy(dict);
	return success;

}

static GDCALLINGCONV godot_variant get_sprite_frame_names(godot_object *p_instance, void *p_method_data, void *p_user_data, int p_num_args, godot_variant **p_args) {
	godot_variant ret;
	data_struct * user_data = (data_struct *) p_user_data;
	
	if (!user_data || !user_data->doc || p_num_args != 0) {
		api->godot_variant_new_bool(&ret, false);
		return ret;
	}

	godot_dictionary dict;
	bool success = _get_sprite_frame_names(user_data->doc, &dict);
	
	if (!success)
		api->godot_variant_new_bool(&ret, false);
	else {
		api->godot_variant_new_dictionary(&ret, &dict);
		api->godot_dictionary_destroy(&dict);
	}
	return ret;
}



const struct godot_psdimporter godot_psdimporter = {0x01,
                                                      constructor, destructor,
                                                      file_load, file_close,
                                                      get_layer_count,
                                                      extract_psd,
                                                      is_sprite_frames, get_sprite_frame_names,
                                                      };
