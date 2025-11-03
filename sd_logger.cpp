// sd_logger.cpp — RAW SD via SPI minimal + buffer compact
// API exposée : sd_init, sd_is_ready, sd_close, sd_append_csv,
//                sd_set_max_file_size, sd_get_current_size, sd_get_current_filename

#include "sd_logger.h"
#include "config.h"
#include <SPI.h>
#include <EEPROM.h>
#include <Arduino.h>

// Fallback LED error handler
extern void led_err_sd();
extern void led_err_sd_write();
extern void led_err_sd_full();

// ---------- CONFIG ----------
static uint8_t SD_CS = 4;               // CS pin (changeable via sd_init param)
static bool    sd_ready = false;

static const uint32_t BLOCK_SIZE = 512;
extern Config config;
static uint32_t g_maxSize = 0;   // LOAD FROM CONFIG AT INIT
 // bytes, configurable

// Base block to start user data (avoid MBR/FAT area) — keep it high
static const uint32_t BASE_BLOCK = 4096UL;

// How many file slots (for hashing date -> slot)
static const uint8_t FILE_SLOTS = 8; // small number to conserve EEPROM usage

// --- RAM OPTIMISATION : blockBuf shrunk to 256B
static uint8_t blockBuf[256];
static uint16_t blockOffset = 0;       // bytes used in current blockBuf

// Current file logical parameters
static char currentDateName[7] = "000000"; // YYMMDD
static uint8_t currentFileSlot = 0;
static uint8_t currentRevision = 0;
static uint32_t bytesInFile = 0;      // bytes written in current logical file

// The next writable block address on the card for current logical file
static uint32_t currentBlockAddr = 0;

// blocks per file (computed)
static uint32_t blocksPerFile = 1;

// ----------------- LOW LEVEL SD (SPI) -----------------
// Minimal SD commands in SPI mode — compact implementation

static uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg, uint8_t crc)
{
  SPI.transfer(0x40 | cmd);
  SPI.transfer((arg >> 24) & 0xFF);
  SPI.transfer((arg >> 16) & 0xFF);
  SPI.transfer((arg >> 8) & 0xFF);
  SPI.transfer((arg >> 0) & 0xFF);
  SPI.transfer(crc);
  // wait for response (R1)
  for (uint8_t i=0; i<10; i++) {
    uint8_t r = SPI.transfer(0xFF);
    if ((r & 0x80) == 0) return r;
  }
  return 0xFF;
}

// Write a 512-byte block to raw block address
static bool sd_write_block_raw(uint32_t blockAddr, const uint8_t *buf)
{
  digitalWrite(SD_CS, LOW);
  SPI.transfer(0xFF);

  // CMD24 : write block
  sd_send_cmd(24, blockAddr, 0xFF);

  SPI.transfer(0xFF);
  // start token
  SPI.transfer(0xFE);

  // write 512 bytes
  for (uint16_t i=0;i<512;i++) SPI.transfer(buf[i]);

  // dummy CRC
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);

  // data response
  uint8_t resp = SPI.transfer(0xFF);
  bool ok = ((resp & 0x1F) == 0x05);

  // wait until not busy
  uint32_t t0 = millis();
  while (SPI.transfer(0xFF) != 0xFF) {
    if (millis() - t0 > 2000) break;
  }

  digitalWrite(SD_CS, HIGH);
  SPI.transfer(0xFF);
  return ok;
}

// Initialize SD card in SPI mode (minimal)
static bool sd_lowlevel_init()
{
  SPI.begin();
  SPI.beginTransaction(SPISettings(250000, MSBFIRST, SPI_MODE0));
  digitalWrite(SD_CS, HIGH);

  // Send 80 clocks
  for (int i=0;i<10;i++) SPI.transfer(0xFF);

  digitalWrite(SD_CS, LOW);

  // CMD0
  uint8_t r = sd_send_cmd(0, 0, 0x95);
  if (r != 0x01) { digitalWrite(SD_CS, HIGH); return false; }

  // CMD8
  r = sd_send_cmd(8, 0x1AA, 0x87);

  // ACMD41 loop
  uint32_t t0 = millis();
  for (;;) {
    sd_send_cmd(55, 0, 0x65);
    r = sd_send_cmd(41, 0x40000000UL, 0x77);
    if (r == 0x00) break;
    if (millis() - t0 > 1000) {
      digitalWrite(SD_CS, HIGH);
      return false;
    }
  }

  // CMD58
  sd_send_cmd(58, 0, 0xFD);

  digitalWrite(SD_CS, HIGH);
  SPI.transfer(0xFF);
  SPI.endTransaction();

  // Higher speed
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  return true;
}

// ----------------- Helpers for logical file mapping -----------------

static uint8_t compute_slot_from_date(const char* date6)
{
  uint16_t s = 0;
  for (uint8_t i=0;i<6;i++) s += (uint8_t)date6[i];
  return (uint8_t)(s % FILE_SLOTS);
}

static uint16_t eeprom_index_for_slot(uint8_t slot)
{
  return 1024 + slot;
}

// load revision from EEPROM
static uint8_t load_revision(uint8_t slot)
{
  return (uint8_t)EEPROM.read(eeprom_index_for_slot(slot));
}

static void save_revision(uint8_t slot, uint8_t rev)
{
  EEPROM.update(eeprom_index_for_slot(slot), rev);
}

static void compute_block_addr()
{
  uint32_t base = BASE_BLOCK + ((uint32_t)currentFileSlot * blocksPerFile);
  currentBlockAddr = base + ((uint32_t)currentRevision * blocksPerFile);
  currentBlockAddr += 1; // data starts at +1
}

