#include <assert.h>
#include <libxml/parser.h>

#include "test-runner.h"

void
validate_xml_with_dtd(const char *xml_path, const char *dtd_path)
{
  int success;
  assert(dtd_path && xml_path);

  xmlParserCtxtPtr ctx = NULL;
  xmlDocPtr doc = NULL;
  xmlDtdPtr dtd = NULL;
  xmlValidCtxtPtr dtdctx;
  xmlParserInputBufferPtr buffer;

  dtdctx = xmlNewValidCtxt();
  ctx = xmlNewParserCtxt();
  assert(dtdctx && ctx);

  buffer = xmlParserInputBufferCreateFilename(dtd_path, XML_CHAR_ENCODING_UTF8);
  assert(buffer);

  dtd = xmlIOParseDTD(NULL, buffer, XML_CHAR_ENCODING_UTF8);
  doc = xmlCtxtReadFile(ctx, xml_path, NULL, 0);
  assert(doc && dtd);

  success = xmlValidateDtd(dtdctx, doc, dtd);
  xmlFreeDoc(doc);
  xmlFreeParserCtxt(ctx);
  xmlFreeDtd(dtd);
  xmlFreeValidCtxt(dtdctx);
  assert(success);
}

TEST(validate_zen_desktop_xml)
{
  const char *zigen_xml = getenv("ZEN_DESKTOP_XML");
  const char *wayland_dtd_path = getenv("WAYLAND_DTD_PATH");
  validate_xml_with_dtd(zigen_xml, wayland_dtd_path);
}
