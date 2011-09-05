/*
    This file is part of Konsole, an X terminal.
    Copyright 2005 by Maksim Orlovich <maksim@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

// Standard
#include <stdlib.h>
#include <iostream>
#include <iomanip>

// Qt
#include <QtCore/QFile>
#include <QtCore/QTextStream>

using namespace std;

static quint32 charVal(QChar val)
{
    if (val == ' ')
        return 0;
    else
        return 1;
}

static quint32 readGlyphLine(QTextStream& input)
{
    QString line = input.readLine();
    while (line.length() < 5)
        line += ' ';

    quint32 val =  charVal(line[0]) |
            (charVal(line[1]) << 1)  |
            (charVal(line[2]) << 2)  |
            (charVal(line[3]) << 3)  |
            (charVal(line[4]) << 4);
    return val;
}

static quint32 readGlyph(QTextStream& input)
{
    return readGlyphLine(input) |
            (readGlyphLine(input) << 5) |
            (readGlyphLine(input) << 10) |
            (readGlyphLine(input) << 15) |
            (readGlyphLine(input) << 20);
}

int main(int argc, char **argv)
{
    if (argc < 1)
    {
        qWarning("usage: fontembedder font.src > font.h");
        exit(1);
    }
    QFile inFile(argv[1]);
    if (!inFile.open(QIODevice::ReadOnly))
    {
        qFatal("Can not open %s", argv[1]);
    }

    QTextStream input(&inFile);

    quint32 glyphStates[128];
    for (int i = 0; i < 128; ++i)
        glyphStates[i] = 0; //nothing..

    while (!input.atEnd())
    {
        QString line = input.readLine();
        line = line.trimmed();
        if (line.isEmpty())
            continue; //Skip empty lines
        if (line[0] == '#')
            continue; //Skip comments

        //Must be a glyph ID.
        int glyph = line.toInt(0, 16);
        if ((glyph < 0x2500) || (glyph > 0x257f))
            qFatal("Invalid glyph number");

        glyph = glyph - 0x2500;

        glyphStates[glyph] = readGlyph(input);
    }

    //Output.
    cout<<"// WARNING: Autogenerated by \"fontembedder " << argv[1] << "\".\n";
    cout<<"// You probably do not want to hand-edit this!\n\n";
    cout<<"static const quint32 LineChars[] = {\n";

    //Nicely formatted: 8 per line, 16 lines
    for (int line = 0; line < 128; line += 8)
    {
        cout<<"\t";
        for (int col = line; col < line + 8; ++col)
        {
            cout<<"0x"<<hex<<setw(8)<<setfill('0')<<glyphStates[col];
            if (col != 127)
                cout<<", ";
        }
        cout<<"\n";
    }
    cout<<"};\n";
    return 0;
}

//kate: indent-width 4; tab-width 4; space-indent on;
