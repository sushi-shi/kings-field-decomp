# Formatter retained-literal ledger

Complete accounting for three files and **88 retained occurrences** after
38 state literals became typed enum members. See the [evidence review](formatter-state-domains.md).
Each duplicate occurrence has its own row. Claims, named definitions and
string contents are excluded.

## `src/game/debug_text.c`

45 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 17 | `0` | `s32 debug_stop_flag = 0;` | The initialized word starts cleared. Its only reviewed writer logically inverts it; no admitted caller or consumer proves a pause or stop-state enum. |
| `initializers` | 20 | `24` | `static char format_number_buffer[24];` | Preserve the current twenty-four-byte scratch-anchor claim. Backward padding writes precede this object; its original enclosing allocation remains unresolved. |
| `debug_stop` | 26 | `0` | `debug_stop_flag = debug_stop_flag == 0;` | Logical inversion of a word marker, including mapping every nonzero input to zero; no evidence establishes a larger semantic state domain. |
| `format_int_dec` | 37 | `0` | `if (value < 0) {` | Signed arithmetic boundary for emitting a minus sign. The existing minimum-signed-value negation issue is unchanged. |
| `format_int_dec` | 38 | `'-'` | `*out++ = '-';` | The visible negative-number sign, emitted before the digit-state flag changes. |
| `format_int_dec` | 41 | `0` | `for (i = 0; i < KF_FORMAT_DECIMAL_DIGITS; i++) {` | Begin the zero-based traversal of the named number of digit places. |
| `format_int_dec` | 44 | `0` | `if (digit != 0 \|\| digit_state != KF_FORMAT_DIGITS_LEADING \|\| i == KF_FORMAT_DECIMAL_DIGITS - 1) {` | Arithmetic zero digit suppression before any digit has been emitted. |
| `format_int_dec` | 44 | `1` | `if (digit != 0 \|\| digit_state != KF_FORMAT_DIGITS_LEADING \|\| i == KF_FORMAT_DECIMAL_DIGITS - 1) {` | Subtract one from the named digit count to identify the final place, which must emit even for value zero. |
| `format_int_dec` | 45 | `'0'` | `*out++ = digit + '0';` | Character encoding of a numeric digit; the literal makes the conversion explicit. |
| `format_int_dec` | 48 | `10` | `divisor /= 10;` | Decimal radix advances to the next place, distinct from the named ten-place extent. |
| `format_int_dec` | 50 | `'\0'` | `*out = '\0';` | C string terminator, written by the number converters and final output writer. |
| `format_int_hex` | 62 | `0` | `for (i = 0; i < KF_FORMAT_HEX_DIGITS; i++) {` | Begin the zero-based traversal of the named number of digit places. |
| `format_int_hex` | 65 | `0` | `if (digit != 0 \|\| digit_state != KF_FORMAT_DIGITS_LEADING \|\| i == KF_FORMAT_HEX_DIGITS - 1) {` | Arithmetic zero digit suppression before any digit has been emitted. |
| `format_int_hex` | 65 | `1` | `if (digit != 0 \|\| digit_state != KF_FORMAT_DIGITS_LEADING \|\| i == KF_FORMAT_HEX_DIGITS - 1) {` | Subtract one from the named digit count to identify the final place, which must emit even for value zero. |
| `format_int_hex` | 66 | `10` | `if (digit < 10) {` | Digits below ten use numeral characters; the remaining hexadecimal values use uppercase letters. |
| `format_int_hex` | 67 | `'0'` | `*out++ = digit + '0';` | Character encoding of a numeric digit; the literal makes the conversion explicit. |
| `format_int_hex` | 69 | `'A'` | `*out++ = digit + ('A' - 10);` | Uppercase A is the encoded first alphabetic hexadecimal digit. |
| `format_int_hex` | 69 | `10` | `*out++ = digit + ('A' - 10);` | Subtract the numeric value of hexadecimal A so digit ten maps to A. |
| `format_int_hex` | 73 | `4` | `divisor >>= 4;` | Four bits encode one hexadecimal digit; shift to the next place. |
| `format_int_hex` | 75 | `'\0'` | `*out = '\0';` | C string terminator, written by the number converters and final output writer. |
| `format_pad_left` | 82 | `0` | `u8 len = 0;` | The byte string-length accumulator starts empty; retain its original wrapping arithmetic. |
| `format_pad_left` | 83 | `1` | `char *p = string + 1;` | Start the look-ahead scan at the second character after separately checking the first. |
| `format_pad_left` | 85 | `0` | `if (*string != 0) {` | Check the first byte for the C string terminator before incrementing length. |
| `format_pad_left` | 88 | `0` | `} while (*p++ != 0);` | End the look-ahead string scan at NUL. |
| `format_pad_left` | 92 | `0` | `while (width-- != 0) {` | Stop prepending after the remaining padding count reaches zero; preserve the post-decrement. |
| `format_vsprintf` | 102 | `0` | `s32 count = 0;` | No output bytes have been written at parser entry. |
| `format_vsprintf` | 109 | `0` | `while ((c = *format++) != 0) {` | NUL terminates the input format string. |
| `format_vsprintf` | 110 | `'1'` | `if (c >= '1' && c <= '8') {` | The actual inclusive width-character grammar accepts one through eight; successive digits overwrite width rather than accumulating a decimal number. |
| `format_vsprintf` | 110 | `'8'` | `if (c >= '1' && c <= '8') {` | The actual inclusive width-character grammar accepts one through eight; successive digits overwrite width rather than accumulating a decimal number. |
| `format_vsprintf` | 112 | `'0'` | `width = c - '0';` | Decode the accepted ASCII width character into its numeric value. |
| `format_vsprintf` | 118 | `'%'` | `case '%':` | A percent starts or restarts conversion parsing, including repeated percent characters; this parser has no escaped-percent output branch. |
| `format_vsprintf` | 123 | `'0'` | `case '0':` | Inside conversion parsing this selects zero padding; outside it remains an ordinary literal character. |
| `format_vsprintf` | 129 | `'D'` | `case 'D':` | Literal signed decimal conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 130 | `'d'` | `case 'd':` | Literal signed decimal conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 139 | `' '` | `s = format_pad_left(s, ' ', width);` | The actual space byte to prepend in space-padding mode. |
| `format_vsprintf` | 141 | `'0'` | `s = format_pad_left(s, '0', width);` | The actual zero digit to prepend for every nonzero padding mode. |
| `format_vsprintf` | 145 | `0` | `while ((c = *s++) != 0) {` | Copy the selected conversion string through its last non-NUL byte. |
| `format_vsprintf` | 150 | `'X'` | `case 'X':` | Literal unsigned hexadecimal conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 151 | `'x'` | `case 'x':` | Literal unsigned hexadecimal conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 158 | `'S'` | `case 'S':` | Literal string-pointer conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 159 | `'s'` | `case 's':` | Literal string-pointer conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 166 | `'\n'` | `case '\n':` | Recognize a literal line feed independently of conversion state. |
| `format_vsprintf` | 167 | `'\r'` | `*out++ = '\r';` | The original newline mapping writes one carriage return, without adding a line feed. |
| `format_vsprintf` | 177 | `'\0'` | `*out = '\0';` | C string terminator, written by the number converters and final output writer. |
| `format_vsprintf` | 178 | `1` | `return count + 1;` | The returned byte count includes the terminating NUL. |

