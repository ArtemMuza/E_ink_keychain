#include "screen.h"

#include <string.h>

#include "driver/spi_master.h"

static spi_device_handle_t spi_device;

void DEV_SPI_WriteByte(UBYTE value)
{
    esp_err_t ret;
	spi_transaction_t tx_transaction;
	memset(&tx_transaction, 0, sizeof(tx_transaction));
	tx_transaction.length = 8 * 1;         // Transmit 1 bytes
	tx_transaction.tx_buffer = &value;

	ret = spi_device_transmit(spi_device, &tx_transaction);
    assert(ret == ESP_OK);

}

int DEV_Module_Init(void)
{
	// Configuration for the GPIO pins
    gpio_config_t io_config;
    io_config.intr_type = GPIO_INTR_DISABLE;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pin_bit_mask = (1ULL << EPD_RST_PIN) | (1ULL << EPD_DC_PIN) | (1ULL << EPD_PWR_PIN) | (1ULL << EPD_CS_PIN) | (1ULL << EPD_BUSY_PIN);
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_config);

    // Configuration for the SPI bus
    spi_bus_config_t bus_cfg = {
           .miso_io_num = -1,                // Master In Slave Out (MISO)
           .mosi_io_num = 14,                // Master Out Slave In (MOSI)
           .sclk_io_num = 13,                // Serial Clock (SCLK)
           .quadwp_io_num = -1,
           .quadhd_io_num = -1,
    };
    // Initialize the SPI bus
    esp_err_t ret;
    ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, 1);
    assert(ret == ESP_OK);


    // Configuration for the SPI device
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 10 * 1000 * 1000,  // Clock speed of 10 MHz
        .mode = 0,                          // SPI mode 0
        .spics_io_num = 15,                 // Chip Select (CS/SS) pin
        .queue_size = 7,                   // Queue size
    };

    // Initialize the SPI device
    ret = spi_bus_add_device(SPI2_HOST, &dev_cfg, &spi_device);
    assert(ret == ESP_OK);

    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
	DEV_Digital_Write(EPD_PWR_PIN, 1);
    DEV_Digital_Write(EPD_RST_PIN, 1);
	return 0;
}

void DEV_Module_Exit(void)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);

    //close 5V
	DEV_Digital_Write(EPD_PWR_PIN, 0);
    DEV_Digital_Write(EPD_RST_PIN, 0);
}


/******************************************************************************
function :	Software reset
parameter:
******************************************************************************/
static void EPD_2in13_V4_Reset(void)
{
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(20);
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(2);
    DEV_Digital_Write(EPD_RST_PIN, 1);
    DEV_Delay_ms(20);
}

/******************************************************************************
function :	send command
parameter:
     Reg : Command register
******************************************************************************/
static void EPD_2in13_V4_SendCommand(UBYTE Reg)
{
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Reg);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	send data
parameter:
    Data : Write data
******************************************************************************/
static void EPD_2in13_V4_SendData(UBYTE Data)
{
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);
    DEV_SPI_WriteByte(Data);
    DEV_Digital_Write(EPD_CS_PIN, 1);
}

/******************************************************************************
function :	Wait until the busy_pin goes LOW
parameter:
******************************************************************************/
void EPD_2in13_V4_ReadBusy(void)
{
    printf("e-Paper busy\r\n");
	while(1)
	{	 //=1 BUSY
		if(DEV_Digital_Read(EPD_BUSY_PIN)==0)
			break;
		DEV_Delay_ms(10);
	}
	DEV_Delay_ms(10);
    printf("e-Paper busy release\r\n");
}

/******************************************************************************
function :	Setting the display window
parameter:
	Xstart : X-axis starting position
	Ystart : Y-axis starting position
	Xend : End position of X-axis
	Yend : End position of Y-axis
******************************************************************************/
static void EPD_2in13_V4_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend)
{
    EPD_2in13_V4_SendCommand(0x44); // SET_RAM_X_ADDRESS_START_END_POSITION
    EPD_2in13_V4_SendData((Xstart>>3) & 0xFF);
    EPD_2in13_V4_SendData((Xend>>3) & 0xFF);

    EPD_2in13_V4_SendCommand(0x45); // SET_RAM_Y_ADDRESS_START_END_POSITION
    EPD_2in13_V4_SendData(Ystart & 0xFF);
    EPD_2in13_V4_SendData((Ystart >> 8) & 0xFF);
    EPD_2in13_V4_SendData(Yend & 0xFF);
    EPD_2in13_V4_SendData((Yend >> 8) & 0xFF);
}

