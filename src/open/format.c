#include <stdarg.h>
#include <kf/lib/address.h>
#include <kf/lib/debug.h>
#include <kf/lib/types.h>

/* Numeric scratch: eight leading bytes receive left padding (at most seven
 * are written), then a sign, ten decimal digits and NUL from the digit anchor
 * at 0x80037978. The claim is the smallest eight-byte-rounded reservation that
 * covers that span; the original allocation's outer bounds remain unresolved. */
DATA(0x80037970, 0x18)
static char format_number_storage[24];

#include "../lib/format.inc"
