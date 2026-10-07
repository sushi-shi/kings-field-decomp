#include <kf/lib/tmd.h>

#define OFFSET(type, field) ((unsigned long)&((type *)0)->field)
#define CHECK(name, condition) typedef char name[(condition) ? 1 : -1]

#ifndef EXPECTED_VECTOR_OFFSET_SHIFT
#define EXPECTED_VECTOR_OFFSET_SHIFT KF_TMD_VECTOR_OFFSET_SHIFT
#endif

/* Prepared indices are byte offsets of whole elements: the shift must remain
 * one element, or every normal and vertex address lands mid-record. */
CHECK(vector_offset_shift,
    (1 << EXPECTED_VECTOR_OFFSET_SHIFT) == sizeof(SVECTOR));
CHECK(prepared_vertex_stride,
    (1 << EXPECTED_VECTOR_OFFSET_SHIFT) == sizeof(KfScreenVertex));

/* The GTE reads and writes these vectors with word loads and stores, which C
 * does not require: SVECTOR is four shorts and only needs two-byte alignment.
 * A word-sized element keeps every index-derived address word-aligned once the
 * block base is. */
CHECK(svector_extent, sizeof(SVECTOR) == 8);
CHECK(svector_c_alignment_is_weaker_than_the_gte_needs,
    __alignof__(SVECTOR) == 2);
CHECK(element_carries_word_alignment, sizeof(SVECTOR) % 4 == 0);

/* Object block offsets are added to the asset base past a whole-word header,
 * so a word-aligned asset keeps vertex and normal blocks word-aligned. */
CHECK(header_is_whole_words, KF_TMD_HEADER_BYTES % 4 == 0);
CHECK(header_matches_record, KF_TMD_HEADER_BYTES == sizeof(KfTmdHeader));
CHECK(packet_header_is_a_word, KF_TMD_PACKET_HEADER_BYTES == KF_TMD_WORD_BYTES);

/* RotTrans is handed &MATRIX.t typed as VECTOR *. VECTOR is four words and t
 * is three, and t ends the object, so the fourth word of the view lies past
 * the MATRIX. Retail RotTrans (GAME.EXE 0x8004dadc) stores exactly three
 * words, at 0, 4 and 8, which is what keeps the call in bounds. */
CHECK(matrix_translation_is_three_words,
    sizeof(((MATRIX *)0)->t) == 3 * sizeof(long));
CHECK(vector_view_is_wider_than_the_translation,
    sizeof(VECTOR) > sizeof(((MATRIX *)0)->t));
CHECK(translation_ends_the_matrix,
    OFFSET(MATRIX, t) + sizeof(((MATRIX *)0)->t) == sizeof(MATRIX));
CHECK(translation_is_word_aligned, OFFSET(MATRIX, t) % 4 == 0);
