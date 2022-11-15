#!/bin/sh

PROGNAME=$(basename "$0")

USAGE=$(cat <<- EOS
Usage: $PROGNAME [variable name] [input file] [output file]
    Generate a header file that defines the content of the input file as 'static const char[]'

Sample: cat file.txt | $PROGNAME variable_name > file.h 
EOS
)

if ! [ $# -eq 3 ]; then
  echo "$USAGE" 1>&2
  exit 1;
fi

VARIABLE_NAME=$1
INPUT=$2
OUTPUT=$3

# Check VARIABLE_NAME
if ! expr "$VARIABLE_NAME" : '^\([[:alpha:]]\|\_\)\([[:alnum:]]\|\_\)*$' >> /dev/null; then
  echo "$VARIABLE_NAME is an invalid name for a c variable" 1>&2
  exit 1
fi

# Check INPUT
if ! [ -e "$INPUT" ]; then
  echo "$INPUT does not exit" 1>&2
  exit 1
fi

if ! [ -r "$INPUT" ]; then
  echo "$INPUT does not have read permission" 1>&2
  exit 1
fi

generate() {
  printf 'static const char %s[] = {\n' "$VARIABLE_NAME"
  
  # TODO: Handle error
  od -A n -t x1 -v "$INPUT" |
  tr -Cd '0123456789abcdef\n' |
  sed "s/../0x&, /g"
  
  printf '0\n'
  printf '};\n'
}

if ! generate > "$OUTPUT"; then
  echo "Failed to write content into $OUTPUT" 1>&2
  exit 1
fi

exit 0
