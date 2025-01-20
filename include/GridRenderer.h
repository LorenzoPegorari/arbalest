
#ifndef RT3_GRIDRENDERER_H
#define RT3_GRIDRENDERER_H


#include "Renderer.h"
#include "ADisplay.h"

class ADisplay;

class GridRenderer: public Renderer {

public:
    void render() override;

    GridRenderer(ADisplay *display);

private:
    ADisplay * display;
};


#endif //RT3_GRIDRENDERER_H
