

// EEPROM with i2c module
// 2010 Stas Novoselsky


extern void i2c_mem_init(void);
extern uint8_t i2c_mem_write(uint8_t chip_adr, unsigned int mem_adr, uint8_t *data_, uint8_t data_size);
extern uint8_t i2c_mem_read(uint8_t chip_adr, unsigned int mem_adr, uint8_t *data_, uint8_t data_size);
extern uint8_t i2c_mem_current_read(uint8_t chip_adr, uint8_t *data_, uint8_t data_size_);
extern uint8_t i2c_mem_pages_fill_byte(uint8_t chip_adr, unsigned int page_adr, uint8_t data_byte);

// ----------------  end of file   -------------------------
