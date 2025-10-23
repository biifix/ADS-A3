#!/usr/bin/env python3
"""
Convert ANALYSIS.txt to PDF
Uses fpdf2 library - if not available, falls back to HTML generation
"""

import sys
import os

try:
    from fpdf import FPDF

    class PDF(FPDF):
        def header(self):
            self.set_font('Courier', 'B', 12)
            self.cell(0, 10, 'IMPASSABLE GATE - RESULT ANALYSIS', 0, 1, 'C')
            self.ln(5)

        def footer(self):
            self.set_y(-15)
            self.set_font('Courier', 'I', 8)
            self.cell(0, 10, f'Page {self.page_no()}', 0, 0, 'C')

    # Read the text file
    with open('ANALYSIS.txt', 'r') as f:
        content = f.read()

    # Create PDF
    pdf = PDF()
    pdf.add_page()
    pdf.set_font('Courier', '', 9)
    pdf.set_auto_page_break(auto=True, margin=15)

    # Replace Unicode characters with ASCII equivalents
    content = content.replace('≈', '~=')
    content = content.replace('×', 'x')
    content = content.replace('→', '->')
    content = content.replace('✓', '-')

    # Add content line by line
    for line in content.split('\n'):
        # Handle long lines
        if len(line) > 95:
            # Wrap long lines
            words = line.split()
            current_line = ""
            for word in words:
                if len(current_line) + len(word) + 1 <= 95:
                    current_line += word + " "
                else:
                    pdf.cell(0, 4, current_line.rstrip(), 0, 1)
                    current_line = word + " "
            if current_line:
                pdf.cell(0, 4, current_line.rstrip(), 0, 1)
        else:
            pdf.cell(0, 4, line, 0, 1)

    # Save PDF
    pdf.output('ANALYSIS.pdf')
    print("PDF created successfully: ANALYSIS.pdf")

except ImportError:
    print("fpdf2 not available. Installing...")
    os.system("pip3 install fpdf2 --quiet")
    print("Please run this script again.")
    sys.exit(1)
