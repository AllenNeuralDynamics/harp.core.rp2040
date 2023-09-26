#include <harp_c_app.h>

HarpCApp& HarpCApp::init(uint16_t who_am_i,
                         uint8_t hw_version_major, uint8_t hw_version_minor,
                         uint8_t assembly_version,
                         uint8_t harp_version_major, uint8_t harp_version_minor,
                         uint8_t fw_version_major, uint8_t fw_version_minor,
                         uint16_t serial_number, const char name[],
                         void* app_reg_values, RegSpecs* app_reg_specs,
                         RegFnPair* app_reg_fns, size_t app_reg_count,
                         void (* update_fn)(void))
{
    static HarpCApp app(who_am_i, hw_version_major, hw_version_minor,
                        assembly_version,
                        harp_version_major, harp_version_minor,
                        fw_version_major, fw_version_minor, serial_number,
                        name, app_reg_values, app_reg_specs, app_reg_fns,
                        app_reg_count, update_fn);
    return app;
}

HarpCApp::HarpCApp(uint16_t who_am_i,
                   uint8_t hw_version_major, uint8_t hw_version_minor,
                   uint8_t assembly_version,
                   uint8_t harp_version_major, uint8_t harp_version_minor,
                   uint8_t fw_version_major, uint8_t fw_version_minor,
                   uint16_t serial_number, const char name[],
                   void* app_reg_values, RegSpecs* app_reg_specs,
                   RegFnPair* app_reg_fns, size_t app_reg_count,
                   void (*update_fn)(void))
:reg_values_{app_reg_values},
 reg_specs_{app_reg_specs},
 reg_fns_{app_reg_fns},
 reg_count_{app_reg_count},
 update_fn_{update_fn},
 HarpCore(who_am_i, hw_version_major, hw_version_minor,
          assembly_version, harp_version_major, harp_version_minor,
          fw_version_major, fw_version_minor, serial_number, name)
{
    // Call base class constructor.
    // Create a ptr to the first (and only) derived class instance created.
    if (self == nullptr)
        self = this;
}

HarpCApp::~HarpCApp(){self = nullptr;}

void HarpCApp::run()
{
    HarpCore::run();
    // Handle in-range register msgs. Ignore msgs outside our range.
    msg_header_t& header = get_buffered_msg_header();
    if (header.address-32 < reg_count_) // FIXME: unhardcode 32.
        handle_buffered_core_message();
}

void HarpCApp::update_state()
{
    HarpCore::update_state(); // update Harp Core state first.
    update_fn_();   // Update app state.
}

void HarpCApp::read_app_reg_generic(uint8_t reg_name)
{

}

void HarpCApp::write_app_reg_generic(msg_t& msg)
{

}
