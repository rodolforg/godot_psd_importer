#ifndef PSD_DOCUMENT
#define PSD_DOCUMENT

#include "psd_layer.h"

struct psd_document;

typedef struct psd_document psd_document;

psd_document * psd_document_new();
void psd_document_free(psd_document * doc);

int psd_document_load(psd_document * doc, const char * filename);
void psd_document_unload(psd_document * doc);

int psd_document_parse(psd_document * doc);

int psd_document_children_count(const psd_document * doc);
int psd_document_total_layer_count(const psd_document * doc);

int psd_document_foreach_layer(const psd_document * doc, int (*foreach_cb)(const psd_layer_record * layer, void * cb_data), void * cb_data);

int psd_document_foreach_layer_level(const psd_document * doc, int (*foreach_cb)(const psd_layer_record * layer, void * cb_data, int level), void * cb_data);

#endif // PSD_DOCUMENT
