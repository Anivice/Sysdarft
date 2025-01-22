#!/usr/bin/env python3

import threading
import time
import os
import pypandoc
import sys
from pandocfilters import toJSONFilter, RawBlock, Header


def my_thread_function():
    """Thread function to print progress dots."""
    while True:
        print(".", end="", flush=True)
        time.sleep(0.1)


def add_pagebreak(key, value, format, meta):
    if key == 'Header':
        level, _, _ = value
        # Check if header level is 1
        if level == 1:
            # Insert raw LaTeX newpage before the header
            return [RawBlock('latex', '\\newpage'), Header(*value)]


def main():
    """Main function to manage file conversion."""

    # Check if the right number of arguments were provided
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <INPUT FILE> <OUTPUT FILE>")
        sys.exit(0)

    # Get arguments
    input_file = sys.argv[1]
    output_file = sys.argv[2]

    print(f"Converting {input_file} to {output_file}")

    script_path = os.path.abspath(__file__)

    # Extract the directory from the script path
    script_dir = os.path.dirname(script_path)
    docfilter = "--filter=" + script_dir + '/pagebreak_filter.py'
    reference = "--bibliography=" + script_dir + "/../doc/references.bib"
    csl = "--csl=" + script_dir + "/chicago-fullnote-bibliography.csl"
    my_thread = threading.Thread(target=my_thread_function, daemon=True)
    my_thread.start()

    pypandoc.convert_file(
        input_file,
        'pdf',
        outputfile=output_file,
        extra_args=[docfilter,
                    "-V", "geometry:margin=0.5in",
                    "-f", "markdown+smart",
                    reference,
                    csl,
                    "--citeproc"]
    )


if __name__ == "__main__":
    main()
    print("done")
