#include <harp_c_app.h>

HarpCApp& HarpCApp::init(uint16_t who_am_i,
                         uint8_t hw_version_major, uint8_t hw_version_minor,
                         uint8_t assembly_version,
                         uint8_t harp_version_major, uint8_t harp_version_minor,
                         uint8_t fw_version_major, uint8_t fw_version_minor,
                         uint16_t serial_number, const char name[],
                         const uint8_t tag[],
                         void* app_reg_values, RegSpecs* app_reg_specs,
                         RegFnPair* app_reg_fns, size_t app_reg_count,
                         void (* update_fn)(void), void (* reset_fn)(void))
{
    static HarpCApp app(who_am_i, hw_version_major, hw_version_minor,
                        assembly_version,
                        harp_version_major, harp_version_minor,
                        fw_version_major, fw_version_minor, serial_number,
                        name, tag, app_reg_values, app_reg_specs,
                        app_reg_fns, app_reg_count, update_fn, reset_fn);
    return app;
}

HarpCApp::HarpCApp(uint16_t who_am_i,
                   uint8_t hw_version_major, uint8_t hw_version_minor,
                   uint8_t assembly_version,
                   uint8_t harp_version_major, uint8_t harp_version_minor,
                   uint8_t fw_version_major, uint8_t fw_version_minor,
                   uint16_t serial_number, const char name[],
                   const uint8_t tag[],
                   void* app_reg_values, RegSpecs* app_reg_specs,
                   RegFnPair* app_reg_fns, size_t app_reg_count,
                   void (*update_fn)(void), void (* reset_fn)(void))
:reg_values_{app_reg_values},
 reg_specs_{app_reg_specs},
 reg_fns_{app_reg_fns},
 reg_count_{app_reg_count},
 update_fn_{update_fn},
 reset_fn_{reset_fn},
 HarpCore(who_am_i, hw_version_major, hw_version_minor,
          assembly_version, harp_version_major, harp_version_minor,
          fw_version_major, fw_version_minor, serial_number, name, tag)
{
    // Call base class constructor.
    // Create a ptr to the first (and only) derived class instance created.
    if (self == nullptr)
        self = this;
}

HarpCApp::~HarpCApp(){self = nullptr;}

void HarpCApp::handle_buffered_app_message()
{
    msg_t msg = get_buffered_msg();
    // Ignore out-of-range msgs.
    if (msg.header.address < APP_REG_START_ADDRESS ||
        msg.header.address >= (APP_REG_START_ADDRESS + reg_count_))
        return;
    uint8_t app_reg_address = msg.header.address - APP_REG_START_ADDRESS;
    switch (msg.header.type)
    {
        // Note: handler functions take the full address, but they live in
        // pairs in a separate struct indexed by app register address.
        case READ:
            reg_fns_[app_reg_address].read_fn_ptr(msg.header.address);
            break;
        case WRITE:
            reg_fns_[app_reg_address].write_fn_ptr(msg);
            break;
        default:
        {
            break;
        }
    }
    clear_msg();
}

void HarpCApp::dump_app_registers()
{
    for (uint8_t address = APP_REG_START_ADDRESS;
         address < reg_count_ + APP_REG_START_ADDRESS; ++address)
        send_harp_reply(READ, address);
}
