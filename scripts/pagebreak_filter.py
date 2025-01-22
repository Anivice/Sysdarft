#!/usr/bin/env python
import sys
from pandocfilters import toJSONFilter, RawBlock, Header


def add_pagebreak(key, value, format, meta):
    if key == 'Header':
        level, _, _ = value
        # Check if header level is 1
        if level == 1:
            # Insert raw LaTeX newpage before the header
            return [RawBlock('latex', '\\newpage'), Header(*value)]


if __name__ == "__main__":
    toJSONFilter(add_pagebreak)
