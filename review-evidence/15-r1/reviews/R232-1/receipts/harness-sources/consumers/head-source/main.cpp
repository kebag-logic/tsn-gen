#include <tsn/session.h>
#ifndef R232_PARENT_OPTION
#error parent option lost
#endif
int main() { tsn::Session session(CORPUS); return session.parse() ? 0 : 1; }
