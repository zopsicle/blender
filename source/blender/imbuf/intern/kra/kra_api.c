#include <stdlib.h>
#include <string.h>
#include <zip.h>

#include "IMB_imbuf.h"
#include "IMB_filetype.h"
#include "IMB_imbuf_types.h"

/*
 * A Krita .kra file is simply a ZIP file that contains layers and metadata.
 * Among the ZIP entries are two interesting files: `mimetype` and `mergedimage.png`.
 * The `mimetype` file contains the string `application/x-krita`.
 * The `mergedimage.png` file contains all the layers merged into a single PNG.
 * This `mergedimage.png` file can simply be offloaded to #IMB_loadpng.
 * See https://docs.krita.org/en/general_concepts/file_formats/file_kra.html. */

#define MIMETYPE_LEN 19
static char const expected_mimetype[MIMETYPE_LEN + 1] = "application/x-krita";
static char const *mergedimagepng_name = "mergedimage.png";

/**
 * Check that the zip file contains an entry called `mimetype`
 * and that this entry contains the string `application/x-krita`.
 */
static bool has_krita_mimetype(zip_t *zip)
{
  zip_file_t * const file = zip_fopen(zip, "mimetype", 0);
  if (file == NULL)
    return false;

  char data[MIMETYPE_LEN + 1] = {0};
  zip_fread(file, data, MIMETYPE_LEN);

  bool is_krita = strcmp(data, expected_mimetype) != 0;

  zip_fclose(file);
  return is_krita;
}

/**
 * Check that the buffer is a ZIP file and check mimetype.
 */
bool imb_is_a_krita(const unsigned char *mem, const size_t size)
{
  zip_error_t error;
  zip_source_t *source;
  zip_t *zip;
  bool is_a = false;

  source = zip_source_buffer_create(mem, size, 0, &error);
  if (source == NULL) {
    goto finalize;
  }

  zip = zip_open_from_source(source, ZIP_RDONLY, &error);
  if (zip == NULL) {
    goto finalize;
  }

  is_a = has_krita_mimetype(zip);

finalize:
  if (zip != NULL) {
    zip_close(zip);
  }
  if (source != NULL) {
    zip_source_close(source);
  }
  return is_a;
}

/**
 * Interpret the given buffer as a ZIP file,
 * and read the zipped `mergedimage.png` file as an imbuf.
 */
struct ImBuf *imb_load_krita(const unsigned char *mem, size_t size, int flags, char *colorspace)
{
  zip_error_t error;
  int status;

  zip_source_t *source = NULL;
  zip_t *zip = NULL;
  zip_file_t *file = NULL;
  void *buf = NULL;
  struct ImBuf *imbuf = NULL;

  source = zip_source_buffer_create(mem, size, 0, &error);
  if (source == NULL) {
    goto finalize;
  }

  zip = zip_open_from_source(source, ZIP_RDONLY, &error);
  if (zip == NULL) {
    goto finalize;
  }

  zip_stat_t stat;
  zip_stat_init(&stat);
  status = zip_stat(zip, mergedimagepng_name, 0, &stat);
  if (status == -1) {
    goto finalize;
  }

  file = zip_fopen(zip, mergedimagepng_name, 0);
  if (file == NULL) {
    goto finalize;
  }

  buf = malloc(stat.size);
  if (buf == NULL) {
    goto finalize;
  }

  status = zip_fread(file, buf, stat.size);
  if (status != stat.size) {
    goto finalize;
  }

  imbuf = imb_loadpng(buf, stat.size, flags, colorspace);
  if (imbuf != NULL) {
    imbuf->ftype = IMB_FTYPE_KRA;
  }

finalize:
  free(buf);
  if (file != NULL) {
    zip_fclose(file);
  }
  if (zip != NULL) {
    zip_close(zip);
  }
  if (source != NULL) {
    zip_source_close(source);
  }
  return imbuf;
}
