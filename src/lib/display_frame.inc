ADDRESS_AT("GAME", 0x8001bfb8, 0x98)
ADDRESS_AT("OPEN", 0x80016d38, 0x98)
void display_begin_frame(void)
{
    KF_GRAPHICS_RUNTIME.display_state.buffer_index = display_next_buffer(KF_GRAPHICS_RUNTIME.display_state.buffer_index);
    KF_GRAPHICS_RUNTIME.display_state.primitive_buffer = &KF_GRAPHICS_RUNTIME.display_state.primitive_buffers[KF_ENUM_ENCODE(u8, KF_GRAPHICS_RUNTIME.display_state.buffer_index)];
    KF_ACTIVE_ORDERING_TABLE =
        KF_GRAPHICS_RUNTIME.display_state.ordering_tables[KF_ENUM_ENCODE(u8, KF_GRAPHICS_RUNTIME.display_state.buffer_index)].entries;
    ClearOTagR(KF_ACTIVE_ORDERING_TABLE, KF_ORDERING_TABLE_LENGTH);
    KF_GRAPHICS_RUNTIME.display_state.primitive_buffer->cursor = KF_GRAPHICS_RUNTIME.display_state.primitive_buffer->start;
#ifdef KF_OPEN
    primitive_allocation_count = 0;
    KF_GRAPHICS_RUNTIME.DAT_8006e044 = 0;
    KF_GRAPHICS_RUNTIME.DAT_8006e040 = 0;
#else
    DAT_800a0768 = 0;
    KF_GRAPHICS_RUNTIME.DAT_8009569c = 0;
    KF_GRAPHICS_RUNTIME.DAT_80095698 = 0;
#endif
}

ADDRESS_AT("GAME", 0x8001c050, 0x98)
ADDRESS_AT("OPEN", 0x80016dd0, 0x98)
void display_present_frame(void)
{
    DrawSync(0);
    VSync(0);
    PutDrawEnv(&KF_GRAPHICS_RUNTIME.display_draw_environments[KF_ENUM_ENCODE(u8, KF_GRAPHICS_RUNTIME.display_state.buffer_index)]);
    PutDispEnv(&KF_GRAPHICS_RUNTIME.display_disp_environments[KF_ENUM_ENCODE(u8, KF_GRAPHICS_RUNTIME.display_state.buffer_index)]);
    DrawOTag(KF_ACTIVE_ORDERING_TABLE + (KF_ORDERING_TABLE_LENGTH - 1));
}
