#include <WiFi.h>
#include <Wire.h>

#include "esp_wifi.h"

#include <Adafruit_NeoPixel.h>

// See: https://www.willhackforsushi.com/papers/80211_Pocket_Reference_Guide.pdf

Adafruit_NeoPixel strip(8, GPIO_NUM_23, NEO_GRB + NEO_KHZ800);

String maclist[64][3];
int listcount = 0;

String defaultTTL = "60"; // Maximum time (Apx seconds) elapsed before device is consirded offline

const wifi_promiscuous_filter_t filt = { //Idk what this does
    .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL};

typedef struct
{
    uint64_t addr : 48;
} __attribute__((packed)) MacAddr;

typedef struct
{
    int16_t fctl;
    int16_t duration;
    MacAddr a1;
    MacAddr a2;
    MacAddr a3;
    int16_t seqctl;
    unsigned char payload[];
} __attribute__((packed)) WifiMgmtHdr;

typedef struct
{
    unsigned pad : 8;
    unsigned ds : 2;
    unsigned padd : 6;
} __attribute__((packed)) header_data;

typedef struct
{
    MacAddr addr;
    uint16_t count;
    int8_t rssi;
} record;

record list[256];

int counter = 0;

#define maxCh 11 //max Channel -> US = 11, EU = 13, Japan = 14

int curChannel = 1;

void sniffer(void *buf, wifi_promiscuous_pkt_type_t type)
{
    wifi_promiscuous_pkt_t *p = (wifi_promiscuous_pkt_t *)buf;
    MacAddr fixed;
    WifiMgmtHdr *wh = (WifiMgmtHdr *)p->payload;
    header_data data = *(header_data *)(&(wh->fctl));

    if (data.ds == 0b10) //from STA to AP
    {
        fixed = wh->a2;
    }
    else if (data.ds == 0b01)
    {
        fixed = wh->a3;
    }
    else if (data.ds == 0b00)
    {
        fixed = wh->a2;
    }
    else
    {
        return;
    }

    bool found = false;

    for (int i = 0; i < listcount; i++)
    {
        if (list[i].addr.addr == fixed.addr)
        {
            list[i].count++;
            list[i].rssi = p->rx_ctrl.rssi;
            found = true;
            break;
        }
    }

    if (!found)
    {
        if (listcount >= 256)
        {
            Serial.println("Full!");
        }
        else
        {
            list[listcount].addr.addr = fixed.addr;
            list[listcount].count = 1;
            list[listcount].rssi = p->rx_ctrl.rssi;
            listcount++;
        }
    }
}

//===== SETUP =====//
void setup()
{

    pinMode(GPIO_NUM_2, OUTPUT);
    strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
    strip.show();  // Initialize all pixels to 'off'

    /* start Serial */
    Serial.begin(115200);

    /* setup wifi */
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_start();
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_filter(&filt);
    esp_wifi_set_promiscuous_rx_cb(&sniffer);
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);

    Serial.println("starting!");
}

char *macPrint(MacAddr mac)
{
    char *str = (char *)malloc(18); //12+5+1
    int cur = 0;
    for (int i = 0; i < 48; i += 8)
    {
        if (i)
        {
            str[cur] = ':';
            cur++;
        }
        uint8_t bite = (mac.addr >> i) & 0xff;
        sprintf(str + cur, "%02x", bite);
        cur += 2;
    }
    str[cur] = '\0';
    return str;
}

void showpeople()
{ // This checks if the MAC is in the reckonized list and then displays it on the OLED and/or prints it to serial.
    Serial.printf("Found %d devices.\n", listcount);
    int max = -80;
    for (int i = 0; i < listcount; i++)
    {
        if (list[i].count && list[i].rssi > -80)
        {
            char *mac = macPrint(list[i].addr);
            Serial.printf("%s : rssi: %d, count: %d\n", mac, list[i].rssi, list[i].count);
            free(mac);
            if (list[i].rssi > max)
            {
                max = list[i].rssi;
            }
            if(list[i].addr.addr == 0x1bcd71caefdc){
                uint8_t light = constrain(map(list[i].rssi, -70, -30, 0, 255),0,255);
                Serial.printf("I: %d, light: %d\n", list[i].rssi, light);
                strip.fill(strip.ColorHSV(light*256));
                strip.show();
            }
        }
    }
    
}


void loop()
{
    //Serial.println("Changed channel:" + String(curChannel));
    if (curChannel > maxCh)
    {
        curChannel = 1;
    }
    esp_wifi_set_channel(curChannel, WIFI_SECOND_CHAN_NONE);
    delay(100);
    //updatetime();
    //purge();
    showpeople();
    curChannel++;
    counter++;
    // if (counter > 10000)
    // {
    //     counter = 0;
    //     memset(&list, 0, sizeof(list));
    //     listcount = 0;
    // }
}