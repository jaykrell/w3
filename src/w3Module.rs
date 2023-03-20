#[derive(Educe)]
#[educe(Default)]
pub struct T {
    pub size: i64,
    #[educe(Default = 1)]
    pub j : i32,
    pub k : i32,
}

impl T {
    pub fn read_byte(&mut self) -> i32 {
        // implementation goes here
        0
    }
}
