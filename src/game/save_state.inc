// Version one lists every persisted field explicitly. Runtime pointers are
// deliberately absent; enum widths and byte order do not depend on the host ABI.
struct SavedGameState {
    KfPlayerState player;
    KfMapSavedWorld world;
    u8 stock[KF_ITEM_STOCK_BANK_COUNT][KF_ITEM_COUNT];
    KfMagicLearningState learned[KF_MAGIC_RECORD_COUNT];
};
enum class SaveCodecMode { Read, Write };
struct SaveCodec {
    u8 *data;
    std::size_t size, position;
    SaveCodecMode mode;
    bool valid;
};

template <typename T, typename Wire>
static void save_assign(T &field, Wire value) { field = static_cast<T>(value); }
template <typename Enum, typename Storage, typename Wire>
static void save_assign(KfEnumStorage<Enum, Storage> &field, Wire value) { field = static_cast<Enum>(value); }

template <typename Wire, typename T>
static void save_field(SaveCodec &io, T &field) {
    static_assert(std::is_integral_v<Wire> && sizeof(Wire) <= 4);
    if (!io.valid || sizeof(Wire) > io.size - io.position) {
        io.valid = false;
        return;
    }
    u32 bits = 0;
    if (io.mode == SaveCodecMode::Write) {
        if constexpr (std::is_integral_v<T>)
            bits = static_cast<Wire>(field);
        else
            bits = kf_enum_encode<Wire>(field);
        for (unsigned i = 0; i < sizeof(Wire); ++i)
            io.data[io.position++] = bits >> (i * 8);
    } else {
        for (unsigned i = 0; i < sizeof(Wire); ++i)
            bits |= static_cast<u32>(io.data[io.position++]) << (i * 8);
        save_assign(field, static_cast<Wire>(bits));
    }
}

template <std::size_t Count>
static void save_bytes(SaveCodec &io, u8 (&bytes)[Count]) {
    for (u8 &byte : bytes)
        save_field<u8>(io, byte);
}

