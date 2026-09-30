use core::{fmt, panic::Location};

pub(crate) type Result<T> = core::result::Result<T, Error>;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(crate) struct Error {
    pub location: &'static Location<'static>,
    pub kind: Kind,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(crate) enum Kind {
    Truncated {
        at: usize,
        need: usize,
        available: usize,
    },
    Invalid(&'static str),
    InvalidAt {
        message: &'static str,
        at: usize,
    },
    InvalidMagic {
        format: &'static str,
        bytes: [u8; 4],
    },
    InvalidIndex {
        index: usize,
        count: usize,
    },
    InvalidCount {
        field: &'static str,
        value: u16,
        maximum: u16,
    },
    SizeMismatch {
        field: &'static str,
        expected: usize,
        actual: usize,
    },
    InvalidValue {
        field: &'static str,
        value: i64,
    },
    InvalidByte {
        message: &'static str,
        at: usize,
        value: u8,
    },
    InvalidRecordSize {
        at: usize,
        declared: u32,
    },
    InvalidRectangle {
        at: usize,
        width: i16,
        height: i16,
    },
    OutputFull,
    End,
}

impl Error {
    #[track_caller]
    fn new(kind: Kind) -> Self {
        Self {
            location: Location::caller(),
            kind,
        }
    }

    #[track_caller]
    pub(crate) fn truncated(at: usize, need: usize, available: usize) -> Self {
        Self::new(Kind::Truncated {
            at,
            need,
            available,
        })
    }

    #[track_caller]
    pub(crate) fn invalid(message: &'static str) -> Self {
        Self::new(Kind::Invalid(message))
    }

    #[track_caller]
    pub(crate) fn invalid_at(message: &'static str, at: usize) -> Self {
        Self::new(Kind::InvalidAt { message, at })
    }

    #[track_caller]
    pub(crate) fn invalid_magic(format: &'static str, bytes: [u8; 4]) -> Self {
        Self::new(Kind::InvalidMagic { format, bytes })
    }

    #[track_caller]
    pub(crate) fn invalid_index(index: usize, count: usize) -> Self {
        Self::new(Kind::InvalidIndex { index, count })
    }

    #[track_caller]
    pub(crate) fn invalid_count(field: &'static str, value: u16, maximum: u16) -> Self {
        Self::new(Kind::InvalidCount {
            field,
            value,
            maximum,
        })
    }

    #[track_caller]
    pub(crate) fn size_mismatch(field: &'static str, expected: usize, actual: usize) -> Self {
        Self::new(Kind::SizeMismatch {
            field,
            expected,
            actual,
        })
    }

    #[track_caller]
    pub(crate) fn invalid_value(field: &'static str, value: i64) -> Self {
        Self::new(Kind::InvalidValue { field, value })
    }

    #[track_caller]
    pub(crate) fn invalid_byte(message: &'static str, at: usize, value: u8) -> Self {
        Self::new(Kind::InvalidByte { message, at, value })
    }

    #[track_caller]
    pub(crate) fn invalid_record_size(at: usize, declared: u32) -> Self {
        Self::new(Kind::InvalidRecordSize { at, declared })
    }

    #[track_caller]
    pub(crate) fn invalid_rectangle(at: usize, width: i16, height: i16) -> Self {
        Self::new(Kind::InvalidRectangle { at, width, height })
    }

    #[track_caller]
    pub(crate) fn output_full() -> Self {
        Self::new(Kind::OutputFull)
    }

    #[track_caller]
    pub(crate) fn end() -> Self {
        Self::new(Kind::End)
    }
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.kind {
            Kind::Truncated {
                at,
                need,
                available,
            } => write!(
                f,
                "truncated input at {at}: need {need} bytes, have {available}"
            ),
            Kind::Invalid(message) => f.write_str(message),
            Kind::InvalidAt { message, at } => write!(f, "{message} at {at}"),
            Kind::InvalidMagic { format, bytes } => {
                write!(f, "invalid {format} magic: {bytes:02x?}")
            }
            Kind::InvalidIndex { index, count } => {
                write!(f, "index {index} exceeds record count {count}")
            }
            Kind::InvalidCount {
                field,
                value,
                maximum,
            } => write!(f, "{field} count {value} exceeds {maximum}"),
            Kind::SizeMismatch {
                field,
                expected,
                actual,
            } => write!(f, "{field} size: expected {expected}, have {actual}"),
            Kind::InvalidValue { field, value } => write!(f, "invalid {field}: {value}"),
            Kind::InvalidByte { message, at, value } => {
                write!(f, "{message} at {at}: {value:#04x}")
            }
            Kind::InvalidRecordSize { at, declared } => {
                write!(f, "invalid record size {declared} at {at}")
            }
            Kind::InvalidRectangle { at, width, height } => {
                write!(f, "invalid rectangle {width}x{height} at {at}")
            }
            Kind::OutputFull => f.write_str("codec output is full"),
            Kind::End => f.write_str("end of input"),
        }
    }
}

impl core::error::Error for Error {}

macro_rules! bail {
    ($message:expr) => {
        return Err($crate::Error::invalid($message))
    };
}
pub(crate) use bail;
