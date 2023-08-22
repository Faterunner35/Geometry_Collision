

#pragma once

#include "GraphicsObject.h"

namespace Vector_GM
{
    class DrawingState : public GraphicsObject
    {
    protected:
        // Abstract base class for grouping state classes.  This supports
        // simplification and reduction of member functions in the graphics
        // engine code.
        DrawingState();
    };
}