static void save_state_fields(SaveCodec &io, SavedGameState &state) {
    auto &p = state.player;
#define SAVE_FIELD(type, name) save_field<type>(io, p.name)
#define SAVE_BYTES(name) save_bytes(io, p.name)
    SAVE_FIELD(s32, experience);
    SAVE_FIELD(s32, next_level_experience);
    SAVE_FIELD(u8, progress_state.level);
    SAVE_FIELD(u8, progress_state.unknown_01);
    SAVE_FIELD(u8, progress_state.current_floor);
    SAVE_FIELD(u8, progress_state.highest_floor);
    SAVE_FIELD(u8, map_variant);
    SAVE_FIELD(u8, allow_near_actor_spawn);
    SAVE_FIELD(u8, weapon_charge_delay);
    SAVE_FIELD(u8, unknown_0f);
    SAVE_FIELD(u16, vitals.maximum_hp);
    SAVE_FIELD(u16, vitals.current_hp);
    SAVE_FIELD(u16, vitals.maximum_mp);
    SAVE_FIELD(u16, vitals.current_mp);
    SAVE_FIELD(u16, attack_charge_state.current);
    SAVE_FIELD(u16, attack_charge_state.committed);
    SAVE_FIELD(u16, magic_charge);
    SAVE_FIELD(u16, physical_power_training);
    SAVE_FIELD(u16, magic_training);
    SAVE_FIELD(u16, base_physical_power);
    SAVE_FIELD(u16, base_magic);
    SAVE_FIELD(u16, physical_power);
    SAVE_FIELD(u16, magic);
    SAVE_FIELD(u16, status_effect_flags);
    SAVE_FIELD(u32, gold);
    SAVE_FIELD(u16, cutting_attack);
    SAVE_FIELD(u16, striking_attack);
    SAVE_FIELD(u16, piercing_attack);
    SAVE_FIELD(u16, holy_attack);
    SAVE_FIELD(u16, fire_attack);
    SAVE_BYTES(unknown_3a);
    SAVE_FIELD(u16, cutting_defense);
    SAVE_FIELD(u16, striking_defense);
    SAVE_FIELD(u16, piercing_defense);
    SAVE_FIELD(u16, poison_resistance);
    SAVE_FIELD(u16, magic_defense);
    SAVE_FIELD(u16, fire_defense);
    SAVE_FIELD(s16, curse_timer);
    SAVE_FIELD(s16, darkness_timer);
    SAVE_FIELD(s16, poison_timer);
    SAVE_FIELD(s16, slowed_timer);
    SAVE_FIELD(s16, fire_defense_timer);
    SAVE_FIELD(s16, illusion_staff_timer);
    SAVE_BYTES(unknown_54);
    SAVE_FIELD(u32, equipment_effect_ticks);
    SAVE_FIELD(u8, selected_magic_id);
    SAVE_BYTES(unknown_5d);
    SAVE_FIELD(u8, equipped_weapon_id);
    SAVE_BYTES(unknown_65);
    SAVE_FIELD(s16, weapon_attack_phase);
    SAVE_BYTES(unknown_72);
    SAVE_FIELD(u8, weapon_magic_shots_remaining);
    SAVE_FIELD(u8, weapon_magic_delay);
    SAVE_FIELD(u8, weapon_attack_fully_charged);
    SAVE_BYTES(unknown_7b);
    SAVE_FIELD(u8, equipped_head_armor_id);
    SAVE_FIELD(u8, equipped_body_armor_id);
    SAVE_FIELD(u8, equipped_shield_id);
    SAVE_FIELD(u8, equipped_arm_armor_id);
    SAVE_FIELD(u8, equipped_leg_armor_id);
    SAVE_FIELD(u8, equipped_accessory_id);
    SAVE_FIELD(u8, audio_effects_enabled);
    SAVE_FIELD(u8, audio_music_enabled);
    SAVE_FIELD(u8, hud_gauges_enabled);
    SAVE_FIELD(u8, compass_enabled);
    SAVE_FIELD(s16, view_rotation_offset.vx);
    SAVE_FIELD(s16, view_rotation_offset.vy);
    SAVE_FIELD(s16, view_rotation_offset.vz);
    SAVE_FIELD(u8, update_state);
    SAVE_FIELD(u8, unknown_a3);
    SAVE_FIELD(s32, camera_position.vx);
    SAVE_FIELD(s32, camera_position.vy);
    SAVE_FIELD(s32, camera_position.vz);
    SAVE_FIELD(s32, floor_height);
    SAVE_FIELD(s16, camera_rotation.vx);
    SAVE_FIELD(s16, camera_rotation.vy);
    SAVE_FIELD(s16, camera_rotation.vz);
    SAVE_FIELD(s16, motion_state.fields.strafe_velocity);
    SAVE_FIELD(s16, motion_state.fields.forward_velocity);
    SAVE_FIELD(u16, motion_state.fields.movement_speed);
    SAVE_FIELD(s16, motion_state.fields.yaw_step);
    SAVE_FIELD(s16, motion_state.fields.pitch_step);
    SAVE_FIELD(u8, motion_state.fields.map_cell.coords.x);
    SAVE_FIELD(u8, motion_state.fields.map_cell.coords.z);
    SAVE_FIELD(u8, previous_map_cell.coords.x);
    SAVE_FIELD(u8, previous_map_cell.coords.z);
    SAVE_BYTES(unknown_ce);
    SAVE_FIELD(s16, view_bob_offset);
    SAVE_FIELD(u16, view_bob_phase);
    SAVE_FIELD(u16, death_camera_pitch_step);
    SAVE_FIELD(s16, death_visual_blend);
    SAVE_FIELD(s16, vertical_velocity);
    SAVE_FIELD(u8, vertical_state);
    SAVE_BYTES(unknown_df);
#undef SAVE_FIELD
#undef SAVE_BYTES
    // The existing world persistence producer already writes bounded-width
    // script/record byte streams. Neither field contains native pointers.
    for (auto &floor : state.world.floors) {
        save_bytes(io, floor.script.bytes);
        save_bytes(io, floor.records);
    }
    for (auto &stock : state.stock)
        save_bytes(io, stock);
    for (auto &learned : state.learned)
        save_field<u8>(io, learned);
}