/******************************************************************************
function :	Set Cursor
parameter:
	Xstart : X-axis starting position
	Ystart : Y-axis starting position
******************************************************************************/
static void EPD_2in13_V4_SetCursor(UWORD Xstart, UWORD Ystart)
{
    EPD_2in13_V4_SendCommand(0x4E); // SET_RAM_X_ADDRESS_COUNTER
    EPD_2in13_V4_SendData(Xstart & 0xFF);

    EPD_2in13_V4_SendCommand(0x4F); // SET_RAM_Y_ADDRESS_COUNTER
    EPD_2in13_V4_SendData(Ystart & 0xFF);
    EPD_2in13_V4_SendData((Ystart >> 8) & 0xFF);
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void EPD_2in13_V4_TurnOnDisplay(void)
{
	EPD_2in13_V4_SendCommand(0x22); // Display Update Control
	EPD_2in13_V4_SendData(0xf7);
	EPD_2in13_V4_SendCommand(0x20); // Activate Display Update Sequence
	EPD_2in13_V4_ReadBusy();
}

static void EPD_2in13_V4_TurnOnDisplay_Fast(void)
{
	EPD_2in13_V4_SendCommand(0x22); // Display Update Control
	EPD_2in13_V4_SendData(0xc7);	// fast:0x0c, quality:0x0f, 0xcf
	EPD_2in13_V4_SendCommand(0x20); // Activate Display Update Sequence
	EPD_2in13_V4_ReadBusy();
}

static void EPD_2in13_V4_TurnOnDisplay_Partial(void)
{
	EPD_2in13_V4_SendCommand(0x22); // Display Update Control
	EPD_2in13_V4_SendData(0xff);	// fast:0x0c, quality:0x0f, 0xcf
	EPD_2in13_V4_SendCommand(0x20); // Activate Display Update Sequence
	EPD_2in13_V4_ReadBusy();
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void EPD_2in13_V4_Init(void)
{
	EPD_2in13_V4_Reset();

	EPD_2in13_V4_ReadBusy();
	EPD_2in13_V4_SendCommand(0x12);  //SWRESET
	EPD_2in13_V4_ReadBusy();

	EPD_2in13_V4_SendCommand(0x01); //Driver output control
	EPD_2in13_V4_SendData(0xF9);
	EPD_2in13_V4_SendData(0x00);
	EPD_2in13_V4_SendData(0x00);

	EPD_2in13_V4_SendCommand(0x11); //data entry mode
	EPD_2in13_V4_SendData(0x03);

	EPD_2in13_V4_SetWindows(0, 0, EPD_2in13_V4_WIDTH-1, EPD_2in13_V4_HEIGHT-1);
	EPD_2in13_V4_SetCursor(0, 0);

	EPD_2in13_V4_SendCommand(0x3C); //BorderWavefrom
	EPD_2in13_V4_SendData(0x05);

	EPD_2in13_V4_SendCommand(0x21); //  Display update control
	EPD_2in13_V4_SendData(0x00);
	EPD_2in13_V4_SendData(0x80);

	EPD_2in13_V4_SendCommand(0x18); //Read built-in temperature sensor
	EPD_2in13_V4_SendData(0x80);
	EPD_2in13_V4_ReadBusy();
}

void EPD_2in13_V4_Init_Fast(void)
{
	EPD_2in13_V4_Reset();

	EPD_2in13_V4_SendCommand(0x12);  //SWRESET
	EPD_2in13_V4_ReadBusy();

	EPD_2in13_V4_SendCommand(0x18); //Read built-in temperature sensor
	EPD_2in13_V4_SendData(0x80);

	EPD_2in13_V4_SendCommand(0x11); //data entry mode
	EPD_2in13_V4_SendData(0x03);

	EPD_2in13_V4_SetWindows(0, 0, EPD_2in13_V4_WIDTH-1, EPD_2in13_V4_HEIGHT-1);
	EPD_2in13_V4_SetCursor(0, 0);

	EPD_2in13_V4_SendCommand(0x22); // Load temperature value
	EPD_2in13_V4_SendData(0xB1);
	EPD_2in13_V4_SendCommand(0x20);
	EPD_2in13_V4_ReadBusy();

	EPD_2in13_V4_SendCommand(0x1A); // Write to temperature register
	EPD_2in13_V4_SendData(0x64);
	EPD_2in13_V4_SendData(0x00);

	EPD_2in13_V4_SendCommand(0x22); // Load temperature value
	EPD_2in13_V4_SendData(0x91);
	EPD_2in13_V4_SendCommand(0x20);
	EPD_2in13_V4_ReadBusy();
}

/******************************************************************************
function :	Clear screen
parameter:
******************************************************************************/
void EPD_2in13_V4_Clear(void)
{
	UWORD Width, Height;
    Width = (EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1);
    Height = EPD_2in13_V4_HEIGHT;

    EPD_2in13_V4_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2in13_V4_SendData(0XFF);
        }
    }

	EPD_2in13_V4_TurnOnDisplay();
}

