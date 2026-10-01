//! Bounded game framing, commands and input sequencing. No allocation or I/O.
//! The transport consumes these values before exposing typed session events.
use crate::ffi::bindings::*;
pub(crate) mod transport;
pub(crate) mod world;

#[derive(Debug, PartialEq, Eq)]
pub enum Error {
    Invalid,
    Full,
    Empty,
}
type Result<T> = core::result::Result<T, Error>;

struct Reader<'a> {
    bytes: &'a [u8],
    at: usize,
}
impl<'a> Reader<'a> {
    fn new(bytes: &'a [u8]) -> Result<Self> {
        if bytes.len() > KF_NET_PACKET_LIMIT as usize {
            return Err(Error::Invalid);
        }
        Ok(Self { bytes, at: 0 })
    }
    fn take<const N: usize>(&mut self) -> Result<[u8; N]> {
        let end = self.at.checked_add(N).ok_or(Error::Invalid)?;
        let result = self
            .bytes
            .get(self.at..end)
            .ok_or(Error::Invalid)?
            .try_into()
            .map_err(|_| Error::Invalid)?;
        self.at = end;
        Ok(result)
    }
    fn u8(&mut self) -> Result<u8> {
        Ok(self.take::<1>()?[0])
    }
    fn u16(&mut self) -> Result<u16> {
        Ok(u16::from_le_bytes(self.take()?))
    }
    fn u32(&mut self) -> Result<u32> {
        Ok(u32::from_le_bytes(self.take()?))
    }
    fn i32(&mut self) -> Result<i32> {
        Ok(i32::from_le_bytes(self.take()?))
    }
    fn finish(&self) -> Result<()> {
        if self.at == self.bytes.len() {
            Ok(())
        } else {
            Err(Error::Invalid)
        }
    }
}
struct Writer<'a> {
    bytes: &'a mut [u8],
    at: usize,
}
impl Writer<'_> {
    fn put(&mut self, bytes: &[u8]) -> Result<()> {
        let end = self.at.checked_add(bytes.len()).ok_or(Error::Full)?;
        self.bytes
            .get_mut(self.at..end)
            .ok_or(Error::Full)?
            .copy_from_slice(bytes);
        self.at = end;
        Ok(())
    }
    fn u8(&mut self, value: u8) -> Result<()> {
        self.put(&[value])
    }
    fn u16(&mut self, value: u16) -> Result<()> {
        self.put(&value.to_le_bytes())
    }
    fn u32(&mut self, value: u32) -> Result<()> {
        self.put(&value.to_le_bytes())
    }
    fn i32(&mut self, value: i32) -> Result<()> {
        self.put(&value.to_le_bytes())
    }
}
fn header_valid(header: &KfNetHeader) -> Result<()> {
    if !(1..=11).contains(&header.kind) || header.epoch == 0 {
        Err(Error::Invalid)
    } else {
        Ok(())
    }
}
fn read_header(reader: &mut Reader<'_>) -> Result<KfNetHeader> {
    if reader.u32()? != 0x314e464b || reader.u16()? != KF_NET_PROTOCOL_VERSION as u16 {
        return Err(Error::Invalid);
    }
    let header = KfNetHeader {
        kind: reader.u8()?,
        epoch: reader.u32()?,
        sequence: reader.u32()?,
        generation: reader.u32()?,
    };
    header_valid(&header)?;
    Ok(header)
}
fn write_header(writer: &mut Writer<'_>, header: &KfNetHeader) -> Result<()> {
    header_valid(header)?;
    writer.u32(0x314e464b)?;
    writer.u16(KF_NET_PROTOCOL_VERSION as u16)?;
    writer.u8(header.kind)?;
    writer.u32(header.epoch)?;
    writer.u32(header.sequence)?;
    writer.u32(header.generation)
}
pub fn decode_header(bytes: &[u8]) -> Result<KfNetHeader> {
    read_header(&mut Reader::new(bytes)?)
}
pub fn encode_header(header: &KfNetHeader, bytes: &mut [u8]) -> Result<usize> {
    let mut writer = Writer { bytes, at: 0 };
    write_header(&mut writer, header)?;
    Ok(writer.at)
}
fn valid_input(bundle: &KfNetInputBundle) -> Result<()> {
    header_valid(&bundle.header)?;
    if bundle.header.kind != 1
        || bundle.count == 0
        || usize::from(bundle.count) > bundle.frames.len()
    {
        return Err(Error::Invalid);
    }
    let mut previous = 0;
    for frame in &bundle.frames[..usize::from(bundle.count)] {
        if frame.sequence <= previous
            || frame.buttons & !0xf9ff != 0
            || !(-1024..=1024).contains(&frame.yaw)
            || !(-1024..=1024).contains(&frame.pitch)
        {
            return Err(Error::Invalid);
        }
        previous = frame.sequence;
    }
    if bundle.header.sequence != previous {
        return Err(Error::Invalid);
    }
    Ok(())
}
pub fn decode_input(bytes: &[u8]) -> Result<KfNetInputBundle> {
    let mut reader = Reader::new(bytes)?;
    let header = read_header(&mut reader)?;
    let count = reader.u8()?;
    if count == 0 || count > KF_NET_INPUT_REDUNDANCY as u8 {
        return Err(Error::Invalid);
    }
    let zero = KfNetInputFrame {
        sequence: 0,
        tick: 0,
        buttons: 0,
        yaw: 0,
        pitch: 0,
    };
    let mut bundle = KfNetInputBundle {
        header,
        count,
        frames: [zero; KF_NET_INPUT_REDUNDANCY as usize],
    };
    for frame in &mut bundle.frames[..usize::from(count)] {
        *frame = KfNetInputFrame {
            sequence: reader.u32()?,
            tick: reader.u32()?,
            buttons: reader.u32()?,
            yaw: reader.i32()?,
            pitch: reader.i32()?,
        };
    }
    reader.finish()?;
    valid_input(&bundle)?;
    Ok(bundle)
}
pub fn encode_input(bundle: &KfNetInputBundle, bytes: &mut [u8]) -> Result<usize> {
    valid_input(bundle)?;
    let needed = KF_NET_HEADER_BYTES as usize + 1 + usize::from(bundle.count) * 20;
    if bytes.len() < needed {
        return Err(Error::Full);
    }
    let mut writer = Writer { bytes, at: 0 };
    write_header(&mut writer, &bundle.header)?;
    writer.u8(bundle.count)?;
    for frame in &bundle.frames[..usize::from(bundle.count)] {
        writer.u32(frame.sequence)?;
        writer.u32(frame.tick)?;
        writer.u32(frame.buttons)?;
        writer.i32(frame.yaw)?;
        writer.i32(frame.pitch)?;
    }
    Ok(writer.at)
}
fn valid_interaction(view: &KfNetInteraction) -> Result<()> {
    header_valid(&view.header)?;
    if view.header.kind != 11
        || view.header.sequence == 0
        || !(1..=5).contains(&view.floor)
        || view.stage > 5
        || view.character > 99
        || view.page > 9
        || (view.stage == 0) != (view.page == 0)
        || view.shop > 2
    {
        return Err(Error::Invalid);
    }
    Ok(())
}
pub fn decode_interaction(bytes: &[u8]) -> Result<KfNetInteraction> {
    let mut reader = Reader::new(bytes)?;
    let value = KfNetInteraction {
        header: read_header(&mut reader)?,
        floor: reader.u8()?,
        stage: reader.u8()?,
        character: reader.u8()?,
        page: reader.u8()?,
        shop: reader.u8()?,
    };
    reader.finish()?;
    valid_interaction(&value)?;
    Ok(value)
}
pub fn encode_interaction(view: &KfNetInteraction, bytes: &mut [u8]) -> Result<usize> {
    valid_interaction(view)?;
    let mut writer = Writer { bytes, at: 0 };
    write_header(&mut writer, &view.header)?;
    writer.u8(view.floor)?;
    writer.u8(view.stage)?;
    writer.u8(view.character)?;
    writer.u8(view.page)?;
    writer.u8(view.shop)?;
    Ok(writer.at)
}
fn valid_command(command: &KfNetCommand) -> Result<()> {
    header_valid(&command.header)?;
    if command.header.kind != 4
        || command.header.sequence == 0
        || !(1..=14).contains(&command.kind)
        || (command.kind == 14 && (!(1..=2).contains(&command.object) || command.argument != 0))
        || (command.kind == 13
            && (u32::from(command.object) >= KF_AVATAR_SLOTS || command.argument != 0))
        || if command.kind == 12 {
            command.target_generation == 0
                || command.object >= 190
                || command.argument & 255 >= 4
                || command.argument >> 8 >= 80
        } else {
            command.target_generation != 0
        }
    {
        Err(Error::Invalid)
    } else {
        Ok(())
    }
}
pub fn decode_command(bytes: &[u8]) -> Result<KfNetCommand> {
    let mut reader = Reader::new(bytes)?;
    let value = KfNetCommand {
        header: read_header(&mut reader)?,
        kind: reader.u8()?,
        object: reader.u16()?,
        argument: reader.u16()?,
        target_generation: reader.u32()?,
    };
    reader.finish()?;
    valid_command(&value)?;
    Ok(value)
}
pub fn encode_command(command: &KfNetCommand, bytes: &mut [u8]) -> Result<usize> {
    valid_command(command)?;
    if bytes.len() < KF_NET_HEADER_BYTES as usize + 9 {
        return Err(Error::Full);
    }
    let mut writer = Writer { bytes, at: 0 };
    write_header(&mut writer, &command.header)?;
    writer.u8(command.kind)?;
    writer.u16(command.object)?;
    writer.u16(command.argument)?;
    writer.u32(command.target_generation)?;
    Ok(writer.at)
}
pub fn decode_fragment(bytes: &[u8]) -> Result<KfNetFragment> {
    let mut reader = Reader::new(bytes)?;
    let header = read_header(&mut reader)?;
    let total = reader.u32()?;
    let offset = reader.u32()?;
    let size = bytes.len() - reader.at;
    if header.kind != 3
        || total == 0
        || total > KF_NET_TRANSFER_LIMIT
        || offset >= total
        || size == 0
        || size > KF_NET_FRAGMENT_BYTES as usize
        || size > (total - offset) as usize
    {
        return Err(Error::Invalid);
    }
    Ok(KfNetFragment {
        header,
        total,
        offset,
        payload_offset: reader.at as u32,
        payload_size: size as u32,
    })
}
pub fn encode_fragment(
    header: &KfNetHeader,
    payload: &[u8],
    offset: u32,
    bytes: &mut [u8],
) -> Result<usize> {
    if header.kind != 3
        || payload.is_empty()
        || payload.len() > KF_NET_TRANSFER_LIMIT as usize
        || offset as usize >= payload.len()
    {
        return Err(Error::Invalid);
    }
    header_valid(header)?;
    let count = core::cmp::min(
        KF_NET_FRAGMENT_BYTES as usize,
        payload.len() - offset as usize,
    );
    if bytes.len() < KF_NET_HEADER_BYTES as usize + 8 + count {
        return Err(Error::Full);
    }
    let mut writer = Writer { bytes, at: 0 };
    write_header(&mut writer, header)?;
    writer.u32(payload.len() as u32)?;
    writer.u32(offset)?;
    writer.put(&payload[offset as usize..offset as usize + count])?;
    Ok(writer.at)
}
pub fn input_push(inbox: &mut KfNetInputInbox, bundle: &KfNetInputBundle) -> Result<()> {
    valid_input(bundle)?;
    if inbox.begin >= KF_NET_INPUT_QUEUE as u8 || inbox.count > KF_NET_INPUT_QUEUE as u8 {
        return Err(Error::Invalid);
    }
    let mut fresh = 0;
    let mut newest = inbox.received;
    for frame in &bundle.frames[..usize::from(bundle.count)] {
        if frame.sequence <= inbox.received {
            continue;
        }
        if frame.sequence - newest > 1024 {
            return Err(Error::Invalid);
        }
        newest = frame.sequence;
        fresh += 1;
    }
    if fresh > KF_NET_INPUT_QUEUE as u8 - inbox.count {
        return Err(Error::Full);
    }
    for frame in &bundle.frames[..usize::from(bundle.count)] {
        if frame.sequence <= inbox.received {
            continue;
        }
        let at = (usize::from(inbox.begin) + usize::from(inbox.count)) % inbox.pending.len();
        inbox.pending[at] = *frame;
        inbox.count += 1;
        inbox.received = frame.sequence;
    }
    Ok(())
}
pub fn input_pop(inbox: &mut KfNetInputInbox) -> Result<KfNetInputFrame> {
    if inbox.begin >= KF_NET_INPUT_QUEUE as u8 || inbox.count > KF_NET_INPUT_QUEUE as u8 {
        return Err(Error::Invalid);
    }
    if inbox.count == 0 {
        return Err(Error::Empty);
    }
    let frame = inbox.pending[usize::from(inbox.begin)];
    inbox.begin = (inbox.begin + 1) % KF_NET_INPUT_QUEUE as u8;
    inbox.count -= 1;
    inbox.consumed = frame.sequence;
    Ok(frame)
}

