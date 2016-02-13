extern void init_usi_i2c_master(void);
extern void i2c_start(void);
extern void i2c_stop(void);
//extern uint8_t usi_i2c_master_transfer(uint8_t tmp_sr);
extern uint8_t i2c_master_send(uint8_t *data, uint8_t data_size);
extern uint8_t i2c_master_read(uint8_t *data, uint8_t data_size);


//--------  end of file   -------------------------
