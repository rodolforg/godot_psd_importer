#include "psd_document.h"

#include "libpsd.h"

#include "simple_list.h"

#include <stdlib.h>
#include <string.h>

struct item {
	const psd_layer_record * record;
	List * children;
};

struct item * psd_doc_item_new() {
	struct item * item = malloc(sizeof(struct item));
	if (item == NULL)
		return NULL;
	return item;
}

void psd_doc_item_free(void * item) {
	list_free(((struct item*) item)->children);
	free(item);
}

void psd_doc_item_init(struct item * item, const psd_layer_record * record) {
	item->record = record;
	item->children = NULL;
}

struct psd_document {
	int width;
	int height;
	List * children;
	char * filename;
	psd_context * context;
};

static int _psd_document_init(psd_document * doc);
static void _psd_document_finish(psd_document * doc);

psd_document * psd_document_new() {
	psd_document * doc = malloc(sizeof(psd_document));
	if (doc == NULL)
		return NULL;
	int ok = _psd_document_init(doc);
	if (!ok) {
		free(doc);
		return NULL;
	}
	return doc;
}

void psd_document_free(psd_document * doc) {
	_psd_document_finish(doc);
	free(doc);
}

static int _psd_document_init(psd_document * doc) {
	if (doc == NULL)
		return 0;
	doc->width = 0;
	doc->height = 0;
	doc->filename = NULL;
	doc->context = NULL;
	doc->children = list_new(psd_doc_item_free);
	
	return (doc->children != NULL);
}

static void _psd_document_finish(psd_document * doc) {
	if (doc == NULL)
		return;
	list_free(doc->children);
	free(doc->filename);
	psd_image_free(doc->context);
}

int psd_document_load(psd_document * doc, const char * filename) {
	if (doc == NULL || filename == NULL)
		return 0;

	if (doc->filename != NULL) {
		free(doc->filename);
		doc->filename = NULL;
	}
	int len = strlen(filename)+1;
	if (len < 0)
		return 0;
	doc->filename = malloc(len);
	if (doc->filename == NULL)
		return 0;
	memcpy(doc->filename, filename, len);

	psd_status status = psd_image_load(&doc->context, (psd_char *)filename);
	if (status != psd_status_done) {
		free(doc->filename);
		doc->filename = NULL;
		return 0;
	}

	doc->width = doc->context->width;
	doc->height = doc->context->height;

	return 1;
}

void psd_document_unload(psd_document * doc) {
	if (doc == NULL)
		return;

	list_free(doc->children);
	doc->children = NULL;

	psd_image_free(doc->context);
	doc->context = NULL;

	free(doc->filename);
	doc->filename = NULL;
}

/* A bunch of utility methods */

static int _is_group(const psd_layer_record *record)
{
  return record->layer_type == psd_layer_type_folder;
}

static int _is_end_of_group(const psd_layer_record *record)
{
  return record->layer_type == psd_layer_type_hidden
    && strcmp((const char *)record->layer_name, "</Layer group>") == 0;
}

static int _is_layer(const psd_layer_record *record)
{
  return record->layer_type == psd_layer_type_normal;
}

static void _parse_group(const psd_context * context, List * group, int * layer_index) {
	while (*layer_index >= 0) {
		const psd_layer_record * record = &context->layer_records[*layer_index];
		if (_is_layer(record)) {
			struct item * item = psd_doc_item_new();
			psd_doc_item_init(item, record);
			list_append(group, item);
		} else if (_is_group(record)) {
			struct item * item = psd_doc_item_new();
			psd_doc_item_init(item, record);
			item->children = list_new(psd_doc_item_free);
			list_append(group, item);
			--*layer_index;
			_parse_group(context, item->children, layer_index);
		} else if (_is_end_of_group(record)) {
			return;
		} else {
			// alert!
		}
		--*layer_index;
	}
}

int psd_document_parse(psd_document * doc) {
	if (doc == NULL || doc->context == NULL)
		return 0;

	int layer_index = doc->context->layer_count - 1;
	_parse_group(doc->context, doc->children, &layer_index);

	return 1;
}

int psd_document_children_count(const psd_document * doc) {
	if (doc == NULL || doc->context == NULL)
		return -1;

	return list_count(doc->children);
}

int psd_document_total_layer_count(const psd_document * doc) {
	if (doc == NULL || doc->context == NULL)
		return 0;

	return doc->context->layer_count;
}

struct inner_foreach_data {
	int (*foreach_cb)(const psd_layer_record * layer, void * cb_data);
	void * cb_data;
};

static int _foreach_layer(const struct item * item, void * fk_data) {
	struct inner_foreach_data * inner_data = (struct inner_foreach_data*) fk_data;
	
	const psd_layer_record * layer = item->record;
	
	int should_continue = inner_data->foreach_cb(layer, inner_data->cb_data);
	if (!should_continue)
		return 0;
	
	if (!psd_layer_is_group(layer))
		return 1;

	return list_foreach(item->children, (int (*)(void *, void *)) _foreach_layer, inner_data);
}

int psd_document_foreach_layer(const psd_document * doc, int (*foreach_cb)(const psd_layer_record * layer, void * cb_data), void * cb_data) {
	if (doc == NULL || foreach_cb == NULL)
		return 0;
		
	struct inner_foreach_data inner_data;
	inner_data.cb_data = cb_data;
	inner_data.foreach_cb = foreach_cb;
	
	return list_foreach(doc->children, (int (*)(void *, void *))_foreach_layer, &inner_data);
}

struct inner_foreach_level_data {
	int (*foreach_cb)(const psd_layer_record * layer, void * cb_data, int level);
	void * cb_data;
	int level;
};

static int _foreach_layer_level(const struct item * item, void * fk_data) {
	struct inner_foreach_level_data * inner_data = (struct inner_foreach_level_data*) fk_data;
	
	const psd_layer_record * layer = item->record;
	
	int should_continue = inner_data->foreach_cb(layer, inner_data->cb_data, inner_data->level);
	if (!should_continue)
		return 0;
	
	if (!psd_layer_is_group(layer))
		return 1;

	struct inner_foreach_level_data inner_data2 = *inner_data;
	inner_data2.level++;
	return list_foreach(item->children, (int (*)(void *, void *))_foreach_layer_level, &inner_data2);
}

int psd_document_foreach_layer_level(const psd_document * doc, int (*foreach_cb)(const psd_layer_record * layer, void * cb_data, int level), void * cb_data) {
	if (doc == NULL || foreach_cb == NULL)
		return 0;
		
	struct inner_foreach_level_data inner_data;
	inner_data.cb_data = cb_data;
	inner_data.foreach_cb = foreach_cb;
	inner_data.level = 0;
	
	return list_foreach(doc->children, (int (*)(void *, void *))_foreach_layer_level, &inner_data);
}

