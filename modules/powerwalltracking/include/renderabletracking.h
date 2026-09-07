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

 /**** HOWTO: ****************************************************************************
 *
 * 1) Start 'Motive' software on NatNet server 'mini'
 * 2) Start VRPN server on VRPN server 'mini' (C:\vrpn\start64.bat)
 * 3) Start OpenSpace and enable PowerwallTrackingModule:
 *        Windows / Settings / Modules / Check PowerwallTracking
 *
 * Info: Powerwall Resolution: 10.800 x 4.096 pixel | Aspect Ratio: 2,64
 * 
 ****************************************************************************************/


#ifndef __OPENSPACE_MODULE_POWERWALLTRACKING___TRACKING___H__
#define __OPENSPACE_MODULE_POWERWALLTRACKING___TRACKING___H__


#include <openspace/rendering/renderable.h>
#include <openspace/properties/scalar/boolproperty.h>


/// NOTE: Foward declaration because header include of external library fails when added here
namespace tracking {
    class Tracker;
    class TrackingUtilizer;
} // namespace tracking


namespace openspace {

    class RenderableTracking : public Renderable {
    public:
        explicit RenderableTracking(const ghoul::Dictionary& dictionary);
        ~RenderableTracking() override;

        void initialize() override;
        void deinitialize() override;

        void render(const RenderData& data, RendererTasks& tasks) override;

        void update(const UpdateData& data) override;

    protected:

    private:
        void connect(void);
        void disconnect(void);


        BoolProperty _enabled;
        bool _connected;

        std::shared_ptr<tracking::Tracker> _tracker = nullptr;
        std::vector<tracking::TrackingUtilizer> _utilizers;
    };

} // namespace openspace

#endif // __OPENSPACE_MODULE_POWERWALLTRACKING___TRACKING___H__
