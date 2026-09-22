// Embedded consumer control: parse the shipped corpus, generate one
// interface-list stack frame and one logic-driven stack frame, print both.
#include <tsn/session.h>
#include <tsn/stack.h>

#include <cstdio>
#include <memory>

int main()
{
    tsn::Session session(PROTOCOLS_DIR);
    if (!session.parse()) {
        std::fprintf(stderr, "parse failed\n");
        return 1;
    }
    session.seed(42);
    const tsn::ProtocolInterface* eth =
        session.findInterface("ethernet_mac_frame::ETHERNET_FRAME::ETHERNET_FRAME_IF");
    const tsn::ProtocolInterface* avtp =
        session.findInterface("avtp_control_header::AVTP_CONTROL::AVTP_CONTROL_IF");
    const tsn::ProtocolInterface* aecp = session.findInterface(
        "atdecc_aecp_acquire_entity::AECP_ACQUIRE_ENTITY::AECP_ACQUIRE_ENTITY_IF");
    if (!eth || !avtp || !aecp) {
        std::fprintf(stderr, "interface lookup failed\n");
        return 1;
    }
    auto frame = session.generateStack({eth, avtp, aecp});
    std::printf("interface stack: %zu bytes %s\n", frame.bytes.size(),
                tsn::Session::toHex(frame.bytes).c_str());

    std::unique_ptr<tsn::Stack> stack;
    if (session.loadStack(STACK_FILE, stack).getErrorCode() !=
        tsn::StackBuilderErr::STACK_SUCCESS) {
        std::fprintf(stderr, "loadStack failed\n");
        return 1;
    }
    auto logic = session.generateStack(*stack);
    std::printf("logic stack: %zu bytes %s\n", logic.bytes.size(),
                tsn::Session::toHex(logic.bytes).c_str());
    return frame.bytes.size() == 55 && logic.bytes.size() == 55 ? 0 : 1;
}
