// Copyright (c) 2010 NANRI <southly@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <stdio.h>
#include <stdlib.h>

#include <string>

#include <expat.h>

#include "cmdline.h"

enum style_t
{
  style_lxml,
  style_sxml,
  style_xmlpm,
  style_count,
};

static std::string buffer;
static int style;
static FILE *out;

static void
write_symbol(FILE *os, const char *s)
{
  fputc('|', os);
  for (int i = 0; s[i]; ++i) {
    switch (s[i]) {
    case '|':
    case '\\':
      fputc('\\', os);
    default:
      fputc(s[i], os);
      break;
    }
  }
  fputc('|', os);
}

static void
write_string(FILE *os, const char *s)
{
  fputc('"', os);
  for (int i = 0; s[i]; ++i) {
    switch (s[i]) {
    case '\n':
      fputs("\\n", os);
      break;

    case '"':
    case '\\':
      fputc('\\', os);
    default:
      fputc(s[i], os);
      break;
    }
  }
  fputc('"', os);
}

static void
flush(FILE *os, std::string& s)
{
  if (s.find_first_not_of(" \n\r\t\f") != std::string::npos) {
    write_string(os, s.c_str());
  }
  s.clear();
}

static void
start_element_lxml(void */* userData */, const char *name, const char **atts)
{
  if (!buffer.empty()) {
    flush(out, buffer);
    fputc(' ', out);
  }

  if (atts[0]) {
    fputs("((", out);
    write_symbol(out, name);
    for (int i = 0; atts[i]; i += 2) {
      fputc(' ', out);
      write_symbol(out, atts[i]);
      fputc(' ', out);
      write_string(out, atts[i+1]);
    }
    fputs(") ", out);
  } else {
    fputc('(', out);
    write_symbol(out, name);
    fputc(' ', out);
  }
}

static void
start_element_sxml(void */* userData */, const char *name, const char **atts)
{
  if (!buffer.empty()) {
    flush(out, buffer);
    fputc(' ', out);
  }

  fputc('(', out);
  write_symbol(out, name);
  fputc(' ', out);
  if (atts[0]) {
    fputs("(@", out);
    for (int i = 0; atts[i]; i += 2) {
      fputs(" (", out);
      write_symbol(out, atts[i]);
      fputc(' ', out);
      write_string(out, atts[i+1]);
      fputc(')', out);
    }
    fputc(')', out);
  }
}

static void
start_element_xmlpm(void */* userData */, const char *name, const char **atts)
{
  if (!buffer.empty()) {
    flush(out, buffer);
    fputc(' ', out);
  }

  fputc('(', out);
  write_string(out, name);
  fputs(" (", out);
  for (int i = 0; atts[i]; i += 2) {
    fputs(" (", out);
    write_string(out, atts[i]);
    fputs(" . ", out);
    write_string(out, atts[i+1]);
    fputc(')', out);
  }
  fputc(')', out);
}

static void
end_element(void */* userData */, const char */* name */)
{
  if (!buffer.empty()) flush(out, buffer);
  fputs(")\n", out);
}

static void
charactor_data(void */* userData */, const char *s, int len)
{
  std::string str(s, len);
  buffer += str;
}

static int
parse(FILE *in)
{
  char buf[2048];
  XML_StartElementHandler f[] = {
    start_element_lxml,
    start_element_sxml,
    start_element_xmlpm,
    0,
  };
  XML_Parser parser = XML_ParserCreate("UTF-8");
  XML_SetElementHandler(parser, f[style], end_element);
  XML_SetCharacterDataHandler(parser, charactor_data);
  int done;
  do {
    size_t len = fread(buf, 1, sizeof buf, in);
    done = len < sizeof buf;
    if (XML_Parse(parser, buf, len, done) == XML_STATUS_ERROR) {
      fprintf(stderr, "%s: %lu\n", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
      return 1;
    }
  } while (!done);
  XML_ParserFree(parser);
  return 0;
}

static int
get_style(bool lxml, bool sxml, bool xmlpm)
{
  int i = 0;
  if (lxml) i += 1;
  if (sxml) i += 2;
  if (xmlpm) i += 4;
  int r[] = { style_lxml, style_lxml, style_sxml, -1, style_xmlpm, -1, -1, -1 };
  return r[i];
}

int
main(int argc, char *argv[])
{
  buffer.clear();

  cmdline::parser p;
  p.add<std::string>("input", 'i', "input filename", false);
  p.add<std::string>("output", 'o', "output filename", false);
  p.add("lxml", 'l', "lxml style output [default]");
  p.add("sxml", 's', "sxml style output");
  p.add("xmlpm", 'x', "xmlpm style output");
  p.add("help", 0, "print help");
  p.set_program_name("mapel");

  bool ok = p.parse(argc, argv);
  if (argc == 1 || p.exist("help")) {
    fprintf(stdout, "%s\n", p.usage().c_str());
    return 0;
  }

  if (!ok) {
    fprintf(stderr, "%s\n%s\n", p.error().c_str(), p.usage().c_str());
    return 1;
  }
  style = get_style(p.exist("lxml"), p.exist("sxml"), p.exist("xmlpm"));
  if (style < 0) {
    fprintf(stderr, "Error : duplicate styles.\n");
    return 2;
  }

  FILE *in = stdin;
  if (p.exist("input")) {
    in = fopen(p.get<std::string>("input").c_str(), "r");
  }

  out = stdout;
  if (p.exist("output")) {
    out = fopen(p.get<std::string>("output").c_str(), "w");
  }

  int r = parse(in);

  if (in != stdin) {
    fclose(in);
  }
  if (out != stdout) {
    fclose(out);
  }

  return r;
}