static u32 save_checksum(const u8 *data, std::size_t size) {
    u32 crc = ~u32{0};
    for (std::size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = (crc >> 1) ^ (0xedb88320u & (0u - (crc & 1)));
    }
    return ~crc;
}

static bool save_world_valid(const KfMapSavedWorld &world) {
    static_assert(sizeof(KfMapObjectLink) == 8, "Update the floor record boundary when the compact link representation changes");
    for (const auto &floor : world.floors) {
        if (floor.records[0] == 0)
            continue;
        if (floor.records[0] != 1)
            return false;
        std::size_t position = 1;
        for (unsigned i = 0; i < KF_MAP_EVENT_CAPACITY; ++i, position += 7) {
            const auto state = kf_enum_decode<KfMapEventState>(floor.records[position]);
            const auto stage = floor.records[position + 2];
            if (floor.records[position + 1] > KF_DIALOGUE_STAGE_COUNT || stage > KF_DIALOGUE_STAGE_COUNT
                    || (state != KF_MAP_EVENT_FREE && stage == 0))
                return false;
        }
        const unsigned actors = floor.records[position++];
        if (actors > KF_ACTOR_CAPACITY)
            return false;
        for (unsigned i = 0; i < actors; ++i, position += 2)
            if (floor.records[position] >= KF_ACTOR_CAPACITY)
                return false;
        const u8 *object_ids = floor.records + position;
        for (unsigned i = 0; i < KF_MAP_OBJECT_CAPACITY; ++i) {
            const auto id = floor.records[position++];
            if (id >= KF_MAP_OBJECT_DEFINITION_COUNT && id != kf_enum_encode<u8>(KF_OBJECT_NONE))
                return false;
            if (id != kf_enum_encode<u8>(KF_OBJECT_NONE) && i >= KF_MAP_OBJECT_GOLD_DROP_FIRST) {
                if (i < KF_MAP_OBJECT_DEFINITION_DROP_FIRST ? id != kf_enum_encode<u8>(KF_ITEM_GOLD_COIN) : id >= KF_ITEM_COUNT)
                    return false;
            }
        }
        const unsigned objects = floor.records[position++];
        // Eight serialized link bytes, preceded by the original object index.
        constexpr unsigned link_record_bytes = 9;
        constexpr unsigned drop_bytes = KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY * (4 + 2 * 3);
        if (objects > KF_MAP_OBJECT_EFFECT_FIRST || objects * link_record_bytes + drop_bytes > sizeof floor.records - position)
            return false;
        for (unsigned i = 0; i < objects; ++i, position += link_record_bytes) {
            const unsigned index = floor.records[position];
            if (index >= KF_MAP_OBJECT_EFFECT_FIRST)
                return false;
            const auto id = object_ids[index];
            if (id == kf_enum_encode<u8>(KF_OBJECT_NONE))
                return false;
            KfMapObjectLink link{};
            std::memcpy(&link, floor.records + position + 1, sizeof link);
            if (!map_saved_link_valid(map_object_state.definitions.entries[id].behavior_type, link))
                return false;
        }
        for (unsigned group = 0; group < 3; ++group) {
            for (unsigned i = 0; i < KF_MAP_OBJECT_EFFECT_GROUP_CAPACITY; ++i) {
                if (floor.records[position] >= KF_MAP_COLUMNS || floor.records[position + 1] >= KF_MAP_ROWS)
                    return false;
                position += group == 0 ? 4 : 3;
            }
        }
    }
    return true;
}

