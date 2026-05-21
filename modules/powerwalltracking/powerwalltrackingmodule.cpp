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
#include <openspace/documentation/documentation.h>
#include <openspace/engine/globals.h>
#include <openspace/engine/globalscallbacks.h>
#include <openspace/engine/windowdelegate.h>
#include <openspace/interaction/interactionhandler.h>
#include <openspace/interaction/interactionmonitor.h>
#include <openspace/navigation/navigationhandler.h>
#include <openspace/rendering/renderable.h>
#include <openspace/util/factorymanager.h>
#include <ghoul/format.h>
#include <ghoul/logging/logmanager.h>
#include <ghoul/misc/dictionary.h>
#include <ghoul/misc/templatefactory.h>
#include <algorithm>
#include <functional>
#include <optional>
#include <utility>

#include "TrackingUtilizer.h"


 /**** HOWTO: ******************************************************************
 *
 * 1) Start 'Motive' software on NatNet server 'mini'
 * 2) Start VRPN server on VRPN server 'mini' (C:\vrpn\start64.bat)
 * 3) Start OpenSpace with PowerwallTrackingModule enabled
 *
 ******************************************************************************/
 /// Info: Powerwall Resolution: 10.800 x 4.096 pixel | Aspect Ratio: 2,64


namespace openspace {

    PowerwallTrackingModule::PowerwallTrackingModule()
        : OpenSpaceModule("PowerwallTracking")
    {

    }

    PowerwallTrackingModule::~PowerwallTrackingModule() {

        // intentionally left empty
    }

    void PowerwallTrackingModule::internalInitialize(const ghoul::Dictionary& dict) {

        // TRACKING INITIALISATION ////////////////////////////////////////////////

        /// Tracker Parameters
        tracking::Tracker::Params tp;
        std::string active_node = ""; // Allowing all clients to access tracking data
        tp.active_node = active_node;
        std::string client_ip = "129.69.205.76"; // = MINYOU
        tp.natnet_params.client_ip = client_ip;
        std::string server_ip = "129.69.205.86"; // = MINI
        tp.natnet_params.server_ip = server_ip;
        tp.natnet_params.cmd_port = 1510;
        tp.natnet_params.data_port = 1511;
        tp.natnet_params.con_type = tracking::NatNetDevicePool::ConnectionType::UniCast;
        tp.natnet_params.verbose_client = true;

        tracking::VrpnDevice<vrpn_Button_Remote>::Params bp;
        const std::string device_name = "ControlBox"; // Leave empty to disable use of VRPN device
        bp.device_name = device_name;
        bp.server_ip = server_ip;
        bp.port = 3884;
        bp.protocol = tracking::VrpnDevice<vrpn_Button_Remote>::Protocols::VRPN_TCP;

        std::vector<tracking::VrpnDevice<vrpn_Button_Remote>::Params> bps;
        bps.emplace_back(bp);

        tp.vrpn_params = bps.data();
        tp.vrpn_params_count = bps.size();

        /// ----- Tracker -----
        /*
        // Handles all communication with NatNet and VRPN.
        // Creating one (!) Tracker which runs separate thread for receiving tracking data.
        auto tracker = std::make_shared<tracking::Tracker>();
        if (!tracker->Initialise(tp)) {
            std::cerr << std::endl << "[ERROR] [test] Unable to initialise <Tracker>. " << "[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
            return 1;
        }
        if (!tracker->Connect()) {
            std::cerr << std::endl << "[ERROR] [test] Failed to establish <Tracker> connection. " << "[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
            return 1;
        }
        size_t rigiBodyCount = tracker->GetRigidBodyCount();
        std::vector<std::string> rigidBodies;
        for (size_t i = 0; i < rigiBodyCount; i++) {
            rigidBodies.emplace_back(std::string(tracker->GetRigidBodyName(i)));
        }
        */

        // TRACKING UTILIZERS /////////////////////////////////////////////////////

        /// TrackingUtilizer Parameters
        tracking::TrackingUtilizer::Params tup;
        tup.rigid_body_name = ""; // Will be set in line 116 depending on what rigid bodies are available.
        tup.btn_device_name = device_name; // Insert bp.device_name only for rigid body you want to use.
        tup.select_btn = 0;
        tup.rotate_btn = 1;
        tup.translate_btn = 2;
        tup.zoom_btn = 3;
        tup.invert_rotate = true;
        tup.invert_translate = true;
        tup.invert_zoom = true;
        tup.rotate_speed = 1.0f;
        tup.translate_speed = 1.0f;
        tup.zoom_speed = 1.0f;
        tup.single_interaction = false;
        tup.fov_mode = tracking::TrackingUtilizer::FovMode::WIDTH_AND_ASPECT_RATIO;
        tup.fov_height = 0.2f;
        tup.fov_width = 0.2f;
        tup.fov_horiz_angle = 60.0f;
        tup.fov_vert_angle = 30.0f;
        tup.fov_aspect_ratio = tracking::TrackingUtilizer::FovAspectRatio::AR_1_77__1; // 16:9
        tup.physical_height = 2.4f;
        tup.physical_width = 6.0f;
        tup.physical_origin = glm::vec3(-3.0f, 0.3f, 0.0f);
        tup.physical_x_dir = glm::vec3(1.0f, 0.0f, 0.0f);
        tup.physical_y_dir = glm::vec3(0.0f, 1.0f, 0.0f);

        /// ----- Tracking Utilizers -----
        /*
        // Manage tracking data for one rigid body each.
        // Creating a TrackingUtilizer for each found rigid body (and corresponding button device).
        // Initialising TrackingUtilizer with exisiting Tracker.
        std::vector<tracking::TrackingUtilizer> utilizers;
        utilizers.clear();
        for (auto& rb : rigidBodies) {
            tup.rigid_body_name = rb;
            utilizers.emplace_back(tracking::TrackingUtilizer());
            if (!utilizers.back().Initialise(tup, tracker)) {
                std::cerr << std::endl << "[ERROR] [test] Failed to initialise <TrackingUtilizer>. " << "[" << __FILE__ << ", " << __FUNCTION__ << ", line " << __LINE__ << "]" << std::endl << std::endl;
                return 1;

            }
        }
        */

        // LOOP ///////////////////////////////////////////////////////////////////
        /*
        unsigned int btn;
        float pos_x, pos_y, pos_z;
        float orient_x, orient_y, orient_z, orient_w;
        float inters_x, inters_y;
        bool btn_state, rb_state;

        bool exit = false;
        while (!exit) {

            // Get current tracking data for each rigid body managed by the TrackingUtilizers.
            for (auto& tu : utilizers) {

                // Button State
                btn_state = tu.GetRawData(btn, pos_x, pos_y, pos_z, orient_x, orient_y, orient_z, orient_w);
                // Intersection
                rb_state = tu.GetIntersection(inters_x, inters_y);

                if (rb_state) {

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

            // Wait for user input (press 'ESC') to end loop
            std::cout << "[Press 'ESC' to exit program]" << std::endl << std::endl;
            int key = 0x1B; // ESC
            if (GetAsyncKeyState(key)) {
                exit = true;
            }

            // Sleep some ms to limit data output
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        */
    }

} // namespace openspace
