pdf2svg
=======

A simple PDF to SVG converter using the Poppler and Cairo libraries.

For Windows binaries see <https://github.com/tbolsh/pdf2svg-windows>.

C# usage example:
```
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace Vectorizer
{
    public class Vectorizer
    {
        [DllImport("pdf2svg.dll")]
        public static extern ulong getSVGSize(IntPtr svgPtr);
        [DllImport("pdf2svg.dll")]
        public static extern IntPtr getSVGPtr(IntPtr svgPtr);
        [DllImport("pdf2svg.dll")]
        public static extern IntPtr getPDFDocument(IntPtr pdfBytes, uint size);
        [DllImport("pdf2svg.dll")]
        public static extern void freePDFDocument(IntPtr docPtr);
        [DllImport("pdf2svg.dll")]
        public static extern IntPtr getSinglePage(IntPtr pdfBytes, uint size, int pageNo);
        [DllImport("pdf2svg.dll")]
        public static extern IntPtr getOnePage(IntPtr docPtr, uint pageNo);
        [DllImport("pdf2svg.dll")]
        public static extern void freeSVG(IntPtr docPtr);

        public byte[] GetPage(byte[] data, int pageNumber)
        {
            IntPtr svgPtr = IntPtr.Zero;
            try
            {
                svgPtr = getSinglePage(Marshal.UnsafeAddrOfPinnedArrayElement(data, 0), (uint)data.Length);
                if (svgPtr == IntPtr.Zero)
                {
                    return new byte[0];
                }

                string svg = Marshal.PtrToStringAnsi(getSVGPtr(svgPtr));
                freeSVG(svgPtr);
                svgPtr = IntPtr.Zero;
                return Encoding.UTF8.GetBytes(svg);
            }
            finally
            {
                if (svgPtr != IntPtr.Zero)
                {
                    freeSVG(svgPtr);
                }
            }
        }

        public List<byte[]> GetRangeOfPages(byte[] data, int fromPage, int toPage)
        {
            List<byte[]> images = new List<byte[]>();
            IntPtr docPtr = IntPtr.Zero;
            IntPtr svgPtr = IntPtr.Zero;
            try
            {
                docPtr = getPDFDocument(Marshal.UnsafeAddrOfPinnedArrayElement(data, 0), (uint)data.Length);
                if (docPtr != IntPtr.Zero)
                {
                    for (int i = fromPage; i <= toPage; i++)
                    {
                        svgPtr = getOnePage(docPtr, i);
                        if (svgPtr == IntPtr.Zero)
                        {
                            images.Add(new byte[0]);
                            continue;
                        }

                        string svg = Marshal.PtrToStringAnsi(getSVGPtr(svgPtr));
                        freeSVG(svgPtr);
                        svgPtr = IntPtr.Zero;
                        images.Add( Encoding.UTF8.GetBytes(svg) );
                    }
                }
            }
            finally
            {
                if (svgPtr != IntPtr.Zero)
                {
                    freeSVG(svgPtr);
                }
                if (docPtr != IntPtr.Zero)
                {
                    freePDFDocument(docPtr);
                }
            }
            return images;
        }
    }
}
```
____________________________________________________________________

Copyright (C) 2025 Timofei Bolshakov (tbolsh@gmail.com)
Converted it to windows dll.

Copyright (C) 2007-2013 David Barton (davebarton@cityinthesky.co.uk)
[http://www.cityinthesky.co.uk/](http://www.cityinthesky.co.uk/)

Copyright (C) 2007 Matthew Flaschen (matthew.flaschen@gatech.edu)
Updated to allow conversion of all pages at once.

Copyright (C) 2008 Ed Grace
Added GNU Autotools commands.
