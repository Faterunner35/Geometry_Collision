
#pragma once

#include "Controller.h"
#include <list>
#include <memory>

namespace Vector_GM
{
    class ControlledObject
    {
    protected:
        // Abstract base class.
        ControlledObject() = default;
    public:
        virtual ~ControlledObject() = default;

        // Access to the controllers that control this object.
        typedef std::list<std::shared_ptr<Controller>> List;

        inline List const& GetControllers() const
        {
            return mControllers;
        }

        void AttachController(std::shared_ptr<Controller> const& controller);
        void DetachController(std::shared_ptr<Controller> const& controller);
        void DetachAllControllers();
        bool UpdateControllers(double applicationTime);

    private:
        List mControllers;
    };
}
