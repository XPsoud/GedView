#ifndef TOOLBARICONS_H_INCLUDED
#define TOOLBARICONS_H_INCLUDED

#include <wx/wx.h>
#include <wx/bitmap.h>

#define about_png_Length 1537
extern unsigned char about_png[];

wxBitmap wxGet_about_png_Bitmap();

#define open_png_Length 1521
extern unsigned char open_png[];

wxBitmap wxGet_open_png_Bitmap();

#define pdf_png_Length 1903
extern unsigned char pdf_png[];

wxBitmap wxGet_pdf_png_Bitmap();

#define preferences_png_Length 1964
extern unsigned char preferences_png[];

wxBitmap wxGet_preferences_png_Bitmap();

#define save_png_Length 1915
extern unsigned char save_png[];

wxBitmap wxGet_save_png_Bitmap();

#endif //TOOLBARICONS_H_INCLUDED
