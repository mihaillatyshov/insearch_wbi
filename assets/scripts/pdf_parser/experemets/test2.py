from img2table.document import PDF, Image
from img2table.ocr import TesseractOCR

# Instantiation of the pdf
print("Instantiation of the pdf")
pdf = PDF(src="./pdf/5_Amati_Каталог_07.08.2023.pdf", pdf_text_extraction=True)

# Instantiation of the OCR, Tesseract, which requires prior installation
ocr = TesseractOCR(lang="eng+rus", psm=6)

# Table identification and extraction
# pdf_tables = pdf.extract_tables(borderless_tables=True, min_confidence=6)
# pdf_tables = pdf.extract_tables(ocr=ocr, borderless_tables=True, min_confidence=6)
# print(pdf_tables)

# We can also create an excel file with the tables
# pdf.to_xlsx('./out/xlsx/tables.xlsx', ocr=ocr, borderless_tables=True, min_confidence=6)

print("Start Extract")
pdf.to_xlsx('./out/xlsx/tables.xlsx', ocr=ocr, borderless_tables=True, min_confidence=6)