pub fn reset_transfer(transfer: &mut KfNetTransfer) {
    *transfer = KfNetTransfer {
        epoch: 0,
        sequence: 0,
        generation: 0,
        total: 0,
        received: 0,
    };
}

pub fn accept_fragment(
    transfer: &mut KfNetTransfer,
    bytes: &[u8],
    output: &mut [u8],
) -> Result<()> {
    let result = append_fragment(transfer, bytes, output);
    if result.is_err() {
        reset_transfer(transfer);
    }
    result
}

fn append_fragment(transfer: &mut KfNetTransfer, bytes: &[u8], output: &mut [u8]) -> Result<()> {
    let fragment = decode_fragment(bytes)?;
    if output.len() < fragment.total as usize {
        return Err(Error::Full);
    }
    if fragment.offset == 0 {
        *transfer = KfNetTransfer {
            epoch: fragment.header.epoch,
            sequence: fragment.header.sequence,
            generation: fragment.header.generation,
            total: fragment.total,
            received: 0,
        };
    }
    if transfer.epoch != fragment.header.epoch
        || transfer.sequence != fragment.header.sequence
        || transfer.generation != fragment.header.generation
        || transfer.total != fragment.total
        || transfer.received != fragment.offset
    {
        return Err(Error::Invalid);
    }
    let end = fragment.offset as usize + fragment.payload_size as usize;
    output[fragment.offset as usize..end]
        .copy_from_slice(&bytes[fragment.payload_offset as usize..]);
    transfer.received = end as u32;
    Ok(())
}
