#!/usr/bin/env python3

import markdown2
import pdfkit
import sys
import threading
import time
import os
import subprocess


def my_thread_function():
    """Thread function to print progress dots."""
    while True:
        print(".", end="", flush=True)
        time.sleep(0.1)


def convert_markdown_to_pdf(md_file_path, output_pdf_path, css_file):
    """Convert a Markdown file to PDF with custom layout using pdfkit."""

    # Read the markdown file
    with open(md_file_path, 'r', encoding='utf-8') as md_file:
        markdown_content = md_file.read()

    # Convert markdown to HTML
    html_content = markdown2.markdown(markdown_content)

    # Temporary HTML file for conversion
    temp_html_file = md_file_path.replace(".md", "_temp.html")

    # Save the HTML content to a temporary file
    with open(temp_html_file, 'w', encoding='utf-8') as f:
        f.write(html_content)

    # Convert HTML to PDF using pdfkit and custom CSS
    pdfkit.from_file(temp_html_file, output_pdf_path, css=css_file)

    # Clean up the temporary HTML file
    os.remove(temp_html_file)


def main():
    """Main function to manage file conversion."""
    my_thread = threading.Thread(target=my_thread_function, daemon=True)
    my_thread.start()

    # Check if the right number of arguments were provided
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <INPUT FILE> <OUTPUT FILE>")
        sys.exit(0)

    # Get arguments
    input_file = sys.argv[1]
    output_file = sys.argv[2]

    # Path to the CSS file
    script_dir = os.path.dirname(os.path.abspath(__file__))
    css_file = script_dir + "/styles.css"  # Ensure the CSS file is in the same directory

    # Convert Markdown to PDF with custom layout
    convert_markdown_to_pdf(input_file, output_file, css_file)


if __name__ == "__main__":
    main()
