/*                D I S P L A Y M A N A G E R . H
 * BRL-CAD
 *
 * Copyright (c) 2020 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file ViewportManager.h */


#ifndef RT3_VIEWPORTMANAGER_H
#define RT3_VIEWPORTMANAGER_H
#ifdef _WIN32
#include <Windows.h>
#endif

#include "Viewport.h"
#include "brlcad/VectorList.h"
class Viewport;

class ViewportManager{
public:
    explicit ViewportManager(Viewport &display);

    // most of the methods below correspond to a method with a similar name from libdm
    void drawVList(BRLCAD::VectorList *vp);
    void setFGColor(float r, float g, float b, float transparency);
    void setBGColor(float r, float g, float b);
    void setLineAttr(int width, int style);
    void setLineStyle(int style);
    void setLineWidth(int width);
    unsigned int genDLists(size_t range);
    void beginDList(unsigned int list);
    void endDList();
    void drawDList(unsigned int list);
    void freeDLists(unsigned int list, int range);
    GLboolean isDListValid(unsigned int list);
    void saveState();
    void restoreState();
    void drawBegin();
    void loadMatrix(const GLfloat *m);
    void loadPMatrix(const GLfloat *m);
    void setSuffix(const BRLCAD::VectorList& vectorList);
    void clearSuffix(void);
    void drawSuffix(void);


    class DrawVListElementCallback {
    public:
        struct DrawVlistVars{
            int first = 1;
            int mFlag = 1;
        };
        const ViewportManager * displayManager;
        DrawVlistVars * vars;
        explicit DrawVListElementCallback(const ViewportManager *displayManager, DrawVlistVars *vars);
        bool operator()(BRLCAD::VectorList::Element* element);
    };

private:
    Viewport &display;

    int dmLight = 1;
    bool dmTransparency = false;

    float wireColor[4] = {.9,.1,.1,1};
    float diffuseColor[4];
    float  ambientColor[4];
    float  specularColor[4];
    float  backDiffuseColorDark[4];
    float backDiffuseColorLight[4];
    float bgColor[3] = {0.0, 0.0, 0.125};

    BRLCAD::VectorList suffix;
};


#endif //RT3_VIEWPORTMANAGER_H
