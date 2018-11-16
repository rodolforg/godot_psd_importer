#include "psd_parser.h"

#include "parser/PsdParser.h"
#include "Layer.h"
#include "LayerGroup.h"

#include "register_types.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psd_parser {
	PsdParser * parser;
};

struct psd_document {
	Document * doc;
};

struct psd_layer {
	Layer * layer;
};

struct psd_layer_group {
	LayerGroup * group;
};

#ifdef __cplusplus
}
#endif

static struct psd_record * cxx_record_to_c(Record * record)
{
	if (!record)
		return NULL;

	struct psd_record * ret = (struct psd_record *) api->godot_alloc(sizeof(struct psd_record));
	
	ret->is_group = record->is_group()? 1 : 0;
	
	if (ret->is_group) {
		ret->data.group = (struct psd_layer_group *) api->godot_alloc(sizeof(struct psd_layer_group));
		ret->data.group->group = (LayerGroup *) record;
	} else {
		ret->data.layer = (struct psd_layer *) api->godot_alloc(sizeof(struct psd_layer));
		ret->data.layer->layer = (Layer *) record;
	}
	return ret;
}


struct psd_parser * psd_parser_new(const char * filename)
{
	PsdParser * parser = new PsdParser(filename);
	if (parser == NULL) {
		return NULL;
	}
	struct psd_parser * ret = (struct psd_parser *) api->godot_alloc(sizeof(struct psd_parser));
	if (ret == NULL) {
		delete parser;
		return NULL;
	}
	
	ret->parser = parser;
	
	return ret;
}

void psd_parser_free(struct psd_parser * parser)
{
	if (parser == NULL)
		return;
	
	delete parser->parser;
	api->godot_free(parser);
}

struct psd_document * psd_parser_parse(struct psd_parser * parser)
{
	if (parser == NULL || parser->parser == NULL) // !!
		return NULL;

	Document * doc = parser->parser->parse(); // !!!
	if (doc == NULL)
		return NULL;

	struct psd_document * ret = (struct psd_document *) api->godot_alloc(sizeof(struct psd_document));
	if (ret == NULL) {
		delete doc;
		return NULL;
	}

	ret->doc = doc;
	return ret;
}

int psd_document_width(const struct psd_document * doc)
{
	if (doc == NULL || doc->doc == NULL)
		return -1;
	return doc->doc->width();
}

int psd_document_height(const struct psd_document * doc)
{
	if (doc == NULL || doc->doc == NULL)
		return -1;
	return doc->doc->height();
}

void psd_document_save_layers(const struct psd_document * doc, const char * dir)
{
	if (doc == NULL || doc->doc == NULL)
		return;
	return doc->doc->save_layers(dir);
}

int psd_document_children_count(const struct psd_document * doc)
{
	if (doc == NULL || doc->doc == NULL)
		return -1;
	return doc->doc->children_count();
}

void psd_document_free(struct psd_document * doc)
{
	if (doc == NULL)
		return;
	
	delete doc->doc;
	api->godot_free(doc);
}

struct psd_record * psd_document_first_child(struct psd_document * doc)
{
	if (doc == NULL || doc->doc == NULL)
		return NULL;
	return cxx_record_to_c(doc->doc->first_child());
}

struct psd_record * psd_document_next_child(struct psd_document * doc)
{
	if (doc == NULL || doc->doc == NULL)
		return NULL;
	return cxx_record_to_c(doc->doc->next_child());
}

int psd_document_has_next_child(const struct psd_document * doc)
{
	if (doc == NULL || doc->doc == NULL)
		return -1;
	return doc->doc->has_next_child()? 1 : 0;
}


void psd_record_free(struct psd_record * record)
{
	if (record == NULL)
		return;
	if (record->is_group)
		api->godot_free(record->data.group);
	else
		api->godot_free(record->data.layer);
	api->godot_free(record);
}

const char * psd_record_name(const struct psd_record * record)
{
	if (record == NULL)
		return NULL;
	if (record->is_group)
		return record->data.group->group->name();
	else
		return record->data.layer->layer->name();
}

struct psd_record * psd_layer_group_first_child(struct psd_layer_group * group)
{
	if (group == NULL || group->group == NULL)
		return NULL;
	return cxx_record_to_c(group->group->first_child());
}

struct psd_record * psd_layer_group_next_child(struct psd_layer_group * group)
{
	if (group == NULL || group->group == NULL)
		return NULL;
	return cxx_record_to_c(group->group->next_child());
}

int psd_layer_group_has_next_child(const struct psd_layer_group * group)
{
	if (group == NULL || group->group == NULL)
		return -1;
	return group->group->has_next_child()? 1 : 0;
}

int psd_layer_group_children_count(const struct psd_layer_group * group)
{
	if (group == NULL || group->group == NULL)
		return -1;
	return group->group->children_count();
}

int psd_layer_x(const struct psd_layer * layer)
{
	if (layer == NULL || layer->layer == NULL)
		return -1;
	return layer->layer->x();
}

int psd_layer_y(const struct psd_layer * layer)
{
	if (layer == NULL || layer->layer == NULL)
		return -1;
	return layer->layer->y();
}

int psd_layer_width(const struct psd_layer * layer)
{
	if (layer == NULL || layer->layer == NULL)
		return -1;
	return layer->layer->width();
}

int psd_layer_height(const struct psd_layer * layer)
{
	if (layer == NULL || layer->layer == NULL)
		return -1;
	return layer->layer->height();
}
