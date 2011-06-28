
#include <bdlib/src/String.h>
#include <bdlib/src/Stream.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

int skipline (const char *line, int *skip) {
  static int multi = 0;
  if ((!strncmp(line, "//", 2))) {
    (*skip)++;
  } else if ( (strstr(line, "/*")) && (strstr(line, "*/")) ) {
    multi = 0;
    (*skip)++;
  } else if ( (strstr(line, "/*")) ) {
    (*skip)++;
    multi = 1;
  } else if ( (strstr(line, "*/")) ) {
    multi = 0;
  } else {
    if (!multi) (*skip) = 0;
  }
  return (*skip);
}

#define type(hub, leaf) (hub ? 1 : (leaf ? 2 : 0))

int parse_help(bd::String infile, bd::String outfile) {
  bd::Stream in, out;
  bd::String buffer, cmd, buf;
  size_t pos = 0;
  int skip = 0, leaf = 0, hub = 0;

  in.loadFile(infile);
  printf("Parsing help file '%s'", infile.c_str());
  out <<  bd::String::printf("/* DO NOT EDIT THIS FILE, EDIT doc/help.txt INSTEAD */\n#ifndef HELP_H\n\
#define HELP_H\n\
#include \"cmds.h\"\n\
\n\
help_t help[] = \n\
{ \n");
  while (in.tell() < in.length()) {
    buffer = in.getline().chomp();

    if ((skipline(buffer.c_str(), &skip))) continue;

    if (buffer[0] == ':') { /* New cmd */
      bd::String ifdef(buffer.length());
      int cl = 0, doleaf = 0, dohub = 0;

      ++buffer;
      ifdef = newsplit(buffer, ':');

      if (ifdef.length()) {
        if (ifdef == "leaf") {
          if (hub) { cl = 1; hub = 0; }
          if (!leaf) {
            doleaf = leaf = 1;
          }
        } else if (ifdef == "hub") {
          if (leaf) { cl = 1; hub = 0; }
          if (!hub) {
            dohub = hub = 1;
          }
        }
      } else { if (leaf || hub) { cl = 1; } leaf = 0; hub = 0;  }

      if (cmd.length()) {		/* CLOSE LAST CMD */
        if (cmd.find(':') != bd::String::npos)		/* garbled */
          out << "\")},\n";
        else
          out << "\"},\n";
        cmd.clear();
      }

      cmd = newsplit(buffer, ':');
      if (cmd != "end") {		/* NEXT CMD */
        printf(".");
        if ((pos = cmd.find(':')) != bd::String::npos)
          cmd = cmd(0, pos);
        out << bd::String::printf("  {%d, \"%s\", 0, \"", type(dohub, doleaf), cmd.c_str());
      } else {			/* END */
        out << "  {0, NULL, 0, NULL}\n};\n";
      }
    } else {				/* CMD HELP INFO */
      out << bd::String::printf("%s\\n", buffer.sub("\"", "\\\"").c_str());
    }
  }
  out << "#endif /* HELP_H */\n";

  if (out.writeFile(outfile)) {
    fprintf(stderr, "Failure writing %s\n", outfile.c_str());
    return 1;
  }
  printf(" Success\n");
  return 0;
}

int main(int argc, char **argv) {
  if (argc < 3) return 1;
  bd::String in(argv[1]), out(argv[2]);
  return parse_help(in, out);
}
