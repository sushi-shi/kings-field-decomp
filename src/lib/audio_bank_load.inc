static u32 audio_chunk_size(const u8 *data)
{
    return static_cast<u32>(data[0]) | (static_cast<u32>(data[1]) << 8)
        | (static_cast<u32>(data[2]) << 16) | (static_cast<u32>(data[3]) << 24);
}

void audio_load_vab_resource(const u8 *data, std::size_t size)
{
    if (size < 4)
        kf::host_fail("Truncated sound-bank resource");
    const std::size_t header_size = audio_chunk_size(data);
    if (header_size > size - 4 || size - 4 - header_size < 4)
        kf::host_fail("Truncated sound-bank header");
    const u8 *body_chunk = data + 4 + header_size;
    const std::size_t body_size = audio_chunk_size(body_chunk);
    if (body_size > size - 8 - header_size)
        kf::host_fail("Truncated sound-bank samples");
    audio_load_vab(data + 4, header_size, body_chunk + 4, body_size);
}
