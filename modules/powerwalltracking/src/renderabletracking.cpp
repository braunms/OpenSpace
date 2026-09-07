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


#include "modules/powerwalltracking/include/renderabletracking.h"

#include <openspace/documentation/documentation.h>
#include <openspace/engine/globals.h>

#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/dictionary.h>

#include <TrackingUtilizer.h>


namespace {
    using namespace openspace;

    constexpr std::string_view _loggerCat = "TrackingRenderer";

    constexpr Property::PropertyInfo EnabledInfo = {
        "Enabled",
        "Enabled",
        "Enable or disable the powerwall tracking module connecting to the VRPN and NatNet server.",
        Property::Visibility::User
    };

} // namespace


namespace openspace {

    RenderableTracking::RenderableTracking(const ghoul::Dictionary& dictionary)
        : Renderable(dictionary)
        , _enabled(EnabledInfo, false)
        , _connected(false)
        , _tracker(nullptr)
        , _utilizers()
    {
        _enabled.onChange([this]() {
            if (_enabled) {
                connect();
            }
            else {
                disconnect();
            }
            });
        addProperty(_enabled);

    }

    RenderableTracking::~RenderableTracking() {

        // intentionally left empty
    }

    void RenderableTracking::initialize() {


    }

    void RenderableTracking::deinitialize() {


    }

    void RenderableTracking::connect(void) {
        LINFO(std::format("Connecting to VRPN and NatNet server... "));

        /// Initialize Tracker Parameters
        tracking::Tracker::Params tracker_params;
        std::string active_node = ""; // Allowing all clients to access tracking data
        tracker_params.active_node = active_node;
        std::string client_ip = "129.69.205.76"; // = MINYOU
        tracker_params.natnet_params.client_ip = client_ip;
        std::string server_ip = "129.69.205.86"; // = MINI
        tracker_params.natnet_params.server_ip = server_ip;
        tracker_params.natnet_params.cmd_port = 1510;
        tracker_params.natnet_params.data_port = 1511;
        tracker_params.natnet_params.con_type = tracking::NatNetDevicePool::ConnectionType::UniCast;
        tracker_params.natnet_params.verbose_client = true;
        tracking::VrpnDevice<vrpn_Button_Remote>::Params bp;
        const std::string device_name = "ControlBox"; // Leave empty to disable use of VRPN device
        bp.device_name = device_name;
        bp.server_ip = server_ip;
        bp.port = 3884;
        bp.protocol = tracking::VrpnDevice<vrpn_Button_Remote>::Protocols::VRPN_TCP;
        std::vector<tracking::VrpnDevice<vrpn_Button_Remote>::Params> bps;
        bps.emplace_back(bp);
        tracker_params.vrpn_params = bps.data();
        tracker_params.vrpn_params_count = bps.size();

        /// Initialize TrackingUtilizer Parameters
        tracking::TrackingUtilizer::Params utilizer_params;
        utilizer_params.rigid_body_name = ""; // Will be set in line 116 depending on what rigid bodies are available.
        utilizer_params.btn_device_name = device_name; // Insert bp.device_name only for rigid body you want to use.
        utilizer_params.select_btn = 0;
        utilizer_params.rotate_btn = 1;
        utilizer_params.translate_btn = 2;
        utilizer_params.zoom_btn = 3;
        utilizer_params.invert_rotate = true;
        utilizer_params.invert_translate = true;
        utilizer_params.invert_zoom = true;
        utilizer_params.rotate_speed = 1.0f;
        utilizer_params.translate_speed = 1.0f;
        utilizer_params.zoom_speed = 1.0f;
        utilizer_params.single_interaction = false;
        utilizer_params.fov_mode = tracking::TrackingUtilizer::FovMode::WIDTH_AND_ASPECT_RATIO;
        utilizer_params.fov_height = 0.2f;
        utilizer_params.fov_width = 0.2f;
        utilizer_params.fov_horiz_angle = 60.0f;
        utilizer_params.fov_vert_angle = 30.0f;
        utilizer_params.fov_aspect_ratio = tracking::TrackingUtilizer::FovAspectRatio::AR_1_77__1; // 16:9
        utilizer_params.physical_height = 2.4f;
        utilizer_params.physical_width = 6.0f;
        utilizer_params.physical_origin = glm::vec3(-3.0f, 0.3f, 0.0f);
        utilizer_params.physical_x_dir = glm::vec3(1.0f, 0.0f, 0.0f);
        utilizer_params.physical_y_dir = glm::vec3(0.0f, 1.0f, 0.0f);

        /// ----- Tracker -----
        // Handles all communication with NatNet and VRPN.
        // Creating one (!) Tracker which runs separate thread for receiving tracking data.
        _tracker = std::make_shared<tracking::Tracker>();
        if (!_tracker->Initialise(tracker_params)) {
            LERROR(std::format("Unable to initialise 'Tracker'. [{}, {}, line {}]", __FILE__, __FUNCTION__, __LINE__));
            return;
        }

        /// ----- Tracking Utilizers -----
        // Manage tracking data for one rigid body each.
        // Creating a TrackingUtilizer for each found rigid body (and corresponding button device).
        // Initialising TrackingUtilizer with exisiting Tracker.
        _utilizers.clear();
        if (!_tracker->Connect()) {
            LERROR(std::format("Failed to establish 'Tracker' connection'. [{}, {}, line {}]", __FILE__, __FUNCTION__, __LINE__));
            return;
        }
        size_t rigiBodyCount = _tracker->GetRigidBodyCount();
        std::vector<std::string> rigidBodies;
        for (size_t i = 0; i < rigiBodyCount; i++) {
            rigidBodies.emplace_back(std::string(_tracker->GetRigidBodyName(i)));
        }
        for (auto& rb : rigidBodies) {
            utilizer_params.rigid_body_name = rb;
            _utilizers.emplace_back(tracking::TrackingUtilizer());
            if (!_utilizers.back().Initialise(utilizer_params, _tracker)) {
                LERROR(std::format("Failed to initialise 'TrackingUtilizer' [{}, {}, line {}]", __FILE__, __FUNCTION__, __LINE__));
                return;

            }
        }

        _connected = true;
    }