## `src/open/format.c`

43 retained occurrences.

| Function | Line | Token | Expression | Reason |
| --- | ---: | --- | --- | --- |
| `initializers` | 12 | `19` | `static char format_number_storage[19];` | Current nineteen-byte accessed-span claim: seven leading pad bytes for the reviewed parser calls plus twelve numeric bytes including sign and NUL. This does not establish the original enclosing allocation or arbitrary standalone padding bounds. |
| `format_int_dec` | 22 | `0` | `if (value < 0) {` | Signed arithmetic boundary for emitting a minus sign. The existing minimum-signed-value negation issue is unchanged. |
| `format_int_dec` | 23 | `'-'` | `*out++ = '-';` | The visible negative-number sign, emitted before the digit-state flag changes. |
| `format_int_dec` | 26 | `0` | `for (i = 0; i < KF_FORMAT_DECIMAL_DIGITS; i++) {` | Begin the zero-based traversal of the named number of digit places. |
| `format_int_dec` | 29 | `0` | `if (digit != 0 \|\| digit_state != KF_FORMAT_DIGITS_LEADING \|\| i == KF_FORMAT_DECIMAL_DIGITS - 1) {` | Arithmetic zero digit suppression before any digit has been emitted. |
| `format_int_dec` | 29 | `1` | `if (digit != 0 \|\| digit_state != KF_FORMAT_DIGITS_LEADING \|\| i == KF_FORMAT_DECIMAL_DIGITS - 1) {` | Subtract one from the named digit count to identify the final place, which must emit even for value zero. |
| `format_int_dec` | 30 | `'0'` | `*out++ = digit + '0';` | Character encoding of a numeric digit; the literal makes the conversion explicit. |
| `format_int_dec` | 33 | `10` | `divisor /= 10;` | Decimal radix advances to the next place, distinct from the named ten-place extent. |
| `format_int_dec` | 35 | `'\0'` | `*out = '\0';` | C string terminator, written by the number converters and final output writer. |
| `format_int_hex` | 47 | `0` | `for (i = 0; i < KF_FORMAT_HEX_DIGITS; i++) {` | Begin the zero-based traversal of the named number of digit places. |
| `format_int_hex` | 50 | `0` | `if (digit != 0 \|\| digit_state != KF_FORMAT_DIGITS_LEADING \|\| i == KF_FORMAT_HEX_DIGITS - 1) {` | Arithmetic zero digit suppression before any digit has been emitted. |
| `format_int_hex` | 50 | `1` | `if (digit != 0 \|\| digit_state != KF_FORMAT_DIGITS_LEADING \|\| i == KF_FORMAT_HEX_DIGITS - 1) {` | Subtract one from the named digit count to identify the final place, which must emit even for value zero. |
| `format_int_hex` | 51 | `10` | `if (digit < 10) {` | Digits below ten use numeral characters; the remaining hexadecimal values use uppercase letters. |
| `format_int_hex` | 52 | `'0'` | `*out++ = digit + '0';` | Character encoding of a numeric digit; the literal makes the conversion explicit. |
| `format_int_hex` | 54 | `'A'` | `*out++ = digit + ('A' - 10);` | Uppercase A is the encoded first alphabetic hexadecimal digit. |
| `format_int_hex` | 54 | `10` | `*out++ = digit + ('A' - 10);` | Subtract the numeric value of hexadecimal A so digit ten maps to A. |
| `format_int_hex` | 58 | `4` | `divisor >>= 4;` | Four bits encode one hexadecimal digit; shift to the next place. |
| `format_int_hex` | 60 | `'\0'` | `*out = '\0';` | C string terminator, written by the number converters and final output writer. |
| `format_pad_left` | 67 | `0` | `u8 len = 0;` | The byte string-length accumulator starts empty; retain its original wrapping arithmetic. |
| `format_pad_left` | 68 | `1` | `char *p = string + 1;` | Start the look-ahead scan at the second character after separately checking the first. |
| `format_pad_left` | 70 | `0` | `if (*string != 0) {` | Check the first byte for the C string terminator before incrementing length. |
| `format_pad_left` | 73 | `0` | `} while (*p++ != 0);` | End the look-ahead string scan at NUL. |
| `format_pad_left` | 77 | `0` | `while (width-- != 0) {` | Stop prepending after the remaining padding count reaches zero; preserve the post-decrement. |
| `format_vsprintf` | 87 | `0` | `s32 count = 0;` | No output bytes have been written at parser entry. |
| `format_vsprintf` | 94 | `0` | `while ((c = *format++) != 0) {` | NUL terminates the input format string. |
| `format_vsprintf` | 95 | `'1'` | `if (c >= '1' && c <= '8') {` | The actual inclusive width-character grammar accepts one through eight; successive digits overwrite width rather than accumulating a decimal number. |
| `format_vsprintf` | 95 | `'8'` | `if (c >= '1' && c <= '8') {` | The actual inclusive width-character grammar accepts one through eight; successive digits overwrite width rather than accumulating a decimal number. |
| `format_vsprintf` | 97 | `'0'` | `width = c - '0';` | Decode the accepted ASCII width character into its numeric value. |
| `format_vsprintf` | 103 | `'%'` | `case '%':` | A percent starts or restarts conversion parsing, including repeated percent characters; this parser has no escaped-percent output branch. |
| `format_vsprintf` | 108 | `'0'` | `case '0':` | Inside conversion parsing this selects zero padding; outside it remains an ordinary literal character. |
| `format_vsprintf` | 114 | `'D'` | `case 'D':` | Literal signed decimal conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 115 | `'d'` | `case 'd':` | Literal signed decimal conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 124 | `' '` | `s = format_pad_left(s, ' ', width);` | The actual space byte to prepend in space-padding mode. |
| `format_vsprintf` | 126 | `'0'` | `s = format_pad_left(s, '0', width);` | The actual zero digit to prepend for every nonzero padding mode. |
| `format_vsprintf` | 130 | `0` | `while ((c = *s++) != 0) {` | Copy the selected conversion string through its last non-NUL byte. |
| `format_vsprintf` | 135 | `'X'` | `case 'X':` | Literal unsigned hexadecimal conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 136 | `'x'` | `case 'x':` | Literal unsigned hexadecimal conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 143 | `'S'` | `case 'S':` | Literal string-pointer conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 144 | `'s'` | `case 's':` | Literal string-pointer conversion letter in the accepted grammar, with both uppercase and lowercase forms. Outside conversion mode it is copied unchanged. |
| `format_vsprintf` | 151 | `'\n'` | `case '\n':` | Recognize a literal line feed independently of conversion state. |
| `format_vsprintf` | 152 | `'\r'` | `*out++ = '\r';` | The original newline mapping writes one carriage return, without adding a line feed. |
| `format_vsprintf` | 162 | `'\0'` | `*out = '\0';` | C string terminator, written by the number converters and final output writer. |
| `format_vsprintf` | 163 | `1` | `return count + 1;` | The returned byte count includes the terminating NUL. |

## `src/open/debug.c`

0 retained occurrences.

The disabled variadic sink has no numeric or character literals and performs no formatting or output.