void EPD_2in13_V4_Clear_Black(void)
{
	UWORD Width, Height;
    Width = (EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1);
    Height = EPD_2in13_V4_HEIGHT;

    EPD_2in13_V4_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2in13_V4_SendData(0X00);
        }
    }

	EPD_2in13_V4_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
	Image : Image data
******************************************************************************/
void EPD_2in13_V4_Display(UBYTE *Image)
{
	UWORD Width, Height;
    Width = (EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1);
    Height = EPD_2in13_V4_HEIGHT;

    EPD_2in13_V4_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2in13_V4_SendData(Image[i + j * Width]);
        }
    }

	EPD_2in13_V4_TurnOnDisplay();
}

void EPD_2in13_V4_Display_Fast(UBYTE *Image)
{
	UWORD Width, Height;
    Width = (EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1);
    Height = EPD_2in13_V4_HEIGHT;

    EPD_2in13_V4_SendCommand(0x24);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            EPD_2in13_V4_SendData(Image[i + j * Width]);
        }
    }

	EPD_2in13_V4_TurnOnDisplay_Fast();
}


/******************************************************************************
function :	Refresh a base image
parameter:
	Image : Image data
******************************************************************************/
void EPD_2in13_V4_Display_Base(UBYTE *Image)
{
	UWORD Width, Height;
    Width = (EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1);
    Height = EPD_2in13_V4_HEIGHT;

	EPD_2in13_V4_SendCommand(0x24);   //Write Black and White image to RAM
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
			EPD_2in13_V4_SendData(Image[i + j * Width]);
		}
	}
	EPD_2in13_V4_SendCommand(0x26);   //Write Black and White image to RAM
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
			EPD_2in13_V4_SendData(Image[i + j * Width]);
		}
	}
	EPD_2in13_V4_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and partial refresh
parameter:
	Image : Image data
******************************************************************************/
void EPD_2in13_V4_Display_Partial(UBYTE *Image)
{
	UWORD Width, Height;
    Width = (EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1);
    Height = EPD_2in13_V4_HEIGHT;

	//Reset
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(1);
    DEV_Digital_Write(EPD_RST_PIN, 1);

	EPD_2in13_V4_SendCommand(0x3C); //BorderWavefrom
	EPD_2in13_V4_SendData(0x80);

	EPD_2in13_V4_SendCommand(0x01); //Driver output control
	EPD_2in13_V4_SendData(0xF9);
	EPD_2in13_V4_SendData(0x00);
	EPD_2in13_V4_SendData(0x00);

	EPD_2in13_V4_SendCommand(0x11); //data entry mode
	EPD_2in13_V4_SendData(0x03);

	EPD_2in13_V4_SetWindows(0, 0, EPD_2in13_V4_WIDTH-1, EPD_2in13_V4_HEIGHT-1);
	EPD_2in13_V4_SetCursor(0, 0);

	EPD_2in13_V4_SendCommand(0x24);   //Write Black and White image to RAM
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
			EPD_2in13_V4_SendData(Image[i + j * Width]);
		}
	}
	EPD_2in13_V4_TurnOnDisplay_Partial();
}

/******************************************************************************
function :	Enter sleep mode
parameter:
******************************************************************************/
void EPD_2in13_V4_Sleep(void)
{
	EPD_2in13_V4_SendCommand(0x10); //enter deep sleep
	EPD_2in13_V4_SendData(0x01);
	DEV_Delay_ms(100);
}