    void RenderableTracking::disconnect(void) {
        LINFO(std::format("Disconnecting from VRPN and NatNet server."));

        _tracker->Disconnect(); /// TODO: Catch false?
        _tracker = nullptr;

        _utilizers.clear();

        _connected = false;
    }

    void RenderableTracking::update(const UpdateData& data) {
        if (!_connected) {
            LERROR(std::format("Not connected. [{}, {}, line {}]", __FILE__, __FUNCTION__, __LINE__));
            return;
        }

        unsigned int btn;
        float pos_x, pos_y, pos_z;
        float orient_x, orient_y, orient_z, orient_w;
        float inters_x, inters_y;
        bool btn_state, rb_state;

        // Get current tracking data for each rigid body managed by the TrackingUtilizers.
        for (auto& tu : _utilizers) {

            // Intersection
            rb_state = tu.GetIntersection(inters_x, inters_y);
            if (rb_state) {

                // Button State
                btn_state = tu.GetRawData(btn, pos_x, pos_y, pos_z, orient_x, orient_y, orient_z, orient_w);

                std::cout << "[INFO][test] UTILIZER ------------------------------------------------------------ " << std::endl;

                std::cout << std::fixed << std::setprecision(4) <<
                    "[INFO] [test] BUTTON-DEVICE \"" << tu.GetButtonDeviceName() << "\" - BUTTON PRESSED (valid = "
                    << ((btn_state) ? ("TRUE") : ("FALSE")) << ") ";
                if (btn_state) {
                    std::cout << " | Button: " << btn;
                }
                std::cout << std::endl;

                std::cout << std::fixed << std::setprecision(4) <<
                    "[INFO] [test] RIGID-BODY \"" << tu.GetRigidBodyName() << "\" - INTERSECTION (valid = "
                    << ((rb_state) ? ("TRUE") : ("FALSE")) << ") ";
                if (rb_state) {
                    std::cout << " | Intersection Coordinates: (" << inters_x << "," << inters_y << ") ";
                }
                std::cout << std::endl;

                std::cout << " ----------------------------------------------------------------------------- " << std::endl;
            }
        }
    }

    void RenderableTracking::render(const RenderData& data, RendererTasks&) {
        ZoneScoped;



    }

} // namespace openspace
