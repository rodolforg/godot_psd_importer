#ifndef PSD_DOCUMENT_LAYER
#define PSD_DOCUMENT_LAYER

struct _psd_layer_record;
typedef struct _psd_layer_record psd_layer_record;

const char * psd_layer_name(const psd_layer_record * record);

int psd_layer_is_group(const psd_layer_record * layer);

int psd_layer_x(const psd_layer_record * layer);
int psd_layer_y(const psd_layer_record * layer);
int psd_layer_width(const psd_layer_record * layer);
int psd_layer_height(const psd_layer_record * layer);
const char * psd_layer_image_data(const psd_layer_record * layer);

#endif // PSD_DOCUMENT_LAYER

