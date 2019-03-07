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

#include "psd_document.h"

typedef struct {
	godot_string filename;
	psd_document * doc;
} data_struct;

static GDCALLINGCONV void * constructor(godot_object *p_instance, void *p_method_data) {
	data_struct *data = api->godot_alloc(sizeof(data_struct));
	api->godot_string_new(&data->filename);
	data->doc = psd_document_new();
	if (data->doc == NULL) {
		api->godot_string_destroy(&data->filename);
		api->godot_free(data);
		return NULL;
	}

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

		int ok = psd_document_load(user_data->doc, filename);
		api->godot_char_string_destroy(&cstr);

		if (ok) {
			ok = psd_document_parse(user_data->doc);
		}

		success = ok != 0;
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
/*		psd_document_save_layers(user_data->doc, dir);*/

		api->godot_char_string_destroy(&cstr);
		api->godot_string_destroy(&dir_str);
	}
	api->godot_variant_new_bool(&ret, success);
	return ret;
}

static int _check_sprite_levels(const psd_layer_record * layer, void * cb_data, int level) {
	bool * ok = (bool*) cb_data;
	if (level >= 2) {
		*ok = false;
		return 0;
	}
	if (level == 1 && psd_layer_is_group(layer)) {
		*ok = false;
		return 0;
	}
	return 1;
}

static bool _is_sprite_frames(struct psd_document * doc) {
	if (doc == NULL)
		return false;

	bool success = true;
	
	psd_document_foreach_layer_level(doc, _check_sprite_levels, &success);

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

struct sprite_anim_names {
	godot_dictionary * dict;
	const char * current_animation_name;
	godot_array frame_info_array;
};

static void _push_animation_names(struct sprite_anim_names * sprite_anim_names) {
	godot_string key_str;
	api->godot_string_new(&key_str);
	api->godot_string_parse_utf8(&key_str, sprite_anim_names->current_animation_name);

	godot_variant key;
	api->godot_variant_new_string(&key, &key_str);

	godot_variant value;
	api->godot_variant_new_array(&value, &sprite_anim_names->frame_info_array);

	api->godot_dictionary_set(sprite_anim_names->dict, &key, &value);
	api->godot_string_destroy(&key_str);
	api->godot_array_destroy(&sprite_anim_names->frame_info_array);
	api->godot_variant_destroy(&value);
	api->godot_variant_destroy(&key);
}

static int _fetch_animation_frame_names(const psd_layer_record * layer, void * cb_data, int level) {
	struct sprite_anim_names * sprite_anim_names = (struct sprite_anim_names*) cb_data;

	if (level == 0) {
		if (sprite_anim_names->current_animation_name != NULL) {
			_push_animation_names(sprite_anim_names);
			sprite_anim_names->current_animation_name = NULL;
		}
		if (psd_layer_is_group(layer)) {
			sprite_anim_names->current_animation_name = psd_layer_name(layer);
			api->godot_array_new(&sprite_anim_names->frame_info_array);
		}
	} else if (level == 1) {
		godot_string string;
		api->godot_string_new(&string);
		api->godot_string_parse_utf8(&string, psd_layer_name(layer));
		
		godot_dictionary dict;
		api->godot_dictionary_new(&dict);
		
		// name
		
		godot_string name_key_str;
		api->godot_string_new(&name_key_str);
		api->godot_string_parse_utf8(&name_key_str, "name");
		godot_variant name_key;
		api->godot_variant_new_string(&name_key, &name_key_str);
		api->godot_string_destroy(&name_key_str);
		
		godot_string name_value_str;
		api->godot_string_new(&name_value_str);
		api->godot_string_parse_utf8(&name_value_str, psd_layer_name(layer));
		godot_variant name_value;
		api->godot_variant_new_string(&name_value, &name_value_str);
		api->godot_string_destroy(&name_value_str);
		
		api->godot_dictionary_set(&dict, &name_key, &name_value);
		api->godot_variant_destroy(&name_key);
		api->godot_variant_destroy(&name_value);
		
		// width
		
		godot_string width_key_str;
		api->godot_string_new(&width_key_str);
		api->godot_string_parse_utf8(&width_key_str, "width");
		godot_variant width_key;
		api->godot_variant_new_string(&width_key, &width_key_str);
		api->godot_string_destroy(&width_key_str);

		godot_variant width_value;
		int64_t width = psd_layer_width(layer);
		api->godot_variant_new_int(&width_value, width);
		
		api->godot_dictionary_set(&dict, &width_key, &width_value);
		api->godot_variant_destroy(&width_key);
		api->godot_variant_destroy(&width_value);

		// height
		
		godot_string height_key_str;
		api->godot_string_new(&height_key_str);
		api->godot_string_parse_utf8(&height_key_str, "height");
		godot_variant height_key;
		api->godot_variant_new_string(&height_key, &height_key_str);
		api->godot_string_destroy(&height_key_str);
		
		godot_variant height_value;
		int64_t height = psd_layer_height(layer);
		api->godot_variant_new_int(&height_value, height);
		
		api->godot_dictionary_set(&dict, &height_key, &height_value);
		api->godot_variant_destroy(&height_key);
		api->godot_variant_destroy(&height_value);

		// image buffer
		
		godot_string image_key_str;
		api->godot_string_new(&image_key_str);
		api->godot_string_parse_utf8(&image_key_str, "image buffer");
		godot_variant image_key;
		api->godot_variant_new_string(&image_key, &image_key_str);
		api->godot_string_destroy(&image_key_str);
		
		godot_variant image_value;
		godot_pool_byte_array image_buffer;
		const char * image_data = psd_layer_image_data(layer);
		int buffer_size = width * height * 4;
		api->godot_pool_byte_array_new(&image_buffer);
		for (int i = 0; i < buffer_size; i++)
			api->godot_pool_byte_array_append(&image_buffer, image_data[i]);
		api->godot_variant_new_pool_byte_array(&image_value, &image_buffer);
		api->godot_pool_byte_array_destroy(&image_buffer);
		
		api->godot_dictionary_set(&dict, &image_key, &image_value);
		api->godot_variant_destroy(&image_key);
		api->godot_variant_destroy(&image_value);

		// everything
		godot_variant dict_var;
		api->godot_variant_new_dictionary(&dict_var, &dict);
		api->godot_array_append(&sprite_anim_names->frame_info_array, &dict_var);
		api->godot_dictionary_destroy(&dict);
		api->godot_variant_destroy(&dict_var);
	} else
		return 0;
	return 1;
}

static bool _get_sprite_frame_names(struct psd_document * doc, godot_dictionary * dict) {
	if (doc == NULL || dict == NULL)
		return false;
	
	if (!_is_sprite_frames(doc))
		return false;

	bool success = true;
	
	api->godot_dictionary_new(dict);
	
	struct sprite_anim_names sprite_anim_names;
	sprite_anim_names.dict = dict;
	sprite_anim_names.current_animation_name = NULL;
	
	psd_document_foreach_layer_level(doc, _fetch_animation_frame_names, &sprite_anim_names);
	if (sprite_anim_names.current_animation_name != NULL)
		_push_animation_names(&sprite_anim_names);

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



const struct godot_psdimporter godot_psdimporter = {0x02,
                                                      constructor, destructor,
                                                      file_load, file_close,
                                                      get_layer_count,
                                                      extract_psd,
                                                      is_sprite_frames, get_sprite_frame_names,
                                                      };
