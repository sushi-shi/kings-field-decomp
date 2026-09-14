ADDRESS_AT("GAME", 0x80032c78, 0x38)
ADDRESS_AT("OPEN", 0x80019ec0, 0x38)
void audio_shutdown(void)
{
    audio_close_vab();
    SsSeqClose(audio_state.sequence_id);
    SsEnd();
}