static bool save_state_valid(const SavedGameState &state) {
    const auto &p = state.player;
    const auto floor = kf_enum_encode<u8>(p.progress_state.current_floor);
    const auto highest = kf_enum_encode<u8>(p.progress_state.highest_floor);
    const auto weapon = kf_enum_encode<u8>(p.equipped_weapon_id);
    const auto magic = kf_enum_encode<u8>(p.selected_magic_id);
    if (p.vitals.maximum_hp == 0 || p.vitals.maximum_mp == 0
            || floor < 1 || floor > KF_MAP_SAVED_FLOOR_COUNT || highest < 1 || highest > KF_MAP_SAVED_FLOOR_COUNT
            || (weapon != 255 && weapon >= KF_WEAPON_RECORD_COUNT)
            || (magic != 255 && magic >= KF_MAGIC_PLAYER_COUNT)
            || p.camera_position.vx < 0 || p.camera_position.vx >= KF_MAP_COLUMNS * KF_MAP_TILE_SIZE
            || p.camera_position.vz < 0 || p.camera_position.vz >= KF_MAP_ROWS * KF_MAP_TILE_SIZE
            || p.motion_state.fields.map_cell.coords.x >= KF_MAP_COLUMNS
            || p.motion_state.fields.map_cell.coords.z >= KF_MAP_ROWS)
        return false;
    const KfObjectId armor[] = {p.equipped_head_armor_id, p.equipped_body_armor_id,
        p.equipped_shield_id, p.equipped_arm_armor_id, p.equipped_leg_armor_id};
    for (const auto id : armor) {
        const int index = kf_enum_encode<u8>(id) - kf_enum_encode<u8>(KF_ITEM_IRON_MASK);
        if (id != KF_OBJECT_NONE && (index < 0 || index >= KF_ARMOR_RECORD_COUNT))
            return false;
    }
    const auto accessory = kf_enum_encode<u8>(p.equipped_accessory_id);
    if (accessory != 255 && accessory >= KF_ITEM_COUNT)
        return false;
    for (const auto flag : state.learned)
        if (flag != KF_MAGIC_UNLEARNED && flag != KF_MAGIC_LEARNED)
            return false;
    return save_world_valid(state.world);
}

static KfArmorRecord *saved_armor(KfObjectId id) {
    return id == KF_OBJECT_NONE ? nullptr : &armor_records.entries[kf_enum_encode<u8>(id) - kf_enum_encode<u8>(KF_ITEM_IRON_MASK)];
}

static void save_state_apply(const SavedGameState &state) {
    auto *asset = player_state.weapon_asset_buffer;
    auto *cache = player_state.weapon_animation_cache;
    player_state = state.player;
    player_state.weapon_asset_buffer = asset;
    player_state.weapon_animation_cache = cache;
    player_state.selected_magic_record = player_state.selected_magic_id == KF_MAGIC_NONE ? nullptr
        : &magic_records[kf_enum_encode<u8>(player_state.selected_magic_id)];
    player_state.equipped_weapon_record = player_state.equipped_weapon_id == KF_OBJECT_NONE ? nullptr
        : &weapon_records.entries[kf_enum_encode<u8>(player_state.equipped_weapon_id)];
    player_state.equipped_head_armor_record = saved_armor(player_state.equipped_head_armor_id);
    player_state.equipped_body_armor_record = saved_armor(player_state.equipped_body_armor_id);
    player_state.equipped_shield_record = saved_armor(player_state.equipped_shield_id);
    player_state.equipped_arm_armor_record = saved_armor(player_state.equipped_arm_armor_id);
    player_state.equipped_leg_armor_record = saved_armor(player_state.equipped_leg_armor_id);
    map_runtime_state.world_state = state.world;
    std::memcpy(item_stock, state.stock, sizeof state.stock);
    for (unsigned i = 0; i < KF_MAGIC_RECORD_COUNT; ++i)
        magic_records[i].learned = state.learned[i];
    // The existing load-return path reloads the floor, weapon and selected magic.
}
