#ifndef PSD_PARSER_H
#define PSD_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

struct psd_parser;
struct psd_document;
struct psd_layer;
struct psd_layer_group;
struct psd_record {
	int is_group;
	union {
		struct psd_layer * layer;
		struct psd_layer_group * group;
	} data;
};

struct psd_parser * psd_parser_new(const char * filename);
void psd_parser_free(struct psd_parser * parser);
struct psd_document * psd_parser_parse(struct psd_parser * parser);

int psd_document_width(const struct psd_document * doc);
int psd_document_height(const struct psd_document * doc);
void psd_document_save_layers(const struct psd_document * doc, const char * dir);
int psd_document_children_count(const struct psd_document * doc);
void psd_document_free(struct psd_document * doc);

struct psd_record * psd_document_first_child(struct psd_document * doc);
struct psd_record * psd_document_next_child(struct psd_document * doc);
int psd_document_has_next_child(const struct psd_document * doc);

void psd_record_free(struct psd_record * record);
const char * psd_record_name(const struct psd_record * record);

struct psd_record * psd_layer_group_first_child(struct psd_layer_group * group);
struct psd_record * psd_layer_group_next_child(struct psd_layer_group * group);
int psd_layer_group_has_next_child(const struct psd_layer_group * group);
int psd_layer_group_children_count(const struct psd_layer_group * group);

int psd_layer_x(const struct psd_layer * layer);
int psd_layer_y(const struct psd_layer * layer);
int psd_layer_width(const struct psd_layer * layer);
int psd_layer_height(const struct psd_layer * layer);

#ifdef __cplusplus
}
#endif

#endif // PSD_PARSER_H
