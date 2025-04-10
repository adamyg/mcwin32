# Extension

Word, Excel and PowerPoint, mapping:

```

# Microsoft Word Document
regex/i/\.(do[ct]|wri|docx)$
	Open=cmd /c start winword %p
	View=cmd /c start winword %p
type/^Microsoft\ Word
	Open=cmd /c start winword %p
	View=cmd /c start winword %p

# Microsoft Excel Worksheet
regex/i/\.(xl[sw]|xlsx)$
	Open=cmd /c start excel %p
	View=cmd /c start excel %p
type/^Microsoft\ Excel
	Open=cmd /c start excel %p
        View=cmd /c start excel %p

# Microsoft PowerPoint Presentation
regex/i/\.(pp[ts]|pptx)$
	Open=cmd /c start powerpnt %p

```