// write header
static void write_header()
{
  uint8_t header[512];
  memset(header, 0, 512);
  memcpy(header, currentDateName, 6);
  header[6] = currentRevision;
  sd_write_block_raw(currentBlockAddr - 1, header);
}

// rotate file revision
static void rotate_current_file()
{
  currentRevision++;
  if (currentRevision == 0) currentRevision = 1;
  save_revision(currentFileSlot, currentRevision);

  bytesInFile = 0;
  blockOffset = 0;
  compute_block_addr();
  write_header();
}

// write pending buffer
static bool flush_current_block()
{
  if (!sd_ready) return false;
  if (blockOffset == 0) return true;

  uint8_t tmp[512];
  memcpy(tmp, blockBuf, blockOffset);
  memset(tmp + blockOffset, 0x00, 512 - blockOffset);

  if (!sd_write_block_raw(currentBlockAddr, tmp)) {
    led_err_sd_write();
    return false;
  }
  currentBlockAddr++;
  bytesInFile += blockOffset;
  blockOffset = 0;

  if (bytesInFile >= g_maxSize) {
    rotate_current_file();
  }
  return true;
}


// ----------------- Public API -----------------

bool sd_init(uint8_t csPin)
{
  SD_CS = csPin;
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  uint32_t blocks = (g_maxSize / BLOCK_SIZE);
  if ((g_maxSize % BLOCK_SIZE) != 0) blocks++;
  blocksPerFile = blocks + 2;

  if (!sd_lowlevel_init()) {
    sd_ready = false;
    return false;
  }
  sd_ready = true;

  blockOffset = 0;
  bytesInFile = 0;
  memset(currentDateName, '0', 6);
  currentDateName[6] = 0;
  currentFileSlot = 0;
  currentRevision = 0;
  currentBlockAddr = BASE_BLOCK;
  return true;
}

bool sd_is_ready()
{
  return sd_ready;
}

void sd_close()
{
  if (sd_ready) flush_current_block();
  sd_ready = false;
  SPI.endTransaction();
  SPI.end();
}

uint32_t sd_get_current_size()
{
  return bytesInFile;
}

static void build_filename(char *buf, const char* date6, uint8_t rev)
{
  snprintf(buf, 13, "%s_%d.LOG", date6, rev);
}

const char* sd_get_current_filename()
{
  static char name[13];
  build_filename(name, currentDateName, currentRevision);
  return name;
}

void sd_set_max_file_size(uint32_t bytes)
{
  g_maxSize = bytes;
  uint32_t blocks = (g_maxSize / BLOCK_SIZE);
  if ((g_maxSize % BLOCK_SIZE) != 0) blocks++;
  blocksPerFile = blocks + 2;
}


// ----------------- BUFFERED WRITE -----------------

static bool append_bytes_to_buffer(const uint8_t* data, uint16_t len)
{
  uint16_t pos = 0;
  while (pos < len) {
    uint16_t can = 256 - blockOffset;
    uint16_t tocopy = (len - pos);
    if (tocopy > can) tocopy = can;

    memcpy(&blockBuf[blockOffset], &data[pos], tocopy);
    blockOffset += tocopy;
    pos += tocopy;

    if (blockOffset == 256) {
      uint8_t tmp[512];
      memcpy(tmp, blockBuf, 256);
      memset(tmp + 256, 0x00, 256);

      if (!sd_write_block_raw(currentBlockAddr, tmp)) {
        led_err_sd_write();
        return false;
      }
      currentBlockAddr++;
      bytesInFile += 256;
      blockOffset = 0;

      if (bytesInFile >= g_maxSize) {
        rotate_current_file();
      }
    }
  }
  return true;
}


// ----------------- CSV WRITE -----------------

bool sd_append_csv(const GpsData& gps,
                   const BmeData& bme,
                   const LightData& l,
                   const char* dateStr,
                   const char* timeStr)
{
  if (!sd_ready) return false;
  if (!dateStr || !timeStr) return false;

  // date change ⇒ new logical file
  if (strncmp(dateStr, currentDateName, 6) != 0) {
    memcpy(currentDateName, dateStr, 6);
    currentDateName[6] = 0;
    currentFileSlot = compute_slot_from_date(currentDateName);
    currentRevision = load_revision(currentFileSlot);

    compute_block_addr();
    bytesInFile = 0;
    blockOffset = 0;
    write_header();
  }

  char line[200];
  char lat[12], lon[12], alt[10], sp[10];
  char t[10], h[10], p[10], ll[10];

  if (gps.fix) {
    dtostrf(gps.lat, 1, 5, lat);
    dtostrf(gps.lon, 1, 5, lon);
    dtostrf(gps.alt, 1, 0, alt);
    dtostrf(gps.speed, 1, 0, sp);
  } else {
    strcpy(lat,"NA"); strcpy(lon,"NA"); strcpy(alt,"NA"); strcpy(sp,"NA");
  }

  if (bme.ok) {
    dtostrf(bme.temp,1,1,t);
    dtostrf(bme.hum ,1,1,h);
    dtostrf(bme.pres,1,1,p);
  } else {
    strcpy(t,"NA"); strcpy(h,"NA"); strcpy(p,"NA");
  }

  if (l.ok) itoa(l.value, ll, 10); else strcpy(ll,"NA");

  snprintf(line, sizeof(line),
           "%s,%s,%s,%s,%s,%u,%s,%s,%s,%s,%s\n",
           dateStr, timeStr,
           lat, lon, alt, gps.sats, sp,
           t, h, p, ll);

  if (!append_bytes_to_buffer((const uint8_t*)line, strlen(line))) {
    led_err_sd_write();
    return false;
  }

  return true;
}
