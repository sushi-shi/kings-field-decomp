void audio_shutdown(void)
{
    audio_close_vab();
    SsSeqClose(audio_state.sequence_id);
    SsEnd();
}
