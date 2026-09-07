/*****************************************************************************************
 *                                                                                       *
 * Visualization Research Center of the University of Stuttgart (VISUS)                  *
 *                                                                                       *
 * Copyright (c) 2026                                                                    *
 *                                                                                       *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this  *
 * software and associated documentation files (the "Software"), to deal in the Software *
 * without restriction, including without limitation the rights to use, copy, modify,    *
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to    *
 * permit persons to whom the Software is furnished to do so, subject to the following   *
 * conditions:                                                                           *
 *                                                                                       *
 * The above copyright notice and this permission notice shall be included in all copies *
 * or substantial portions of the Software.                                              *
 *                                                                                       *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,   *
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A         *
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT    *
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF  *
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE  *
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                         *
 ****************************************************************************************/


#include <modules/powerwalltracking/powerwalltrackingmodule.h>

#include <openspace/rendering/renderable.h>
#include <openspace/util/factorymanager.h>
#include <openspace/engine/globalscallbacks.h>

#include <ghoul/misc/assert.h>
#include <ghoul/misc/templatefactory.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/dictionary.h>


namespace {
    using namespace openspace;

    constexpr std::string_view _loggerCat = "PowerwallTrackingModule";

} // namespace


namespace openspace {

    PowerwallTrackingModule::PowerwallTrackingModule()
        : OpenSpaceModule(Name)
        , _renderable_tracking(nullptr)
    {

    }

    PowerwallTrackingModule::~PowerwallTrackingModule() {
        // intentionally left empty
    }

    void PowerwallTrackingModule::internalInitialize(const ghoul::Dictionary& dictionary) {
        std::string class_name = "RenderableTracking";
        auto fRenderable = FactoryManager::ref().factory<Renderable>();
        ghoul_assert(fRenderable, "Renderable factory was not created");
        fRenderable->registerClass<RenderableTracking>(class_name);
        /*if (fRenderable->hasClass(class_name)) {
            _renderable_tracking = static_cast<RenderableTracking*>(fRenderable->create(class_name));

        }
        else {
            LERROR(std::format("Unable create instance. [{}, {}, line {}]", __FILE__, __FUNCTION__, __LINE__));
        }*/
    }

    void PowerwallTrackingModule::internalDeinitialize() {

    }

} // namespace openspace
